#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <zlib.h>
#include <map>

#include "branch.h"
#include "trace.h"

bool compressing = false;

int main (int argc, char *argv[]) {
	long long int ntraces = 0;
	if (argc < 3) {
		fprintf (stderr, "Usage: %s [ -d | -c ] <filename>.gz\n", argv[0]);
		exit (1);
	}
	if (strcmp (argv[1], "-c") == 0) {
		compressing = true;
	} else if (strcmp (argv[1], "-d") == 0) {
		compressing = false;
	} else {
		fprintf (stderr, "Usage: %s [ -d | -c ] <filename>.gz\n", argv[0]);
		exit (1);
	}
	for (int i=2; i<argc; i++) {
		fprintf (stderr, "reading \"%s\"\n", argv[i]);
		fflush (stderr);
		init_trace (argv[i]);
		long long int tmiss = 0, dmiss = 0, branches = 0;
		for (;;) {
			trace *t = read_trace ();
			if (!t) break;
			ntraces++;
		}
		end_trace ();
	}
	fprintf (stderr, "%lld traces\n", ntraces);
	exit (0);
}
