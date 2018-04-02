#include "kernel.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"


/* TODO: */
/* 1. More tests, must test send_no_wait and receive_no_wait */
/* 2. Turn off interrupts on memory handling (malloc/calloc/free is **not** reentrant). */
/* 3. Check for other non-reentrant functions (such as printf) and be careful with those. */


/*
********************
*****Principles*****
********************
1. Messages are **only** created before entering the mailbox.
Related functions: send_wait, send_no_wait
2. Messages are **only** destroyed after leaving the mailbox.
Related functions: 
  mailbox_pop_no_wait_msg, mailbox_pop_wait_msg
  receive_no_wait, receive_wait
2. Tasks are only destroyed in terminate()
3. 
*/

#define TEST_MODE

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

#ifdef TEST_MODE

void isr_off(void) {}
void isr_on(void) {}

static unsigned int mem_counter;
void *safe_malloc(unsigned int size) {
  void* mem;
  isr_off();
  mem = malloc(size);
  mem_counter++;
  isr_on();
  return mem;
}

/* Something is wrong when using safe_free, maybe there is a NULL pointer need to be fixed */
#define safe_free(p)                                                           \
  do {                                                                         \
    void **__p;                                                                \
    isr_off();                                                                 \
    __p = (void **)(&p);                                                       \
    free(*(__p));                                                              \
    *(__p) = NULL;                                                             \
    mem_counter--;                                                             \
    isr_on();                                                                  \
  } while (0)

#else

void* safe_malloc(unsigned int size) {
  return malloc(size);
}

void safe_free(void* p) {
  free(p);
}

#endif
    
    
/*************************
 * TCB list manipulation *
 *************************/

TCB *create_TCB();
void init_listobj(listobj *node, TCB *data);
listobj *create_listobj(TCB *data);
void destroy_listobj(listobj *node);
list *create_list();
void list_append(list *_list, listobj *node);
void list_prepend(list *_list, listobj *node);
void list_insert_after(list *_list, listobj *pos, listobj *n_node);
void list_insert_before(list *_list, listobj *pos, listobj *n_node);
void list_insert_by_ddl(list *_list, listobj *n_node);
void list_remove_head(list *_list);
void list_remove_tail(list *_list);
TCB *list_get_head_task(list *_list);
listobj *list_get_head(list *_list);
uint if_node_in_list(list *l, listobj *node);
void node_remove(list *_list, listobj *node);
void node_destroy_by_task(list *_list, TCB *tcb);
listobj *node_fetch_by_task(list *_list, TCB *tcb);
void node_transfer_list(list *src, list *dest, listobj *node);
void destroy_list(list *_list);

TCB *create_TCB() {
  TCB *tcb = safe_malloc(sizeof(TCB));
  if (tcb == NULL)
    return NULL;
  tcb->SPSR = 0;
  memset(tcb->Context, 0, sizeof(tcb->Context));
  memset(tcb->StackSeg, 0, sizeof(tcb->StackSeg));
  tcb->DeadLine = UINT_MAX;
  return tcb;
}
void init_listobj(listobj *node, TCB *data) {
  node->pTask = data;
  node->pNext = NULL;
  node->pPrevious = NULL;
  node->pMessage = NULL;
  node->nTCnt = 0;
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
  // node->pNext = NULL;
  // node->pPrevious = NULL;
}

list *create_list() {
  list *_list = (list *)safe_malloc(sizeof(list));
  if (_list == NULL)
    return NULL;
  _list->pHead = NULL;
  _list->pTail = NULL;
  return _list;
}

void list_append(list *_list, listobj *node) {
  if (_list->pTail == NULL) { /* if the list is empty */
    _list->pHead = node;
    _list->pTail = node;
    node->pPrevious = NULL;
    node->pNext = NULL;
  } else {
    _list->pTail->pNext = node;
    node->pPrevious = _list->pTail;
    node->pNext = NULL;
    _list->pTail = node;
  }
}

void list_prepend(list *_list, listobj *node) {
  if (_list->pHead == NULL) { /* if the list is empty */
    _list->pHead = node;
    _list->pTail = node;
    node->pPrevious = NULL;
    node->pNext = NULL;
  } else {
    node->pPrevious = NULL;
    node->pNext = _list->pHead;
    _list->pHead->pPrevious = node;
    _list->pHead = node;
  }
}

void list_insert_after(list *_list, listobj *pos, listobj *n_node) {
  if (pos->pNext == NULL) {
    list_append(_list, n_node);
    return;
  }
  n_node->pNext = pos->pNext;
  n_node->pPrevious = pos;
  pos->pNext = n_node;
  n_node->pNext->pPrevious = n_node;
}

void list_insert_before(list *_list, listobj *pos, listobj *n_node) {
  if (pos->pPrevious == NULL) {
    list_prepend(_list, n_node);
    return;
  }
  n_node->pPrevious = pos->pPrevious;
  n_node->pNext = pos;
  pos->pPrevious->pNext = n_node;
  pos->pPrevious = n_node;
}

void list_insert_by_ddl(list *_list, listobj *n_node) {
  if (_list->pHead == NULL) { /* empty list */
    list_append(_list, n_node);
  } else if (n_node->pTask->DeadLine < _list->pHead->pTask->DeadLine) {
    list_prepend(_list, n_node);
  } else if (n_node->pTask->DeadLine > _list->pTail->pTask->DeadLine) {
    list_append(_list, n_node);
  } else {
    listobj *cursor = _list->pHead->pNext;
    while (cursor) {
      if (n_node->pTask->DeadLine < cursor->pTask->DeadLine) {
        list_insert_before(_list, cursor, n_node);
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
void list_remove_head(list *_list) {
  listobj *node = _list->pHead;
  _list->pHead =
      node->pNext; /* could be NULL when there is only one node in this list */
  if (_list->pHead == NULL) { /* if the list is empty now */
    _list->pTail = NULL;
  } else {
    _list->pHead->pPrevious = NULL;
  }
}

void list_remove_tail(list *_list) {
  listobj *node = _list->pTail;
  _list->pTail = node->pPrevious;
  if (_list->pTail == NULL) {
    _list->pHead = NULL;
  } else {
    _list->pTail->pNext = NULL;
  }
}

TCB *list_get_head_task(list *_list) { return _list->pHead->pTask; }

listobj *list_get_head(list *_list) { return _list->pHead; }

uint if_node_in_list(list *l, listobj *node) {
  listobj *cursor = l->pHead;
  for (; cursor != NULL; cursor = cursor->pNext) {
    if (cursor == node) {
      return TRUE;
    }
  }
  return FALSE;
}

void node_remove(list *_list, listobj *node) {
  if (node == _list->pHead) {
    list_remove_head(_list);
  } else if (node == _list->pTail) {
    list_remove_tail(_list);
  } else {
    node->pPrevious->pNext = node->pNext;
    node->pNext->pPrevious = node->pPrevious;
  }
}

/*
Careful! This function will free node
*/
void node_destroy_by_task(list *_list, TCB *tcb) {
  listobj *cursor = _list->pHead;
  while (cursor != NULL) {
    if (cursor->pTask == tcb) {
      node_remove(_list, cursor);
      destroy_listobj(cursor);
      break;
    } else {
      cursor = cursor->pNext;
    }
  }
}

listobj *node_fetch_by_task(list *_list, TCB *tcb) {
  listobj *cursor = _list->pHead;
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

void destroy_list(list *_list) {
  listobj *cursor;
  for (; _list->pHead != NULL;) {
    cursor = _list->pHead;
    _list->pHead = _list->pHead->pNext;
    destroy_listobj(cursor);
  }
  _list->pHead = NULL;
  _list->pTail = NULL;
  safe_free(_list);
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
  volatile uint first_execute = TRUE;
  isr_off();
  SaveContext();
  if (first_execute == TRUE) {
    listobj *node;
    first_execute = FALSE;
    node = node_fetch_by_task(ready_list, Running);
    node->nTCnt = nTicks;
    node_transfer_list(ready_list, timer_list, node);
    Running = list_get_head_task(ready_list);
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

#define LIST_FOR_EACH(list, item)                                                    \
  for (item = list->pHead; item != NULL; item = item->pNext)

void TimerInt(void) {
  listobj *item;
  tick_counter++;
  LIST_FOR_EACH(timer_list, item) {
    item->nTCnt--;
    if (item->nTCnt == 0) {
      node_transfer_list(timer_list, ready_list, item);
    }
  }
  LIST_FOR_EACH(waiting_list, item) {
    if (ticks() > item->pTask->DeadLine) { /* deadline expired */
      node_transfer_list(waiting_list, ready_list, item);
      // item->pMessage->pBlock = NULL; /* TODO: clear mailbox entry */
    }
  }
}

void set_ticks(uint no_of_ticks) { tick_counter = no_of_ticks; }

uint ticks(void) { return tick_counter; }

uint deadline() { return Running->DeadLine; }

void set_deadline(uint nNew) {
  volatile uint first_execute = TRUE;
  isr_off();
  SaveContext();
  if (first_execute == TRUE) {
    listobj *node;
    first_execute = FALSE;
    Running->DeadLine = nNew;
    /* reschedule readylist */
    node = node_fetch_by_task(ready_list, Running);
    node_remove(ready_list, node);
    list_insert_by_ddl(ready_list, node);
    LoadContext();
  }
}

/************************
 * Mailbox manipulation *
 ************************/
void mailbox_push_wait_msg(mailbox *mBox, msg *m);
void mailbox_push_no_wait_msg(mailbox *mBox, msg *m);
msg *mailbox_pop_wait_msg(mailbox *mBox);
msg *mailbox_pop_no_wait_msg(mailbox *mBox);

void mailbox_push_wait_msg(mailbox *mBox, msg *m) {
  if (mBox->nBlockedMsg == 0) { /* init empty mailbox */
    m->pPrevious = NULL;
    m->pNext = NULL;
    mBox->pHead = m;
    mBox->pTail = m;
    mBox->nBlockedMsg = 1;
  } else if (mBox->nBlockedMsg == mBox->nMaxMessages) { /* if mailbox is full */
    msg *old_msg = mailbox_pop_wait_msg(mBox);     /* remove the oldest msg */
    old_msg->pBlock->pMessage = NULL;
    safe_free(old_msg->pData);
    safe_free(old_msg);
    mailbox_push_wait_msg(mBox, m); /* recursive :) */
  } else {
    m->pPrevious = mBox->pTail;
    m->pNext = NULL;
    mBox->pTail->pNext = m;
    mBox->pTail = m;
    mBox->nBlockedMsg++;
  }
}

void mailbox_push_no_wait_msg(mailbox *mBox, msg *m) {
  if (mBox->nMessages == 0) { /* init empty mailbox */
    m->pPrevious = NULL;
    m->pNext = NULL;
    mBox->pHead = m;
    mBox->pTail = m;
    mBox->nMessages = 1;
  } else if (mBox->nMessages == mBox->nMaxMessages) { /* if mailbox is full */
    msg *old_msg = mailbox_pop_no_wait_msg(mBox);     /* remove the oldest msg */
    old_msg->pBlock->pMessage = NULL;
    safe_free(old_msg->pData);
    safe_free(old_msg);
    mailbox_push_no_wait_msg(mBox, m); /* recursive :) */
  } else {
    m->pPrevious = mBox->pTail;
    m->pNext = NULL;
    mBox->pTail->pNext = m;
    mBox->pTail = m;
    mBox->nMessages++;
  }
}

msg *mailbox_pop_wait_msg(mailbox *mBox) {
  if (mBox->nBlockedMsg == 0) { /* error: pop from empty mailbox */
    return NULL;
  } else if (mBox->nBlockedMsg == 1) { /* only 1 message */
    msg *m = mBox->pHead;
    mBox->pHead = NULL;
    mBox->pTail = NULL;
    mBox->nBlockedMsg = 0;
    return m;
  } else { /* return Head */
    msg *m = mBox->pHead;
    mBox->pHead = mBox->pHead->pNext;
    mBox->nBlockedMsg--;
    return m;
  }
}

msg *mailbox_pop_no_wait_msg(mailbox *mBox) {
  if (mBox->nMessages == 0) { /* error: pop from empty mailbox */
    return NULL;
  } else if (mBox->nMessages == 1) { /* only 1 message */
    msg *m = mBox->pHead;
    mBox->pHead = NULL;
    mBox->pTail = NULL;
    mBox->nMessages = 0;
    return m;
  } else {
    msg *m = mBox->pHead;
    mBox->pHead = mBox->pHead->pNext;
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

int no_messages(mailbox *mBox) { return mBox->nMessages + mBox->nBlockedMsg; }

exception remove_mailbox(mailbox *mBox) {
  if (no_messages(mBox) == 0) {
    safe_free(mBox);
    return OK;
  } else {
    return NOT_EMPTY;
  }
}

exception send_wait(mailbox *mBox, void *pData) {
  volatile uint first_execute = TRUE;
  isr_off();
  SaveContext();
  if (first_execute == TRUE) {
    first_execute = FALSE;
    if (mBox->nBlockedMsg > 0 &&
        mBox->pHead->Status == RECEIVER) { /* is waiting */
      msg *m = mailbox_pop_wait_msg(mBox);
      listobj *node;
      /* receiver has the duty to malloc data storage */
      memcpy(m->pData, pData, mBox->nDataSize);
      node = m->pBlock;
      m->pBlock->pMessage = NULL;
      safe_free(m);
      node_transfer_list(waiting_list, ready_list, node);
    } else { /* if no task is waiting */
      msg *m = safe_malloc(sizeof(msg));
      listobj *node = list_get_head(ready_list);
      NULL_CHECKER(m);
      node->pMessage = m;
      m->Status = SENDER;
      m->pData = pData;
      m->pBlock = node;
      mailbox_push_wait_msg(mBox, m);
      node_transfer_list(ready_list, waiting_list, node);
      Running = list_get_head_task(ready_list);
    }
    LoadContext();
  } else {
    if (deadline() < ticks()) { /* deadline reached */
      msg *m;
      isr_off();
      m = mailbox_pop_wait_msg(mBox);
      safe_free(m);
      isr_on();
      return DEADLINE_REACHED;
    }
  }
  return OK;
}

exception receive_wait(mailbox *mBox, void *pData) {
  volatile uint first_execution = TRUE;
  isr_off();
  SaveContext();
  if (first_execution == TRUE) {
    first_execution = FALSE;
    /* receive wait can access both wait msg and non-wait msg */
    if ((mBox->nMessages + mBox->nBlockedMsg) == 0) { /* mailbox is empty */
      msg *m = safe_malloc(sizeof(msg));
      listobj *node = list_get_head(ready_list);
      NULL_CHECKER(m);
      if (kernel_status == FAIL)
        return kernel_status;
      node->pMessage = m;
      m->Status = RECEIVER;
      m->pBlock = node;
      m->pData = pData;
      mailbox_push_wait_msg(mBox, m);
      node_transfer_list(ready_list, waiting_list, node);
      Running = list_get_head_task(ready_list);
    } else {                                                 /* msg is waiting */
      uint wait_type = mBox->nBlockedMsg > 0 ? TRUE : FALSE; /* is wait type? */
      if (wait_type) {
        msg *m = mailbox_pop_wait_msg(mBox);
        listobj *node = m->pBlock;
        /* receiver has the duty to malloc data storage */
        memcpy(pData, m->pData, mBox->nDataSize);
        m->pBlock->pMessage = NULL;
        safe_free(m);
        node_transfer_list(waiting_list, ready_list, node);
        Running = list_get_head_task(ready_list);
      } else { /* no wait */
        msg *m = mailbox_pop_no_wait_msg(mBox);
        /* receiver has the duty to malloc data storage */
        m->pBlock->pMessage = NULL;
        memcpy(pData, m->pData, mBox->nDataSize);
        safe_free(m);
      }
    }
    LoadContext();
  } else { /* not first execution */
    if (deadline() < ticks()) {
      msg *m;
      isr_off();
      m = mailbox_pop_wait_msg(mBox);
      m->pBlock->pMessage = NULL;
      safe_free(m);
      isr_on();
      return DEADLINE_REACHED;
    }
  }
  return OK;
}

exception send_no_wait(mailbox *mBox, void *pData) {
  volatile uint first_execution = TRUE;
  isr_off();
  SaveContext();
  if (first_execution == TRUE) {
    first_execution = FALSE;
    if (mBox->nMessages > 0 &&
        mBox->pHead->Status == RECEIVER) { /* receiving task is waiting */
      msg *m = mailbox_pop_no_wait_msg(mBox);
      memcpy(m->pData, pData, mBox->nDataSize);
      m->pBlock->pMessage = NULL;
      node_transfer_list(waiting_list, ready_list, m->pBlock);
      safe_free(m);
      Running = list_get_head_task(ready_list);
      LoadContext();
    } else { /* No task is waiting */
      msg *m = safe_malloc(sizeof(msg));
      listobj *node = list_get_head(ready_list);
      NULL_CHECKER(m);
      if (kernel_status == FAIL)
        return kernel_status;
      m->pData = safe_malloc(mBox->nDataSize); 
      NULL_CHECKER(m->pData);
      if (kernel_status == FAIL)
        return kernel_status;
      memcpy(m->pData, pData, mBox->nDataSize); /* copy data to the message */
      m->Status = SENDER;
      m->pBlock = node;
      node->pMessage = m;
      mailbox_push_no_wait_msg(mBox, m);
      /* No reschedule */
    }
  }
  return OK;
}

int receive_no_wait(mailbox *mBox, void *pData) {
  volatile uint first_execution = TRUE;
  isr_off();
  SaveContext();
  if (first_execution == TRUE) {
    first_execution = FALSE;
    if (mBox->nBlockedMsg > 0) { /* wait type */
      msg *m = mailbox_pop_wait_msg(mBox);
      memcpy(pData, m->pData, mBox->nDataSize);
      m->pBlock->pMessage = NULL;
      node_transfer_list(waiting_list, ready_list, m->pBlock);
      safe_free(m);
      Running = list_get_head_task(ready_list);
    } else if (mBox->nMessages > 0) { /* non-wait type */
      msg *m = mailbox_pop_no_wait_msg(mBox);
      memcpy(pData, m->pData, mBox->nDataSize);
      safe_free(m->pData); /* free allocated data from send_no_wait */
      m->pBlock->pMessage = NULL; 
      safe_free(m);
    } else { /* empty, not msg read */
      return FAIL;
    }
    LoadContext();
  }
  return OK;
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
  TCB *t_idle;
  kernel_mode = INIT;
  kernel_status = OK;
  set_ticks(0);
  ready_list = create_list();
  NULL_CHECKER(ready_list);
  waiting_list = create_list();
  NULL_CHECKER(waiting_list);
  timer_list = create_list();
  NULL_CHECKER(timer_list);
  t_idle = create_TCB();
  NULL_CHECKER(t_idle);
  t_idle->PC = idle;
  t_idle->SP = &(Running->StackSeg[STACK_SIZE - 1]);
  t_idle->DeadLine = UINT_MAX;
  list_insert_by_ddl(ready_list, create_listobj(t_idle));
  return kernel_status;   
}

void idle() {
#ifdef TEST_MODE
  // extern mailbox *mb1;
  // extern mailbox *mb2;
  TimerInt();
  Running = list_get_head_task(ready_list);
  if (tick_counter > 100000) {
    destroy_list(ready_list);
    destroy_list(waiting_list);
    destroy_list(timer_list);
    // remove_mailbox(mb1);
    // remove_mailbox(mb2);
    while (1);
  }
  LoadContext();
#else
  while(1);
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
  tcb->DeadLine = d;
  tcb->PC = body;
  tcb->SP = &(tcb->StackSeg[STACK_SIZE - 1]);
  if (kernel_mode == INIT) {
    list_insert_by_ddl(ready_list, create_listobj(tcb));
    return kernel_status;
  } else {     /* RUNNING */
    isr_off(); /* turn off interrupt */
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
  #ifndef TEST_MODE
  timer0_start();
  #endif
  kernel_mode = RUNNING;
  Running = list_get_head_task(ready_list);
  isr_on();
  LoadContext();
}

/*
This call will terminate the running task. All data
structures for the task will be removed. Thereafter,
another task will be scheduled for execution.
*/
void terminate() {
  node_destroy_by_task(ready_list, Running);
  Running = list_get_head_task(ready_list);
  LoadContext();
}
