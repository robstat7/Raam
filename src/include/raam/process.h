#ifndef PROCESS_H
#define PROCESS_H

/* process states */
#define RUNNING		0
#define TERMINATED	1

struct process_control_block_struct {
	int pid;	/* process ID (unique identifier) */
	int state;	/* process state: RUNNING, TERMINATED */
};

#endif	/* PROCESS_H */
