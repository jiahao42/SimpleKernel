#include "kernel.h"

void terminate(void);

TCB *Running;
TCB_list* ready_list;
uint kernel_mode;

exception init_kernel() {
  tick_counter = 0;
  ready_list = create_tcb_list();
  if (ready_list == NULL) {
    return fail;
  }
  kernel_mode = INIT;
  return success;
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
  return success;
}

void idle() {
  while(1);
}

/*
Initialize interrupt timer
Set the kernel in running mode
Enable interrupts
Load context
*/
void run(void) {
  kernel_mode = RUNNING;
  set_isr(ISR_ON);
  Running = tcb_get_data(ready_list, 0);
  Running->PC();
}