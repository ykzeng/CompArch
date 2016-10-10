// trace.h
// This file declares functions and a struct for reading trace files

struct trace {
	bool	taken;
	unsigned int target;
	branch_info bi;

	trace (void) {
		taken = 0;
		target = 0;
	}
};

void init_trace (char *);
trace *read_trace (void);
void end_trace (void);
