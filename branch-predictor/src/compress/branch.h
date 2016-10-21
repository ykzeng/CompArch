// branch.h
// This file defines the branch_info class.

#define OP_JO	0
#define OP_JNO	1
#define OP_JC	2
#define OP_JNC	3
#define OP_JZ	4
#define OP_JNZ	5
#define OP_JBE	6
#define OP_JA	7
#define OP_JS	8
#define OP_JNS	9
#define OP_JP	10
#define OP_JNP	11
#define OP_JL	12
#define OP_JGE	13
#define OP_JLE	14
#define OP_JG	15

#define BR_CONDITIONAL	1
#define BR_INDIRECT	2
#define BR_CALL		4
#define BR_RETURN	8

struct branch_info {
	unsigned int 
		address, 	// branch address
		opcode,		// opcode for conditional branch
		br_flags;	// OR of some BR_ flags

	branch_info (void) {
		address = 0;
		opcode = 0;
		br_flags = 0;
	}
};
