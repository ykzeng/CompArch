// trace.cc
// This file contains code for reading traces.  There's nothing in this
// file you need to understand to participate in the branch prediction 
// contest.

// djimenez

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "branch.h"
#include "trace.h"

// A trace is a piece of information about a branch.  The external 
// representation of a trace is 9 bytes:
// - A one byte "code."  The lower 4 bits are the x86 opcode for a
// conditional branch (modulo 16).  The upper four bits are one of the
// following:
// 1 : taken conditional branch
// 2 : not taken conditional branch
// 3 : unconditional branch
// 4 : indirect branch
// 5 : call
// 6 : indirect call
// 7 : return
// - A four byte little-endian branch address.  This is the address in memory 
// of the first byte of the branch instruction.
// - A four byte little-endian branch target.  This is the address in memory 
// where the branch jumped.
//
// The input file is usually compressed either with gzip or bzip2 and this
// file contains code to support reading from these formats by piping the
// output of the decompressors.  However, this file s does another kind of
// decompression on the traces after they have been decompressed by gzip
// or bzip2.  If the upper four bits of the first byte read are either
// 0 or 8 then the byte indicates that the trace has been compressed
// from the 9 byte representation to a 1 or 2 byte representation.  This
// compression is faciliated with prediction described below.  The compression
// achieved is not impressive -- Huffman coding would do much better -- but
// the purpose is to allow the stream of bytes fed to gzip or bzip2 to be
// much more redundant and hence more compressible.

// number of bytes to read at once from the decompressor

#define BUFSIZE	10000

// file pointer for the pipe from the decompressor

FILE *tracefp;

// buffer to read bytes into

unsigned char buf[BUFSIZE];

// current position in buffer
unsigned int bufpos;

// number of bytes read into buffer

unsigned int bufsize;

// true when end of file is reached

bool end_of_file;

// read a single byte from the trace file

unsigned char read_byte (void) {

	// if the buffer is empty...

	if (bufpos == bufsize) {

		// get a BUFSIZE-sized chunk of bytes from the input

		bufpos = 0;
		bufsize = fread (buf, 1, BUFSIZE, tracefp);

		// nothing to read?  we must be done.

		if (bufsize == 0) {
			end_of_file = true;
			return 0;
		}
	}

	// one more byte 

	return buf[bufpos++];
}

// read an unsigned integer in little endian format from the trace file

unsigned int read_uint (void) {
	unsigned int x0, x1, x2, x3;

	x0 = read_byte ();
	x1 = read_byte ();
	x2 = read_byte ();
	x3 = read_byte ();
	return x0 | (x1 << 8) | (x2 << 16) | (x3 << 24);
}

// these "remember" structs and functions handle decompressing certain traces
// using prediction.  the compression is a simple table-based predictor that
// also uses a return address stack for predicting return addresses.  
// obviously this is a space win, but it is also a measurable performance 
// win since there are fewer bytes to read.

struct remember {
	bool taken;
	unsigned char code; 
	unsigned int address, target;
	unsigned int lru_time;

	// constructor

	remember (void) {
		code = 0;
		address = 0;
		target = 0;
		taken = 0;
		lru_time = 0;
	}

	// return true if two remember structs are equivalent.  optionally
	// ignore the target since it might have been correctly predicted
	// by the return address stack

	bool equal (remember *r, bool ignore_target) {
		return
		   r->code == code
		&& r->taken == taken
		&& r->address == address 
		&& (ignore_target || r->target == target);
	}
};

// a return address stack
                                                                                
#define RAS_SIZE        100
                                                                                
unsigned int ras[RAS_SIZE];
int ras_top = RAS_SIZE;

// (re)initialize the return address stack
void init_ras (void) {
	ras_top = RAS_SIZE;
}

// push a target onto the return address stack

void push_ras (unsigned int a) {
	if (ras_top) ras[--ras_top] = a;
}

// pop a target from the return address stack

unsigned int pop_ras (void) {
	if (ras_top < RAS_SIZE) return ras[ras_top++];
	return 0;
}

// parameters for the predictor table

#define N_REMEMBER	(1<<16)
#define ASSOC		8

// the predictor table; a 64k-entry 8-way set associative memory.
// a hash table with probing would probably be more space-efficient
// but I think this is a little faster (neither has good locality).
// we can only remember up to 8 possible predictions per branch target
// because we're squeezing set indices into a 3-bit code so having
// a fixed set size is OK.  in practice, most branches need only 1 or 2
// possible predictions, but some traces benefit from higher associativity.

remember rtab[N_REMEMBER][ASSOC];

// this int keeps time for the LRU algorithm

static unsigned int now = 0; 

// last trace seen

static remember last_one; 

// predict a trace

remember *predict_remember (void) {
	unsigned int index = last_one.target & (N_REMEMBER-1);
	remember *r = &rtab[index][0];
	return r;
}

// update the predictor

void update_remember (remember & me, remember *r, bool correct, int index) {
	if (correct) {
		r[index].lru_time = now++;
	} else {
		// throw out the LRU item and replace it with me
		int lru = 0;
		for (int i=1; i<ASSOC; i++)
			if (r[i].lru_time < r[lru].lru_time) lru = i;
		r[lru] = me;
		r[lru].lru_time = now++;
	}
	last_one = me;
}

// read a single trace from the file

trace *read_trace (void) {
	static trace t;
	bool ras_correct, ras_offby2, ras_offby3, correct;

	// read the next byte; it will either be a code, a set index for
	// a correct prediction, or a prefix for patching a return address 
	// prediction.

	unsigned char c = read_byte ();
	if (end_of_file) return NULL;
	remember r;

	// predict the next trace

	remember *p = predict_remember ();

	// assume return address prediction is correct

	ras_offby2 = false;
	ras_offby3 = false;

	// if the high bit of the first byte is set...

	if (c & 0x80) {
		// then it means the return address predictor will be
		// slightly off but we can patch the prediction to make
		// it correct.  this happens sometimes (rarely) because of 
		// x86's variable-length call instructions.

		if (c == 0x82)

			// add 2 to the predicted target

			ras_offby2 = true;
		else if (c == 0x83)

			// subtract 3 from the predicted target

			ras_offby3 = true;
		else assert (0);

		// read the next byte; it should be the set index for
		// a correct return address prediction

		c = read_byte ();
	}

	// the byte is a correct prediction if it is less than 8;
	// otherwise it is the first byte (a code) in a 9-byte trace

        correct = c < ASSOC*2;
	if (correct) {

		// if the byte is at least 4 then it means that we have
		// a correct return address prediction
		
		ras_correct = c >= ASSOC;

		// subtract off ASSOC for a correct return address prediction

		if (ras_correct) c -= ASSOC;

		// at this point we have the predicted set in p
		// and the index into the predicted set in c.

		r = p[c];

		// if this is a trace for a return...

		if (r.code == 0x70) {

			// pop the return address stack

			unsigned int popd = pop_ras();

			// if the return address stack prediction was
			// correct...
			if (ras_correct) {

				// set the corresponding field of r

				r.target = popd;

				// and fix the target if need be

				if (ras_offby2) r.target += 2;
				else if (ras_offby3) r.target -= 3;
			} else

				// otherwise, we had a correct prediction
				// but an incorrect return address prediction;
				// flush the return address stack

				init_ras();
		}

		// set the rest of the fields from the prediction

		t.bi.address = r.address;
		t.target = r.target;
		t.taken = r.taken;

		// update the predictor

		update_remember (r, p, true, (int) c);

		// get the code into c for later use

		c = r.code;
	} else {

		// the predictor was incorrect.  just read the trace from
		// the input.  this happens rarely, often less than 1% of the
		// time, but it has to happen sometime because this is where
		// the actual information comes from

		// read the branch address

		t.bi.address = read_uint ();

		// read the branch target

		t.target = read_uint ();

		// assume the branch is taken; fix later

		t.taken = true;

		// prepare a remember struct for the predictor

		r.address = t.bi.address;
		r.target = t.target;
		r.taken = t.taken;
		r.code = c;

		// if we have a return...
		if (r.code == 0x70) {

			// pop the return address stack

			unsigned int popd = pop_ras ();

			// if we have a mispredicted return address,
			// flush the return address stack.  why are we
			// bothering about predicting when we know the
			// prediction is incorrect?  because the original
			// compressor maintains a return address stack 
			// regardless of whether the trace is predicted
			// correctly, so we have to also.

			if (popd != t.target
			 && popd != t.target - 2
			 && popd != t.target + 3) init_ras();
		}

		// update the predictor

		update_remember (r, p, false, -1);
	}

	// get the conditional branch opcode, if any

	t.bi.opcode = c & 15;

	// br_flags gives information about the branch; initially empty

	t.bi.br_flags = 0;

	// get the high 4 bits of the code

	c >>= 4;
	switch (c) {
	case 1: // taken conditional branch
		t.bi.br_flags |= BR_CONDITIONAL;
		break;
	case 2: // not taken conditional branch
		t.taken = false;
		t.bi.br_flags |= BR_CONDITIONAL;
		break;
	case 3: // unconditional branch
		break;
	case 4: // indirect branch
		t.bi.br_flags |= BR_INDIRECT;
		break;
	case 5: // call
		t.bi.br_flags |= BR_CALL;
		push_ras (t.bi.address + 5);
		break;
	case 6: // indirect call
		t.bi.br_flags |= BR_CALL | BR_INDIRECT;
		push_ras (t.bi.address + 2);
		break;
	case 7: // return
		t.bi.br_flags |= BR_RETURN;
		break;
	// this should "never" happen
	default: fprintf (stderr, "%d\n", c); fflush (stderr); assert (0);
	}
	return & t;
}

// open the trace file for reading

#define GZIP_MAGIC     "\037\213"
#define BZIP2_MAGIC	"BZ"

void init_trace (char *fname) {
	const char *dc;
	char s[2] = { 0, 0 };
	char cmd[1000];

	// figure out the compression method from the magic number

	FILE *f = fopen (fname, "r");
	if (!f) {
		perror (fname);
	}
	assert (fread (s, 1, 2, f) == 2);
	fclose (f);
	if (strncmp (s, GZIP_MAGIC, 2) == 0) 
		dc = ZCAT;
	else if (strncmp (s, BZIP2_MAGIC, 2) == 0)
		dc = BZCAT;
	else
		dc = CAT;

	// make a command that will decompress the file to stdout

	sprintf (cmd, "%s %s", dc, fname);

	// pipe that stdout to tracefp

	tracefp = popen (cmd, "r");
	if (!tracefp) {
		perror (fname);
		exit (1);
	}
	bufpos = 0;
	bufsize = 0;
	end_of_file = false;
}

// close the trace file

void end_trace (void) {
	fclose (tracefp);
}
