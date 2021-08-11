#ifndef PROC_H
#define PROC_H

#include <maestro.h>
#include <state.h>

// max number of processes (for now), 
#define NPROC		100

enum prstate
{
	PR_READY,
	PR_RUNNING,
	PR_WAITING,
};

struct proc
{
	char			name[32];
	int				pid;
	int 			prio;
	u32 			stkbase;
	u32 			stkptr;
	struct state 	state;
	enum prstate 	prstate;
	u32				pc;
};

// process ready list
extern struct pq *readylist;

static int proccmp(void *a, void *b) { return -1; }

int curr();
struct proc *currproc();
void sched();
int spawn(int (*)(void), char *, int);

#endif // PROC_H