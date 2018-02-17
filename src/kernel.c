#include "kernel.h"
#include "stdio.h"
#include "string.h"

// TODO:
// Turn off interrupts on memory handling (malloc/calloc/free is not reentrant).

#define OS_ERROR(e)                                                            \
  if (e) {                                                                     \
    do {                                                                       \
      printf("OS-ERROR [%s] in file: (%s), on line: %d\n", e, __FILE__,        \
             __LINE__);                                                        \
    } while (0);                                                               \
  }

static const char *mem_alloc_fail = "Memory allocation failed!";
#define NULL_CHECKER(p)                                                        \
  if (p == NULL) {                                                             \
    do {                                                                       \
      kernel_status = FAIL;                                                    \
      OS_ERROR(mem_alloc_fail);                                                \
    } while (0);                                                               \
  }

/*********
 * Utils *
 *********/

// #define MEM_DEBUG

static unsigned int mem_counter;

void *safe_malloc(unsigned int size);
void safe_free(void *pt);

void *safe_malloc(unsigned int size) {
  void *mem = malloc(size);
  if (mem == NULL) {
    return NULL;
  } else {
    // printf("**** malloc[%d] ****\n", malloc_counter);
    mem_counter++;
    return mem;
  }
}

void safe_free(void *pt) {
  // printf("**** free[%d] ****\n", free_counter);
  mem_counter--;
  free(pt);
  pt = NULL;
}

/*************************
 * TCB list manipulation *
 *************************/

TCB *create_TCB();
void init_listobj(listobj *node, TCB *data);
listobj *create_listobj(TCB *data);
void destroy_listobj(listobj *node);
list *create_list();
void list_append(list *t_list, listobj *node);
void list_prepend(list *t_list, listobj *node);
void list_insert_after(list *t_list, listobj *pos, listobj *n_node);
void list_insert_before(list *t_list, listobj *pos, listobj *n_node);
void list_insert_by_ddl(list *t_list, listobj *n_node);
void list_remove_head(list *t_list);
void list_remove_tail(list *t_list);
TCB *list_get_head_task(list *t_list);
listobj *list_get_head(list *t_list);
uint if_node_in_list(list *l, listobj *node);
void node_remove(list *t_list, listobj *node);
void node_destroy_by_task(list *t_list, TCB *tcb);
listobj *node_fetch_by_task(list *t_list, TCB *tcb);
void node_transfer_list(list *src, list *dest, listobj *node);
void destroy_list(list *t_list);

TCB *create_TCB() {
  TCB *tcb = safe_malloc(sizeof(TCB));
  if (tcb == NULL)
    return NULL;
  tcb->SPSR = 0;
  memset(tcb->Context, 0, sizeof(*tcb->Context) * CONTEXT_SIZE);
  memset(tcb->StackSeg, 0, sizeof(*tcb->StackSeg) * STACK_SIZE);
  tcb->DeadLine = 0xffffffff;
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
  safe_free(node->pTask);
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
      if (n_node->pTask->DeadLine < cursor->pTask->DeadLine) {
        list_insert_before(t_list, cursor, n_node);
        break;
      }
      cursor = cursor->pNext;
    }
  }
}

/*
remove the node from list,
whether free the node is up to the caller
*/
void list_remove_head(list *t_list) {
  listobj *node = t_list->pHead;
  t_list->pHead =
      node->pNext; // could be NULL when there is only one node in this list
  if (t_list->pHead == NULL) { // if the list is empty now
    t_list->pTail = NULL;
  } else {
    t_list->pHead->pPrevious = NULL;
  }
}

void list_remove_tail(list *t_list) {
  listobj *node = t_list->pTail;
  t_list->pTail = node->pPrevious; // could not be NULL here
  t_list->pTail->pNext = NULL;
}

TCB *list_get_head_task(list *t_list) { return t_list->pHead->pTask; }

listobj *list_get_head(list *t_list) { return t_list->pHead; }

uint if_node_in_list(list *l, listobj *node) {
  listobj *cursor = l->pHead;
  for (; cursor != NULL; cursor = cursor->pNext) {
    if (cursor == node) {
      return TRUE;
    }
  }
  return FALSE;
}

void node_remove(list *t_list, listobj *node) {
  if (node == t_list->pHead) {
    return list_remove_head(t_list);
  } else if (node == t_list->pTail) {
    return list_remove_tail(t_list);
  } else {
    node->pPrevious->pNext = node->pNext;
    node->pNext->pPrevious = node->pPrevious;
  }
}

/*
Careful! This function will free node
*/
void node_destroy_by_task(list *t_list, TCB *tcb) {
  listobj *cursor = t_list->pHead;
  while (cursor != NULL) {
    if (cursor->pTask == tcb) {
      node_remove(t_list, cursor);
      destroy_listobj(cursor);
      break;
    } else {
      cursor = cursor->pNext;
    }
  }
}

listobj *node_fetch_by_task(list *t_list, TCB *tcb) {
  listobj *cursor = t_list->pHead;
  while (cursor != NULL) {
    if (cursor->pTask == tcb) {
      return cursor;
    } else {
      cursor = cursor->pNext;
    }
  }
  return NULL;
}

void node_transfer_list(list *src, list *dest, listobj *node) {
  node_remove(src, node);
  list_insert_by_ddl(dest, node);
}

void destroy_list(list *t_list) {
  listobj *cursor = t_list->pHead;
  for (; t_list->pHead != NULL;) {
    cursor = t_list->pHead;
    safe_free(cursor->pTask);
    t_list->pHead = t_list->pHead->pNext;
    safe_free(cursor);
  }
  safe_free(t_list);
}

/******************
 * Timer relevent *
 ******************/
TCB *Running;
static uint kernel_mode;
static uint kernel_status;
list *ready_list;
list *waiting_list;
list *timer_list;
static uint tick_counter;

exception wait(uint nTicks);
void set_ticks(uint no_of_ticks);
uint ticks();
uint deadline();
void set_deadline(uint nNew);
void TimerInt();

/*
This call will block the calling task until the given
number of ticks has expired.
*/
exception wait(uint nTicks) {
#ifdef texas_dep
  isr_off();
#endif
  volatile uint first_execute = TRUE;
  SaveContext();
  if (first_execute == TRUE) {
    first_execute = FALSE;
    listobj *node = node_fetch_by_task(ready_list, Running);
    node->nTCnt = nTicks;
    node_transfer_list(ready_list, timer_list, node);
    LoadContext();
  } else {
    if (ticks() > deadline()) {
      kernel_status = DEADLINE_REACHED;
    } else {
      kernel_status = OK;
    }
  }
  return kernel_status;
}

#define LIST_FOR_EACH(list)                                                    \
  for (listobj *item = list->pHead; item != list->pTail; item = item->pNext)

void TimerInt(void) {
  tick_counter++;
  LIST_FOR_EACH(timer_list) {
    item->nTCnt--;
    if (item->nTCnt == 0) {
      node_transfer_list(timer_list, ready_list, item);
    }
  }
  LIST_FOR_EACH(waiting_list) {
    if (ticks() > deadline()) { // deadline expired
      node_transfer_list(waiting_list, ready_list, item);
    }
    // TODO: clear mail entry
  }
}

void set_ticks(uint no_of_ticks) { tick_counter = no_of_ticks; }

uint ticks(void) { return tick_counter; }

uint deadline() { return Running->DeadLine; }

void set_deadline(uint nNew) {
  volatile uint first_execute = TRUE;
#ifdef texas_dep
  isr_off();
#endif
  SaveContext();
  if (first_execute == TRUE) {
    first_execute = FALSE;
    Running->DeadLine = nNew;
    // reschedule readylist
    listobj *node = node_fetch_by_task(ready_list, Running);
    node_remove(ready_list, node);
    list_insert_by_ddl(ready_list, node);
    LoadContext();
  }
}

/******************************
 * Kernel task administration *
 ******************************/

int init_kernel();
void idle();
exception create_task(void (*body)(), uint d);
void run(void);
void terminate(void);

/*
This function initializes the kernel and its data
structures and leaves the kernel in start-up mode. The
init_kernel call must be made before any other call is
made to the kernel.
*/
int init_kernel() {
  kernel_status = OK;
  set_ticks(0);
  ready_list = create_list();
  NULL_CHECKER(ready_list);
  waiting_list = create_list();
  NULL_CHECKER(waiting_list);
  timer_list = create_list();
  NULL_CHECKER(timer_list);
  TCB *t_idle = create_TCB();
  NULL_CHECKER(t_idle);
  t_idle->PC = idle;
  t_idle->SP = &(Running->StackSeg[STACK_SIZE - 1]);
  t_idle->DeadLine = 0xffffffff;
  list_insert_by_ddl(ready_list, create_listobj(t_idle));
  kernel_mode = INIT;
  return kernel_status;
}

void idle() {
#ifdef texas_dep
  while (1)
    ;
#else
  TimerInt();
  Running = list_get_head_task(ready_list);
  LoadContext();
#endif
}

/*
This function creates a task. If the call is made in startup
mode, i.e. the kernel is not running, only the
necessary data structures will be created. However, if
the call is made in running mode, it will lead to a
rescheduling and possibly a context switch.
*/
exception create_task(void (*body)(), uint d) {
  volatile uint first_execute = TRUE;
  TCB *tcb = create_TCB();
  NULL_CHECKER(tcb);
  if (kernel_status != OK)
    return kernel_status;
  tcb->DeadLine = ticks() + d;
  tcb->PC = body;
  tcb->SP = &(tcb->StackSeg[STACK_SIZE - 1]);
  if (kernel_mode == INIT) {
    list_insert_by_ddl(ready_list, create_listobj(tcb));
    return kernel_status;
  } else { // RUNNING
#ifdef texas_dsp
    isr_off(); // turn off interrupt
#endif
    SaveContext();
    if (first_execute == TRUE) {
      first_execute = FALSE;
      list_insert_by_ddl(ready_list, create_listobj(tcb));
      LoadContext();
    }
  }
  return kernel_status;
}

/*
This function starts the kernel and thus the system of
created tasks. Since the call will start the kernel it will
leave control to the task with tightest deadline.
Therefore, it must be placed last in the application
initialization code. After this call the system will be in
running mode.
*/
void run() {
  kernel_mode = RUNNING;
  Running = list_get_head_task(ready_list);
#ifdef texas_dsp
  isr_on();
#endif
  LoadContext();
}

/*
This call will terminate the running task. All data
structures for the task will be removed. Thereafter,
another task will be scheduled for execution.
*/
void terminate() {
  node_destroy_by_task(ready_list, Running);
  Running = ready_list->pHead->pTask;
  LoadContext();
}

/************************
 * Mailbox manipulation *
 ************************/
void mailbox_push_wait_msg(mailbox *mBox, msg *m);
void mailbox_push_no_wait_msg(mailbox *mBox, msg *m);
msg *mailbox_pop_wait_msg(mailbox *mBox);
msg *mailbox_pop_no_wait_msg(mailbox *mBox);

void mailbox_push_wait_msg(mailbox *mBox, msg *m) {
  if (mBox->nBlockedMsg == 0) { // init empty mailbox
    m->pPrevious = NULL;
    m->pNext = NULL;
    mBox->pHead = m;
    mBox->pTail = m;
    mBox->nBlockedMsg = 1;
  } else {
    m->pPrevious = mBox->pTail;
    m->pNext = NULL;
    mBox->pTail->pNext = m;
    mBox->pTail = m;
    mBox->nBlockedMsg++;
  }
}

void mailbox_push_no_wait_msg(mailbox *mBox, msg *m) {
  if (mBox->nMessages == 0) { // init empty mailbox
    m->pPrevious = NULL;
    m->pNext = NULL;
    mBox->pHead = m;
    mBox->pTail = m;
    mBox->nMessages = 1;
  } else if (mBox->nMessages == mBox->nMaxMessages) { // if mailbox is full
    mailbox_pop_no_wait_msg(mBox); // remove the oldest msg
    mailbox_push_no_wait_msg(mBox, m); // recursive :)
  } else {
    m->pPrevious = mBox->pTail;
    m->pNext = NULL;
    mBox->pTail->pNext = m;
    mBox->pTail = m;
    mBox->nMessages++;
  }
}

msg *mailbox_pop_wait_msg(mailbox *mBox) {
  if (mBox->nBlockedMsg == 0) { // error: pop from empty mailbox
    return NULL;
  } else if (mBox->nBlockedMsg == 1) { // only 1 message
    msg *m = mBox->pHead;
    mBox->pHead = NULL;
    mBox->pTail = NULL;
    mBox->nBlockedMsg = 0;
    return m;
  } else {
    msg *m = mBox->pTail;
    mBox->pTail = mBox->pTail->pPrevious;
    mBox->nBlockedMsg--;
    return m;
  }
}

msg *mailbox_pop_no_wait_msg(mailbox *mBox) {
  if (mBox->nMessages == 0) { // error: pop from empty mailbox
    return NULL;
  } else if (mBox->nMessages == 1) { // only 1 message
    msg *m = mBox->pHead;
    mBox->pHead = NULL;
    mBox->pTail = NULL;
    mBox->nMessages = 0;
    return m;
  } else {
    msg *m = mBox->pTail;
    mBox->pTail = mBox->pTail->pPrevious;
    mBox->nMessages--;
    return m;
  }
}

/*******
 * IPC *
 *******/

mailbox *create_mailbox(uint nMessages, uint nDataSize);
int no_messages(mailbox *mBox);
exception remove_mailbox(mailbox *mBox);
exception send_wait(mailbox *mBox, void *pData);
exception receive_wait(mailbox *mBox, void *pData);
exception send_no_wait(mailbox *mBox, void *pData);
int receive_no_wait(mailbox *mBox, void *pData);

mailbox *create_mailbox(uint nMessages, uint nDataSize) {
  mailbox *mb = safe_malloc(sizeof(mailbox));
  mb->pHead = NULL;
  mb->pTail = NULL;
  mb->nDataSize = nDataSize;
  mb->nMaxMessages = nMessages;
  mb->nBlockedMsg = 0;
  mb->nMessages = 0;
  return mb;
}

int no_messages(mailbox *mBox) { return mBox->nMessages == 0 ? TRUE : FALSE; }

exception remove_mailbox(mailbox *mBox) {
  if (no_messages(mBox)) {
    safe_free(mBox);
    return OK;
  } else {
    return NOT_EMPTY;
  }
}

exception send_wait(mailbox *mBox, void *pData) {
  volatile uint first_execute = TRUE;
#ifdef texas_dsp
  isr_off();
#endif
  SaveContext();
  if (first_execute == TRUE) {
    first_execute = FALSE;
    if (mBox->nBlockedMsg > 0 && mBox->pHead->Status == RECEIVER) {
      msg *m = mailbox_pop_msg(mBox);
      // m->pData = pData;
      m->pData = malloc(mBox->nDataSize);
      NULL_CHECKER(m->pData);
      memcpy(m->pData, pData, mBox->nDataSize);
      node_transfer_list(waiting_list, ready_list, m->pBlock);
    } else { // if no task is waiting
      msg *m = safe_malloc(sizeof(msg));
      listobj *node = list_get_head(ready_list);
      NULL_CHECKER(m);
      node->pMessage = m;
      m->Status = SENDER;
      m->pData = pData;
      m->pBlock = node;
      mailbox_push_msg(mBox, m);
      node_transfer_list(ready_list, waiting_list, node);
      Running = list_get_head_task(ready_list);
    }
    LoadContext();
  } else {
    if (deadline() < ticks()) { // deadline reached
#ifdef texas_dsp
      isr_off();
#endif
      listobj *node = list_get_head(ready_list);
      // TODO
      safe_free(node->pMessage);
#ifdef texas_dsp
      isr_on();
#endif
      return DEADLINE_REACHED;
    } else {
      return OK;
    }
  }
}

exception receive_wait(mailbox *mBox, void *pData) {
  volatile uint first_execution = TRUE;
#ifdef texas_dsp
  isr_off();
#endif
  SaveContext();
  if (first_execution == TRUE) {
    first_execution = FALSE;
    if (mBox->nBlockedMsg > 0 && mBox->pHead->Status == SENDER) {
      msg *m = mailbox_pop_msg(mBox);
      NULL_CHECKER(m);
      pData = malloc(mBox->nDataSize);
      memcpy(pData, m->pData, mBox->nDataSize);
      if (if_node_in_list(waiting_list, m->pBlock) == TRUE) {
        node_transfer_list(waiting_list, ready_list, m->pBlock);
      } else { // not wait type
        safe_free(m->pData);
      }
    } else { // no msg waiting
      msg *m = safe_malloc(sizeof(msg));
      listobj *node = list_get_head(ready_list);
      node->pMessage = m;
      m->Status = RECEIVER;
      m->pBlock = node;
      node_transfer_list(ready_list, waiting_list, node);
    }
    LoadContext();
  } else { // not first execution
    if (deadline() < ticks()) {
#ifdef texas_dsp
      isr_off();
#endif
      listobj *node = list_get_head(ready_list);
      node->pMessage = NULL;
      // TODO
    }
  }
}
