// main.c

#include "kernel.h"

TCB taskA;
TCB *Running;

void task1(void);

void TimerInt(void) {}

int main(void) {

int a = 1;
int b = 2;
int c = a + b;
c += c;
  Running = &taskA;
  Running->PC = task1;
  Running->SP = &(Running->StackSeg[STACK_SIZE - 1]);

  LoadContext();
}

void task1(void) {
  register int reg_var = 1;
  volatile int vol_var = 1;

  SaveContext(); // reg_var = 1 vol_var = 1;
  reg_var++;
  vol_var++;

  SaveContext(); // reg_var = 2 vol_var = 2;
  reg_var++;
  vol_var++;

  LoadContext();
}
