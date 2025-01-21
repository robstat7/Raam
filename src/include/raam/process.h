#ifndef PROCESS_H
#define PROCESS_H

/* process states */
#define RUNNING		1
#define TERMINATED	2
#define READY		0

struct process_control_block_struct {
	int pid;	/* process ID (unique identifier) */
	int state;	/* process state */
};

void switch_process(struct process_control_block_struct *next_process);
void terminate_process(void);

#endif	/* PROCESS_H */
