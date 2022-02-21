/* maestro
 * License: GPLv2
 * See LICENSE.txt for full license text
 * Author: Noah Bennett
 *
 * FILE: kmalloc.test.c
 * DATE: Augh 31, 2021
 * DESCRIPTI N: test kmalloc
 */
#include "../include/kmalloc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "../src/kmalloc.c"

size_t __s = 10000;
uintptr_t heap;
void *block_head = NULL;

void *brk(int amt)
{
	u32 ptr = heap;
	heap += amt;
	return (void *) ptr;
}

/**
 * set a block of memory as free as in libre
 * @param ptr pointer to the memory block to mark as free
 */
void kfree(void *ptr)
{
	if (!ptr)
	{
		// ignore null pointers :o
		return;
	}

	struct mem_block *freeing_block_ptr = get_block_ptr(ptr);

	// assign block as free
	// debug will be 0xBAADF00D if successfully freed
	// the block previous to the block that is being freed
	// is assigned to the next block to prevent 'gaps' in the
	// memory that is available
	freeing_block_ptr->free             = 1;
	freeing_block_ptr->debug            = 0xBAADF00D;

	if (!freeing_block_ptr->prev)
		return;
	freeing_block_ptr->prev->next = freeing_block_ptr->next;
}

/**
 * kernel malloc
 * @param size malloc size in n bytes
 */
void *kmalloc(size_t size)
{
	struct mem_block *i_am;
	// todo: handle alignment with page
	// actually i thhink it works with kmalloca

	// if someone is playing games... return null
	if (size <= 0)
	{
		return NULL;
	}

	if (!block_head)
	{
		// if head is undefined, make request for memory
		// if denied return null, otherwise we know
		// the memory block head is defined
		i_am = request_memory(NULL, size);
		if (!i_am)
		{
			return NULL;
		}
	}
	else
	{
		// keep previous block head, next block head is
		// defined when finding the next free
		struct mem_block *was = block_head;
		i_am                  = find_next_free(&was, size);

		if (!i_am)
		{
			// request more mem if no free blocks were found
			i_am = request_memory(was, size);
			if (!i_am)
			{
				// if request from was was denied return null
				return NULL;
			}
		}
		else
		{
			// successfully found a free block
			i_am->free  = 0;
			i_am->debug = 0xBAD1DEED;
			i_am->prev  = was;
		}
	}

	// return the available memory
	return (++i_am);
}

/**
 * @param size size of malloc in n bytes
 * @param align boolean indicating whether or not the
 * address returned should be aligned to page boundary
 */
void *kmalloca(size_t size)
{
	if (heap & 0xFFFFF000)
	{
		heap &= 0xFFFFF000;

		// if the previous operation rounds down, we don't want to return any memory
		// that may have already been kmalloc'd. so, we'll add a page size to ensure
		// this memory is new.
		heap += _PAGE_SIZE;
	}

	return kmalloc(size);
}

// kmalloc phys - fills the phys pointer with the physical address of the returned memory
void *kmallocp(size_t nbytes, u32 *phys)
{
	u32 ptr = kmalloc(nbytes);
	if (phys)
		*phys = ptr;

	return (void *) ptr;
}

/**
 * resize a block of memory
 * @param ptr pointer to the block of memory
 * @param size new size to allocate to
 */
void *krealloc(void *ptr, size_t size)
{
	if (!ptr)
	{
		// if null pointer is passed, realloc functions as malloc
		return kmalloc(size);
	}

	// get the current block
	struct mem_block *my = get_block_ptr(ptr);
	if (my->size > size)
	{
		// decide for caller that there's already enough memory...
		// blocks could split here in the future, thus freeing some
		// of the memory that is unused in this block, but for now
		// that isn't necessary
		return ptr;
	}

	// initialize new block
	void *new_block_ptr = kmalloc(size);
	if (!new_block_ptr)
	{
		// failled to malloc
		return NULL;
	}

	// copy memory from old block to new block, then
	// free the old block
	memcpy(new_block_ptr, ptr, my->size);
	kfree(ptr);
	return new_block_ptr;
}

/**
 * clear an allocation n elements
 * @param n number of elements
 * @param size_el size of each element
 */
void *kcalloc(size_t len, size_t size_el)
{
	size_t size = len * size_el;
	void *ptr;
	if (size > heap)
	{
		// check for overflow
		return NULL;
	}

	// initialize allocation to zero
	ptr = kmalloc(size);
	memset(ptr, 0, size);
	return ptr;
}

/**
 * find the next available memory block, if one exists
 * @param was double pointer to previous memory block
 * @param size size that is currently trying to be malloc'd 
 */
struct mem_block *find_next_free(struct mem_block **was, size_t size)
{
	struct mem_block *i_am = block_head;
	int i                  = 0;
	for (;;)
	{
		// iterate through the list until free mem is found
		if (i_am && i_am->free && i_am->size >= size)
		{
			// if block is not null, and the block is free, and
			// the block is of a valid size, correct block
			// has been found
			break;
		}

		*was = i_am;
		i_am = i_am->next;
		i++;
	}

	// return the found free block
	return i_am;
}

/**
 * ask `brk` for more memory
 * @param was pointer to the previous memory block
 * @param size n bytes to request
 */
struct mem_block *request_memory(struct mem_block *was, size_t size)
{
	struct mem_block *block;
	block               = brk(0);
	void *requested_mem = brk(sizeof(*block) + size);
	if (requested_mem == (void *) -1)
	{
		// failed to fetch more memory
		return NULL;
	}

	if (was)
	{
		// was will always be null on the first request
		// because we're starting with the head of the list
		was->next = block;
	}

	// prepend this block to the head of the heap and
	// append the new tail of the list to this block
	block->next  = NULL;
	block->prev  = was;
	block->size  = size;
	block->free  = 0;
	block->debug = 0xBADDDD1E;
	return block;
}

/**
 * get pointer to the block of memory
 * just does pointer - 1 but is more semantic / easier
 * to read than pointer arithmetic and casting to struct
 * each time
 * @param ptr pointer to get block of memory for
 */
struct mem_block *get_block_ptr(void *ptr)
{
	return ((struct mem_block *) ptr) - 1;
}

int main()
{
	heap = malloc(__s);
	char *str;
	str = kmalloc(11 * sizeof(*str));
	memcpy(str, "Hello world..?", strlen("Hello world..?"));
	printf("Running a test on chars: \n");
	printf("\nafter filling str\n");
	printf("Char is filled with: \n");
	for (int i = 0; i < 14; i++)
	{
		printf("%c", str[i]);
	}
	printf("\n");
	printf("memory block free bit (*str): %d\n", get_block_ptr(str)->free);
	printf("memory block debug value (*str): %x\n", get_block_ptr(str)->debug);

	kfree(str);
	printf("\nafter freeing str (should be the same as above bc its just marked as free):\n");
	printf("Char is filled with: \n");
	for (int i = 0; i < 14; i++)
	{
		printf("%c", str[i]);
	}

	memcpy(str, "Big Chungis", strlen("Big Chungis"));
	printf("\nafter reassigning str:\n");
	printf("Char is filled with: \n");
	for (int i = 0; i < 14; i++)
	{
		printf("%c", str[i]);
	}
	printf("\n");
	printf("memory block free bit (*str): %d\n", get_block_ptr(str)->free);
	printf("memory block debug value (*str): %x\n", get_block_ptr(str)->debug);

	printf("Finding 7! (factorial (but filling the array with each number))\n");
	printf("filling the array: ");
	int *fac = kmalloc(8 * sizeof(*fac));
	for (int i = 1; i <= 7; i++)
	{
		fac[i - 1] = i;
		if (i < 7)
			printf("%d, ", i);
		else
			printf("%d", i);
	}
	printf("\n");
	printf("memory block free bit (*fac): %d\n", get_block_ptr(fac)->free);
	printf("memory block debug value (*fac): %x\n", get_block_ptr(fac)->debug);

	printf("7! = ");
	for (int i = 1; i <= 7; i++)
	{
		if (i < 7)
			printf("%d * ", fac[i - 1]);
		else
			printf("%d", fac[i - 1]);
	}
	printf("\n");

	int sol = 1;
	for (int i = 1; i < 7; i++)
	{
		sol *= fac[i];
	}
	printf("7! = %d\n", sol);

	printf("Now finding 10!\n");
	printf("filling the array: ");
	fac = krealloc(fac, 11 * sizeof(*fac));
	for (int i = 1; i < 11; i++)
	{
		fac[i] = i + 1;
		if (i < 11)
			printf("%d, ", i);
		else
			printf("%d", i);
	}
	printf("\n");
	printf("memory block free bit (*fac): %d\n", get_block_ptr(fac)->free);
	printf("memory block debug value (*fac): %x\n", get_block_ptr(fac)->debug);

	printf("10! = ");
	for (int i = 1; i < 11; i++)
	{
		if (i < 10)
			printf("%d * ", fac[i - 1]);
		else
			printf("%d", fac[i - 1]);
	}
	printf("\n");

	kfree(fac);
	printf("freed the array of numbers :)\n");
	printf("memory block free bit (*fac): %d\n", get_block_ptr(fac)->free);
	printf("memory block debug value (*fac): %x\n", get_block_ptr(fac)->debug);

	return 0;
}
