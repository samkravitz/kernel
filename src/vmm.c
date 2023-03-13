/* maestro
 * License: GPLv2
 * See LICENSE.txt for full license text
 * Author: Sam Kravitz
 *
 * FILE: vmm.c
 * DATE: March 15th, 2022
 * DESCRIPTION: virtual memory manager
 */

#include <vmm.h>

#include <intr.h>
#include <kprintf.h>
#include <kmalloc.h>
#include <pmm.h>
#include <proc.h>

#include "stdio.h"
#include "string.h"

extern struct proc nullproc;

// kernel page directory
extern struct pde kpage_dir[NUM_TABLE_ENTRIES];

// kernel page table
extern struct pte kpage_table[NUM_TABLE_ENTRIES];

// identity page table
extern struct pde ident_page_table[NUM_TABLE_ENTRIES];

extern struct pte fb_page_table[NUM_TABLE_ENTRIES];

extern u32 start_phys, start;

// pointer to heap, defined in kmalloc.c
extern void *heap;

// converts virtual address addr to a physical address
#define VIRT_TO_PHYS(addr) ((u32) &start_phys + (u32) addr - (u32) &start)

static void page_fault();

/**
 * the bootloader kept data structures for initializing paging in the first ~10K of memory.
 * once our pmm is initialized, that region of memory will be marked as available
 * and could be overridden at any point, so we need to copy those data structures
 * to kernel memory so we know it won't be overriden.
 * 
 * the 3 data structures we have to save are the kernel's page directory,
 * the page table that identity maps the first 1M of memory,
 * and the page table that maps the first 4M of the kernel.
 */
void vmm_init()
{
	set_vect(14, page_fault);

    u32 *kpage_table = pmm_alloc();
    u32 *kpage_dir = pmm_alloc();
    u32 *ident_page_table = pmm_alloc();

    for (int i = 0, phys = &start_phys; i < 1024; i++, phys += PAGE_SIZE)
    {
        kpage_dir[i] = 0;
        kpage_table[i] = phys | PT_PRESENT | PT_WRITABLE;
        ident_page_table[i] = PT_PRESENT | PT_WRITABLE;
    }

    // set kernel pde for identity page table and kernel page table to new addr
	kpage_dir[0] = (uintptr_t) ident_page_table | PT_PRESENT | PT_WRITABLE;
	int i = (u32) &start / 0x400000; // index into kernel page directory that maps the kernel page table
    kpage_dir[i] = (uintptr_t) kpage_table | PT_PRESENT | PT_WRITABLE;

    // identity map final entry of kernel page directory
    kpage_dir[1023] = (uintptr_t) kpage_dir | PT_PRESENT | PT_WRITABLE;

	// move physical address of kernel page directory to cr3
	asm("mov %0, %%cr3" :: "r"((uintptr_t) kpage_dir));

	nullproc.pdir = (uintptr_t) kpage_dir;
	kmalloc_init(heap, 1024 * 1024);
}

/**
 * @brief allocate virtual memory
 * @param virt page aligned virtual address to start mapping from
 * @param count number of pages to allocate
 * @return beginning of mapped memory
 * 
 * @note the granularity of this allocator is the size of a page
 */
void *vmm_alloc(uintptr_t virt, size_t count)
{
	uintptr_t v = virt;
	for (size_t i = 0; i < count; i++)
	{
		// allocate physical memory for this page
		uintptr_t phys = pmm_alloc();

		// figure out which index in the kernel page table the page corresponds to
		int idx = (v - (u32) &start) / PAGE_SIZE;

		struct pte *page = &kpage_table[idx];

		page->present = 1;
		page->addr = phys >> 12;
		v += PAGE_SIZE;
	}

	return (void *) virt;
}

uintptr_t vmm_create_address_space()
{
	uintptr_t phys = pmm_alloc();
	struct pde *dir = NULL;

	for (int i = 0; i < NUM_TABLE_ENTRIES; i++)
	{
		if (kpage_table[i].present == 0)
		{
			kprintf("%d %x\n",i, kpage_dir[i].addr);
			kpage_table[i].present = 1;
			kpage_table[i].addr = phys >> 12;
			dir = (struct pde *) &kpage_table[i];
			break;
		}
	}

	if (dir == NULL)
		kprintf("Error creating address space!\n");
	
	memcpy(dir, kpage_dir, PAGE_DIR_SIZE);
	return phys;
}

/**
 * @brief page fault handler
 */
static void page_fault()
{
	kprintf("Page fault detected!\n");
	while (1) ;
}
