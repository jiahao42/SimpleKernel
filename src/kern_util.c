#include "kern_util.h"
#include <stdlib.h>

static unsigned int malloc_counter;
static unsigned int free_counter;

void *safe_malloc(unsigned int size) {
  void *mem = malloc(size);
  if (mem == NULL) {
    return NULL;
  } else {
    #ifdef MEM_DEBUG
      printf("**** malloc[%d] ****\n", malloc_counter++);
    #endif
    return mem;
  }
}

void safe_free(void *pt) {
  #ifdef MEM_DEBUG
    printf("**** free[%d] ****\n", free_counter++);
  #endif
  free(pt);
  pt = NULL;
}