#ifndef LAMB_KERNEL_ERROR_H_
#define LAMB_KERNEL_ERROR_H_

#include "stdio.h"

#define OS_ERROR(e)  \
  if (e) { \
    do {  \
      printf("OS-ERROR in file: (%s), on line: %d\n", __FILE__, __LINE__); \
    } while (1); \
  }

typedef enum {
  E_NO_MEM = 1, // run out of memory
}error;


#endif