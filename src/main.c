#include "kernel.h"
#include "test.h"

TCB *Running;

void task1();

int main()
{
  internal_test_suite();
  init_kernel();
  create_task(task1, 100);
  run();
}

void task1() {
  int var1 = 0;
  var1++;
  terminate();
}