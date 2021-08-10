/* maestro
 * License: GPLv2
 * See LICENSE.txt for full license text
 * Author: Sam Kravitz
 *
 * FILE: kmain.c
 * DATE: July 26, 2021
 * DESCRIPTION: Where it all begins
 */
#include <init.h>
#include <kout.h>
#include <proc.h>
#include <pq.h>

#include "malloc.h"
#include "string.h"

extern struct pq *readylist;
extern struct proc *proctab[];

struct proc *prntA;

int printA()
{
	while (1)
		koutf("%c", 'A');
	
	return 0;
}

int printB()
{
	while (1)
		koutf("%c", 'B');
	
	return 0;
}

void kmain()
{
	kout("Welcome to maestro!\n");
	init();

	// set up null process
	struct proc *null = (struct proc *) malloc(sizeof(struct proc));
	u32 stkptr = (u32) malloc(1024);
	null->pid = 0;
	null->prio = -20;
	null->stkbase = stkptr;
	null->stkptr = stkptr;
	null->prstate = PR_READY;
	null->pc = printB;
	memcpy(null->name, "null process\0", strlen("null process") + 1);

	// set up print A process
	prntA = (struct proc *) malloc(sizeof(struct proc));
	stkptr = (u32) malloc(1024);
	prntA->pid = 1;
	prntA->prio = 0;
	prntA->stkbase = stkptr;
	prntA->stkptr = stkptr;
	prntA->prstate = PR_READY;
	prntA->pc = printA;
	memcpy(prntA->name, "print A\0", strlen("print A") + 1);

	// set up print B process
	// struct proc *prntB = (struct proc *) malloc(sizeof(struct proc));
	// stkptr = (u32) malloc(1024);
	// prntB->pid = 1;
	// prntB->prio = 0;
	// prntB->stkbase = stkptr;
	// prntB->stkptr = stkptr;
	// prntB->prstate = PR_READY;
	// memcpy(prntB->name, "print B\0", strlen("print B") + 1);


	// create ready list
	readylist = newpq(null);

	insert(&readylist, null, proccmp);
	insert(&readylist, printA, proccmp);

	proctab[0] = null;
	proctab[1] = prntA;
	//insert(&readylist, printB, proccmp);
	

	while (1)
		asm("hlt");
}