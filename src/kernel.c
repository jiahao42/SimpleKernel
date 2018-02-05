#include "kernel.h"
#include "string.h"
#include "stdio.h"

#define OS_ERROR(e)  \
  if (e) { \
    do {  \
      printf("OS-ERROR [%s] in file: (%s), on line: %d\n", e, __FILE__, __LINE__); \
    } while (0); \
  }

static const char* mem_alloc_fail = "Memory allocation failed!";
#define NULL_CHECKER(p) \
  if (p == NULL) { \
    kernel_status = FAIL; \
    OS_ERROR(mem_alloc_fail); \
  }


/*********
 * Utils *
 *********/

static unsigned int mem_counter;
void* safe_malloc(unsigned int size);
void safe_free(void* pt);

void *safe_malloc(unsigned int size) {
  void *mem = malloc(size);
  if (mem == NULL) {
    return NULL;
  } else {
    #ifdef MEM_DEBUG
      // printf("**** malloc[%d] ****\n", malloc_counter);
      mem_counter++;      
    #endif
    return mem;
  }
}

void safe_free(void *pt) {
  #ifdef MEM_DEBUG
    // printf("**** free[%d] ****\n", free_counter);
    mem_counter--;
  #endif
  free(pt);
  pt = NULL;
}


/*************************
 * TCB list manipulation *
 *************************/

TCB* create_TCB();
void init_listobj(listobj *node, TCB *data);
listobj *create_listobj(TCB *data);
void destroy_listobj(listobj *node);
list *create_list();
void list_append(list *t_list, listobj *node);
void list_prepend(list *t_list, listobj *node);
void list_insert_after(list *t_list, listobj *pos, listobj *n_node);
void list_insert_before(list *t_list, listobj *pos, listobj *n_node);
void list_insert_by_ddl(list *t_list, listobj *n_node);
void destroy_list(list *t_list);


TCB* create_TCB() {
  TCB* tcb = safe_malloc(sizeof(TCB));
  if (tcb == NULL)
    return NULL;
  tcb->SPSR = 0;
  memset(tcb->Context, 0, sizeof(*tcb->Context) * CONTEXT_SIZE);
  memset(tcb->StackSeg, 0, sizeof(*tcb->StackSeg) * STACK_SIZE);
  return tcb;
}
void init_listobj(listobj *node, TCB *data) {
  node->pTask = data;
  node->pNext = NULL;
  node->pPrevious = NULL;
}

listobj *create_listobj(TCB *data) {
  listobj *node = (listobj *)safe_malloc(sizeof(listobj));
  if (node == NULL) 
    return NULL;
  init_listobj(node, data);
  return node;
}

void destroy_listobj(listobj *node) {
  safe_free(node);
  node->pNext = NULL;
  node->pPrevious = NULL;
}

list *create_list() {
  list *t_list = (list *)safe_malloc(sizeof(list));
  if (t_list == NULL)
    return NULL;
  t_list->pHead = NULL;
  t_list->pTail = NULL;
  return t_list;
}

void list_append(list *t_list, listobj *node) {
  if (t_list->pTail == NULL) {
    t_list->pHead = node;
    t_list->pTail = node;
  } else {
    t_list->pTail->pNext = node;
    node->pPrevious = t_list->pTail;
    node->pNext = NULL;
    t_list->pTail = node;
  }
}

void list_prepend(list *t_list, listobj *node) {
  if (t_list->pHead == NULL) {
    t_list->pHead = node;
    t_list->pTail = node;
  } else {
    node->pPrevious = NULL;
    node->pNext = t_list->pHead;
    t_list->pHead->pPrevious = node;
    t_list->pHead = node;
  }
}

void list_insert_after(list *t_list, listobj *pos, listobj *n_node) {
  if (pos->pNext == NULL) {
    list_append(t_list, n_node);
    return;
  }
  n_node->pNext = pos->pNext;
  n_node->pPrevious = pos;
  pos->pNext = n_node;
  n_node->pNext->pPrevious = n_node;
}

void list_insert_before(list *t_list, listobj *pos, listobj *n_node) {
  if (pos->pPrevious == NULL) {
    list_prepend(t_list, n_node);
    return;
  }
  n_node->pPrevious = pos->pPrevious;
  n_node->pNext = pos;
  pos->pPrevious->pNext = n_node;
  pos->pPrevious = n_node;
}

void list_insert_by_ddl(list *t_list, listobj *n_node) {
  if (t_list->pHead == NULL) { // empty list
    list_append(t_list, n_node);
  } else if (n_node->pTask->DeadLine < t_list->pHead->pTask->DeadLine) {
    list_prepend(t_list, n_node);
  } else if (n_node->pTask->DeadLine > t_list->pTail->pTask->DeadLine) {
    list_append(t_list, n_node);
  } else {
    listobj *cursor = t_list->pHead->pNext;
    while (cursor) {
      if (n_node->pTask->DeadLine > cursor->pTask->DeadLine) {
        list_append(t_list, n_node);
        break;
      }
      cursor = cursor->pNext;
    }
  }
}


void destroy_list(list *t_list) {
  listobj *cursor = t_list->pHead;
  for (; t_list->pHead != NULL;) {
    cursor = t_list->pHead;
    t_list->pHead = t_list->pHead->pNext;
    safe_free(cursor);
  }
  safe_free(t_list);
}

/******************
 * Timer relevent *
 ******************/
void TimerInt (void)
{

}
/******************************
 * Kernel task administration *
 ******************************/

static uint tick_counter;
static uint kernel_mode;
static uint kernel_status;
list* ready_list;
list* waiting_list;
list* timer_list;

int init_kernel();
void idle();
TCB* Running;

int init_kernel() {
  tick_counter = 0;
  ready_list = create_list();
  NULL_CHECKER(ready_list);
  waiting_list = create_list();
  NULL_CHECKER(waiting_list);
  timer_list = create_list();
  NULL_CHECKER(timer_list);
  TCB* t_idle = create_TCB();
  NULL_CHECKER(t_idle);
  t_idle->PC = idle;
  t_idle->SP = &(Running->StackSeg[STACK_SIZE-1]);
  kernel_mode = INIT;
  return kernel_status;
}

void idle() {
  //TODO: add texas_dsp macro
  while (1) {
    tick_counter++;
    TimerInt();
  }
}

exception	create_task(void (* body)(), uint d) {
  TCB* tcb = create_TCB();
  NULL_CHECKER(tcb);
  if (kernel_status != OK)
    return kernel_status;
  tcb->DeadLine = d;
  tcb->PC = body;
  tcb->SP = &(tcb->StackSeg[STACK_SIZE - 1]);
  if (kernel_mode == INIT) {
    list_insert_by_ddl(ready_list, create_listobj(tcb));
    return kernel_status;
  } else { // RUNNING
    // TODO: disable interrupt
    SaveContext();
    if () {

    }
  }
  
}