// main.c
#include "kernel.h"
#include "string.h"

TCB *Running;

void task1(void);

int main(void)
{
  TCB* tcb = malloc(sizeof(TCB));
  tcb->SPSR = 0;
  memset(tcb->Context, 0, sizeof(*tcb->Context) * CONTEXT_SIZE);
  memset(tcb->StackSeg, 0, sizeof(*tcb->StackSeg) * STACK_SIZE);
  tcb->PC = task1;
  tcb->SP = &(tcb->StackSeg[STACK_SIZE - 1]);

	Running = tcb;

	Running->PC();
}

void task1(void)
{
	volatile int first = 1;
  int second = 0;

	SaveContext();
  second += first;
  second += second;
  if (first) {
    first = 0;
    LoadContext();
  }
  first = 2;
}

