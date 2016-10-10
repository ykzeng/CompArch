// trace.h
// This file declares functions and a struct for reading trace files.

// these #define the Unix commands for decompressing gzip, bzip2, and
// plain files.  If they are somewhere else on your system, change these
// definitions.

#define ZCAT            "/bin/gzip -dc"

// this is where bzip2 is on my Mac

//#define BZCAT		"/opt/local/bin/bzip2 -dc"
//#define BZCAT		"/opt/csw/bin/bzip2 -dc"

// this is where it is on Ubuntu Linux

#define BZCAT           "/bin/bzip2 -dc"
#define CAT             "/bin/cat"

struct trace {
	bool	taken;
	unsigned int target;
	branch_info bi;
};

void init_trace (char *);
trace *read_trace (void);
void end_trace (void);
