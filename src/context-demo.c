// main.c

#include "tcb_chain.h"
#include <time.h>
#include <stdlib.h>

TCB taskA;
TCB *Running;

void task1();
void task2();
void task3();

void TimerInt() {}

int main() {

  TCB_chain* tcb_chain = create_tcb_chain();

  TCB taskA;
  taskA.PC = &task1;
  taskA.SP = &taskA.StackSeg[STACK_SIZE - 1];

  TCB taskB;
  taskB.PC = &task2;
  taskB.SP = &taskB.StackSeg[STACK_SIZE - 1];

  TCB taskC;
  taskC.PC = &task3;
  taskC.SP = &taskC.StackSeg[STACK_SIZE - 1];

  tcb_append(tcb_chain, create_tcb_node(&taskA));
  tcb_append(tcb_chain, create_tcb_node(&taskB));
  tcb_append(tcb_chain, create_tcb_node(&taskC));

  srand(time(NULL));

  int i = 0;
  while (i++ < 10) {
    int r = rand() % 3;
    Running = tcb_get_data(tcb_chain, r);
    LoadContext();
  }
  tcb_destroy_chain(tcb_chain);

  return 0;
}

void task1(void) {
  const char* str = "I am task1";
  str++;
}

void task2() {
  const char* str = "I am task2";
  str++;
}

void task3() {
  const char* str = "I am task3";
  str++;
}
