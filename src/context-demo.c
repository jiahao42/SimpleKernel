// main.c

#include "kernel.h"

TCB taskA;
TCB *Running;

void task1(void);

void TimerInt(void) {}

int main(void) {

  Running = &taskA;
  Running->PC = task1;
  Running->SP = &(Running->StackSeg[STACK_SIZE - 1]);

  LoadContext();
}

void task1(void) {
  register int reg_var = 1;
  volatile int vol_var = 1;

  SaveContext();
  reg_var++;
  vol_var++;

  SaveContext();
  reg_var++;
  vol_var++;

  LoadContext();
}
