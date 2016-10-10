#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <map>

#include "branch.h"
#include "trace.h"

#define BUFSIZE	10000000

extern bool compressing;

FILE *tracefp;

#define ZCAT		"/bin/gzip -dc"
#define BZCAT		"/usr/bin/bzip2 -dc"
#define CAT		"/bin/cat"

unsigned char buf[BUFSIZE];
unsigned int bufpos, bufsize;
bool end_of_file;
long long int Total_bytes = 0;

unsigned char read_byte (void) {
	if (bufpos == bufsize) {
		bufpos = 0;
		bufsize = fread (buf, 1, BUFSIZE, tracefp);
		fprintf (stderr, "read %d bytes\n", bufsize);
		if (bufsize == 0) {
			end_of_file = true;
			return 0;
		}
	}
	Total_bytes++;
	return buf[bufpos++];
}

unsigned int read_uint (void) {
	unsigned int x0, x1, x2, x3;

	x0 = read_byte ();
	x1 = read_byte ();
	x2 = read_byte ();
	x3 = read_byte ();
	return x0 | (x1 << 8) | (x2 << 16) | (x3 << 24);
}

struct remember {
	bool taken;
	unsigned char code;
	unsigned int address, target;
	unsigned int lru_time;

	remember (void) {
		code = 0;
		address = 0;
		target = 0;
		taken = 0;
		lru_time = 0;
	}

	remember (unsigned char c, unsigned int a, unsigned int t, bool ta) {
		code = c;
		address = a;
		target = t;
		taken = ta;
	}

	bool equal (remember *r, bool ignore_target) {
		return
		   r->code == code
		&& r->taken == taken
		&& r->address == address 
		&& (ignore_target || r->target == target);
	}
};

// a return address stack

#define RAS_SIZE	100

unsigned int ras[RAS_SIZE];
int ras_top = 100;

void init_ras (void) {
	ras_top = RAS_SIZE;
}

void push_ras (unsigned int a) {
	if (ras_top) ras[--ras_top] = a;
}

unsigned int pop_ras (void) {
	if (ras_top < RAS_SIZE) return ras[ras_top++];
	return 0;
}

#define N_REMEMBER	(1<<16)
#define ASSOC		8

remember rtab[N_REMEMBER][ASSOC];

static unsigned int now = 0;
static remember last_one;

remember *predict_remember (void) {
	unsigned int index = last_one.target & (N_REMEMBER-1);
	remember *r = &rtab[index][0];
	return r;
}

int search_remember (remember & me, remember *r, bool ras_correct) {
	for (int i=0; i<ASSOC; i++) if (me.equal (&r[i], ras_correct)) return i;
	return -1;
}

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

static unsigned int ntimes = 0;
static unsigned int nright = 0;
static unsigned int total_bytes = 0, trace_bytes = 0;

trace *read_trace (void) {
	static trace t;
	static trace last_trace;
	static int ras_hits = 0, ras_ntimes = 0;
	unsigned char c = read_byte ();
	if (end_of_file) return NULL;
	t.bi.br_flags = 0;
	unsigned int a;
	// pass along instruction counts unchanged (we don't care)
	if (c == 0x87) {
		int x = 0, y = 0;
		fwrite (&c, 1, 1, stdout);
		c = read_byte ();
		x = c;
		fwrite (&c, 1, 1, stdout);
		c = read_byte ();
		y = c;
		y <<= 8;
		x |= y;
		//fprintf (stderr, "%d more insts\n", x);
		fwrite (&c, 1, 1, stdout);
		c = read_byte ();
	}
	if (compressing) {
		t.bi.address = read_uint ();
		t.target = read_uint ();
		// all branches are taken except for conditional not taken branches
		t.taken = true;
	}
	static unsigned int classmispred[8];
	ntimes++;
	bool correct;
	if (compressing) {
		assert ((c & 0x80) == 0);
		remember r(c, t.bi.address, t.target, t.taken);
		remember *p = predict_remember ();
		bool ras_correct = false;
		bool ras_offby2 = false;
		bool ras_offby3 = false;
		if (c == 0x70) {
			unsigned int popd = pop_ras();
			ras_correct = popd == t.target;
			if (!ras_correct) {
				if (t.target == popd + 2) {
					ras_correct = true;
					ras_offby2 = true;
				} else if (t.target == popd - 3) {
					ras_correct = true;
					ras_offby3 = true;
				}
			}
			ras_ntimes++;
			if (!ras_correct)  {
				//fprintf (stderr, "%x %x\n", popd, t.target);
				init_ras ();
			}
			else
				ras_hits++;
		}
		int index = search_remember (r, p, ras_correct);
		correct = index != -1;
		update_remember (r, p, correct, index);
		if (correct) {
			unsigned char out;
			if (ras_correct) index += ASSOC;
			if (ras_offby2) {
				out = 0x82;
				fwrite (&out, 1, 1, stdout);
			} else if (ras_offby3) {
				out = 0x83;
				fwrite (&out, 1, 1, stdout);
			}
			out = (unsigned char) index;
			fwrite (&out, 1, 1, stdout);
			nright++; 
			total_bytes++;
		} else {
			fwrite (&c, 1, 1, stdout);
			fwrite (&t.bi.address, 4, 1, stdout);
			fwrite (&t.target, 4, 1, stdout);
			total_bytes += 1 + 4 + 4;
			trace_bytes += 1 + 4 + 4;
		}
		if (ntimes % 1000000 == 0) {
			fprintf (stderr, "%f %f\n", nright / (double) ntimes, trace_bytes / (double) total_bytes);
			fprintf (stderr, "%f\n", ras_hits / (double) ras_ntimes);
			for (int i=1; i<=7; i++) {
				fprintf (stderr, "%d %d\n", i, classmispred[i]);
			}
		}
	} else {
		remember r;
		remember *p = predict_remember ();
		bool ras_offby2 = false, ras_offby3 = false;
		if (c & 0x80) {
			if (c == 0x82)
				ras_offby2 = true;
			else if (c == 0x83)
				ras_offby3 = true;
			else assert (0);
			c = read_byte ();
		}
		correct = c < ASSOC*2;
		if (correct) {
			bool ras_correct = c >= ASSOC;
			if (ras_correct) c -= ASSOC;
			r.address = p[c].address;
			r.target = p[c].target;
			r.taken = p[c].taken;
			r.code = p[c].code;
			if (r.code == 0x70) {
				unsigned int popd = pop_ras();
				if (ras_correct) {
					r.target = popd;
					if (ras_offby2) r.target += 2;
					else if (ras_offby3) r.target -= 3;
				}
				else
					init_ras();
			}
			assert (r.equal (&p[c], ras_correct));
			t.bi.address = r.address;
			t.target = r.target;
			t.taken = r.taken;
			update_remember (r, p, true, (int) c);
			c = r.code;
		} else {
			t.bi.address = read_uint ();
			t.target = read_uint ();
			t.taken = true;
			r.address = t.bi.address;
			r.target = t.target;
			r.taken = t.taken;
			r.code = c;
			// if this is a return, manage RAS
			if (r.code == 0x70) {
				// could be a correct RAS prediction
				// but with incorrect call site???
				unsigned int popd = pop_ras ();
				if (popd != t.target
				&& popd != t.target - 2
				&& popd != t.target + 3) init_ras();
			}
			update_remember (r, p, false, -1);
		}
		fwrite (&c, 1, 1, stdout);
		fwrite (&t.bi.address, 4, 1, stdout);
		fwrite (&t.target, 4, 1, stdout);
	}
	t.bi.opcode = c & 15;
	c >>= 4;
	if (!correct) classmispred[c]++;
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
	default: fprintf (stderr, "%d at byte %lld\n", c, Total_bytes); fflush (stderr); assert (0);
	}
	last_trace = t;
	return & t;
}

#define GZIP_MAGIC     "\037\213"
#define BZIP2_MAGIC	"BZ"

void init_trace (char *fname) {
	char *dc;
	char s[2] = { 0, 0 };
	char cmd[1000];

	// figure out the compression method from the magic number

	if (!strcmp (fname, "-")) {
		fprintf (stderr, "reading from standard input\n");
		tracefp = stdin;
	} else {
	FILE *f = fopen (fname, "r");
	if (!f) {
		perror (fname);
	}
	fread (s, 1, 2, f);
	fclose (f);
	if (strncmp (s, GZIP_MAGIC, 2) == 0) 
		fprintf (stderr, "GZIP\n"), dc = ZCAT;
	else if (strncmp (s, BZIP2_MAGIC, 2) == 0)
		fprintf (stderr, "BZIP2\n"), dc = BZCAT;
	else
		fprintf (stderr, "nothing\n"), dc = CAT;

	// make a command that will decompress the file to stdout

	sprintf (cmd, "%s %s", dc, fname);

	// pipe that stdout to tracefp

	tracefp = popen (cmd, "r");
	if (!tracefp) {
		perror (fname);
		exit (1);
	}
	}
	bufpos = 0;
	bufsize = 0;
	end_of_file = false;
	memset (rtab, 0, sizeof (rtab));
	now = 0;
	init_ras();
}

void end_trace (void) {
	if (compressing) fprintf (stderr, "pred rate: %f ; trace bytes rate: %f\n", nright / (double) ntimes, trace_bytes / (double) total_bytes);
	if (tracefp != stdin) pclose (tracefp);
}
