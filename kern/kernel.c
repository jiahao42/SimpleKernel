#include "kernel.h"

// Task administration
int init_kernel(void);
exception create_task(void (*body)(), uint d);
void terminate(void);
void run(void);

uint tick_counter;
TCB *Running;
TCB_list* ready_list;
uint kernel_mode;

int init_kernel() {
  tick_counter = 0;
  ready_list = create_tcb_list();
  if (ready_list == NULL) {
    return FAIL;
  }
  kernel_mode = INIT;
  return SUCCESS;
}

exception create_task(void(*task_body)(), uint deadline) {
  TCB *tcb = (TCB*)safe_malloc(sizeof(TCB));
  tcb->DeadLine = deadline;
  tcb->PC = task_body;
  tcb->SP = &tcb->StackSeg[STACK_SIZE - 1];
  TCB_node* node = create_tcb_node(tcb);
  if (node == NULL) {
    OS_ERROR(E_NO_MEM);
  }
  if (kernel_mode == INIT) {
    tcb_append(ready_list, node);
    return success;
  } else {
    set_isr(ISR_OFF);
    SaveContext();
    // TODO
  }
}

void idle() {
  while(1);
}