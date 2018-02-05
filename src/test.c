// main.c
#include "kernel.h"
#include "string.h"

TCB *Running;

void task1(void);

int main(void)
{
  init_kernel();
}

