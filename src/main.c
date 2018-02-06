#include "kernel.h"
#include "test.h"

TCB *Running;

void task1();
void task2();

int main() {
  internal_test_suite();
  init_kernel();
  create_task(task1, 100);
  run();
}

void task1() {
  int var1 = 0;
  create_task(task2, 200);
  var1++;
  terminate();
}

void task2() {
  int var2 = 0;
  var2++;
  terminate();
}