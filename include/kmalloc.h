#ifndef KMALLOC_H
#define KMALLOC_H

#include <maestro.h>

// page size
#define _PAGE_SIZE 4096

struct mem_block {
  size_t size;
  u8 free;
  struct mem_block *next;
  int debug; // delete me
};

void *brk(int);
void  kfree(void *);
void *kmalloc(size_t);
void *kmalloca(size_t);
void *kmallocp(size_t, u32 *);
void *kmallocap(size_t, int, u32 *);

struct mem_block *find_next_free(struct mem_block **, size_t);
struct mem_block *request_memory(struct mem_block *, size_t);
struct mem_block *get_block_ptr(void *);

#endif // KMALLOC
