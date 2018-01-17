#include "tcb_chain.h"
#include <time.h>
#include <stdlib.h>

TCB *Running;
TCB_chain* tcb_chain;

void task0();
void task1();

void TimerInt() {}

int main() {

  tcb_chain = create_tcb_chain();

  TCB taskA;
  taskA.PC = &task0;
  taskA.SP = &taskA.StackSeg[STACK_SIZE - 1];

  TCB taskB;
  taskB.PC = &task1;
  taskB.SP = &taskB.StackSeg[STACK_SIZE - 1];

  tcb_append(tcb_chain, create_tcb_node(&taskA));
  tcb_append(tcb_chain, create_tcb_node(&taskB));

  srand(time(NULL));

  Running = tcb_get_data(tcb_chain, 0);
  Running->PC();

  tcb_destroy_chain(tcb_chain);

  return 0;
}

void task0(void) {
  const char* str = "I am the beginning task0";
  str++;
  SaveContext();
  Running = tcb_get_data(tcb_chain, 1);
  LoadContext();
  const char* str1 = "I am the end of task0";
  str1++;
  Running = tcb_get_data(tcb_chain, 1);
  LoadContext();
}

void task1() {
  const char* str = "I am the beginning of task1";
  str++;
  SaveContext();
  Running = tcb_get_data(tcb_chain, 0);
  LoadContext();
  const char* str1 = "I am the end of task1";
  str1++;
}
