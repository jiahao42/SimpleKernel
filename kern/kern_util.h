#ifndef LAMB_KERNEL_UTILS_
#define LAMB_KERNEL_UTILS_

#define MEM_DEBUG

extern unsigned int malloc_counter;
extern unsigned int free_counter;

void* safe_malloc(unsigned int size);
void safe_free(void* pt);

#endif