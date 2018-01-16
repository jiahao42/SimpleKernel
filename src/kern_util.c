#include "kern_util.h"
#include <stdlib.h>

static unsigned int mem_used;

void *safe_malloc(unsigned int size) {
  void *mem = malloc(size);
  if (mem == NULL) {
    return NULL;
  } else {
    mem_used += size;
    return mem;
  }
}

void safe_free(void *pt) {
  mem_used -= sizeof(*pt);
  free(pt);
}