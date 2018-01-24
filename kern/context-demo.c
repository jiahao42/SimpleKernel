#include "kernel.h"
#include <stdlib.h>
#include <time.h>

void task0();
void task1();


int main() {
  test_suite();

  init_kernel();

  create_task(&task0, 0);
  create_task(&task1, 1);

  run();

  tcb_destroy_chain(ready_list);

  return 0;
}

void task0(void) {
  int test  = 0;
  while (1) {

  }
  const char *str = "I am the beginning task0";
  str++;
  const char *str1 = "I am the end of task0";
  str1++;
}

void task1() {
  const char *str = "I am the beginning of task1";
  str++;
  const char *str1 = "I am the end of task1";
  str1++;
}
