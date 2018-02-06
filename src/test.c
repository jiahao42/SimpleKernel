// main.c
#include "kernel.h"
#include "string.h"

TCB *Running;

void task1();

int main()
{
  init_kernel();
  create_task(task1, 100);
  run();
}

void task1() {
  int var1 = 0;
  var1++;
  terminate();
}
