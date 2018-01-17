#ifndef LAMB_KERNEL_TCB_CHAIN_H_
#define LAMB_KERNEL_TCB_CHAIN_H_

#include "type.h"
#include "kern_util.h"

#ifdef texas_dsp

#define CONTEXT_SIZE 34 - 2

#else

#define CONTEXT_SIZE 13
#define STACK_SIZE 100
#endif

// Task Control Block, TCB
#ifdef texas_dsp
typedef struct {
  void (*PC)();
  uint *SP;
  uint Context[CONTEXT_SIZE];
  uint StackSeg[STACK_SIZE];
  uint DeadLine;
} TCB;
#else
typedef struct {
  uint Context[CONTEXT_SIZE]; // 4 * 13 = 52, 0 - 51
  uint *SP;                   // 4, 52 - 55
  void (*PC)();               // 4, 56 - 59
  uint SPSR;                  // 4, 60 - 63
  uint StackSeg[STACK_SIZE];  // 4 * 100 = 400
  uint DeadLine;              // 4
} TCB;
#endif

typedef struct TCB_node {
  TCB data;
  struct TCB_node *prev;
  struct TCB_node *next;
} TCB_node;

typedef struct {
  TCB_node *head;
  TCB_node *tail;
} TCB_chain;


void init_tcb_node(TCB_node *node, TCB *data) {
  node->data = *data;
  node->next = NULL;
  node->prev = NULL;
}

TCB_node *create_tcb_node(TCB *data) {
  TCB_node *node = (TCB_node *)safe_malloc(sizeof(TCB_node));
  init_tcb_node(node, data);
  return node;
}

void destroy_tcb_node(TCB_node *node) {
  safe_free(node);
  node->next = NULL;
  node->prev = NULL;
}

TCB_chain *create_tcb_chain() {
  TCB_chain *chain = (TCB_chain *)safe_malloc(sizeof(TCB_chain));
  chain->head = NULL;
  chain->tail = NULL;
  return chain;
}

void tcb_append(TCB_chain *chain, TCB_node *node) {
  if (chain->tail == NULL) {
    chain->head = node;
    chain->tail = node;
  } else {
    chain->tail->next = node;
    node->prev = chain->tail;
    node->next = NULL;
    chain->tail = node;
  }
}

void tcb_insert_after(TCB_node *pos, TCB_node *n_node) {
  n_node->next = pos->next;
  n_node->prev = pos;
  pos->next = n_node;
  n_node->next->prev = n_node;
}

void tcb_insert_before(TCB_node *pos, TCB_node *n_node) {
  n_node->prev = pos->prev;
  n_node->next = pos;
  pos->prev->next = n_node;
  pos->prev = n_node;
}

void tcb_destroy_chain(TCB_chain *chain) {
  TCB_node *cursor = chain->head;
  for (; chain->head != NULL;) {
    cursor = chain->head;
    chain->head = chain->head->next;
    safe_free(cursor);
  }
  safe_free(chain);
}

TCB_node *tcb_get_node(TCB_chain *chain, int index) {
  TCB_node *cursor = chain->head;
  int i = 0;
  for (; cursor != NULL; cursor = cursor->next) {
    if (i++ == index) {
      return cursor;
    }
  }
  return NULL;
}

TCB *tcb_get_data(TCB_chain *chain, int index) {
  TCB_node *cursor = chain->head;
  int i = 0;
  for (; cursor != NULL; cursor = cursor->next) {
    if (i++ == index) {
      return &cursor->data;
    }
  }
  return NULL;
}

#endif