#include <proc.h>

#include <intr.h>
#include <pq.h>
#include <kout.h>

extern struct pq *readylist;
extern struct proc *proctab[];
extern struct proc *prntA;
extern int currpid;

extern void ctxsw(u32 *stkptrold, u32 *stkptrnew);

void sched()
{
	static int count = 0;
	currpid = count & 1;
	count++;
	struct proc *prptr = currproc();
	//struct proc *newproc = (struct proc *) readylist->data;

	disable();

	//struct proc *p;
	// if (count & 1)
	// 	p = proctab[1];
	// else
	// 	p = proctab[0];

	struct proc *prnew;

	if (currpid == 0)
		prnew = proctab[1];
	else
		prnew = proctab[0];

	ctxsw(prptr->stkptr, prnew->stkptr);

	

	//koutf("%s\n", prptr->name);


	// if (newproc)
	// {
	// 	if (newproc == prntA)
	// 		koutf("its printA\n");
	// 	koutf("%s\n", newproc->name);
	// 	rm(&readylist);
	// 	insert(&readylist, newproc, proccmp);
	// 	//ctxsw();
	// }

}