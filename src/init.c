/* maestro
 * License: GPLv2
 * See LICENSE.txt for full license text
 * Author: Sam Kravitz
 *
 * FILE: init.c
 * DATE: August 1st, 2021
 * DESCRIPTION: Initializes structures and systems vital for kernel
 */
#include <init.h>

#include <idt.h>
#include <intr.h>
#include <kbd.h>
#include <mm.h>
#include <pit.h>

// initializes IDT, interrupts, and the PIT
void init()
{
	iintr();
	iidt();
	//itimer();
	ikbd();
	mminit();
	asm("sti"); // enable interrupts
}
