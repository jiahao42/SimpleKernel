#include "kern_util.h"
#include "inc/tcb.h"
#include "inc/error.h"

void init_tcb_node(TCB_node *node, TCB *data) {
  node->data = data;
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

TCB_list *create_tcb_list() {
  TCB_list *chain = (TCB_list *)safe_malloc(sizeof(TCB_list));
  chain->head = NULL;
  chain->tail = NULL;
  return chain;
}

void tcb_append(TCB_list *chain, TCB_node *node) {
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

void tcb_prepend(TCB_list *chain, TCB_node *node) {
  if (chain->head == NULL) {
    chain->head = node;
    chain->tail = node;
  } else {
    node->prev = NULL;
    node->next = chain->head;
    chain->head->prev = node;
    chain->head = node;
  }
}

void tcb_insert_after(TCB_list *chain, TCB_node *pos, TCB_node *n_node) {
  if (pos->next == NULL) {
    tcb_append(chain, n_node);
    return;
  }
  n_node->next = pos->next;
  n_node->prev = pos;
  pos->next = n_node;
  n_node->next->prev = n_node;
}

void tcb_insert_before(TCB_list *chain, TCB_node *pos, TCB_node *n_node) {
  if (pos->prev == NULL) {
    tcb_prepend(chain, n_node);
    return;
  }
  n_node->prev = pos->prev;
  n_node->next = pos;
  pos->prev->next = n_node;
  pos->prev = n_node;
}

void tcb_destroy_chain(TCB_list *chain) {
  TCB_node *cursor = chain->head;
  for (; chain->head != NULL;) {
    cursor = chain->head;
    chain->head = chain->head->next;
    safe_free(cursor);
  }
  safe_free(chain);
}

TCB_node *tcb_get_node(TCB_list *chain, int index) {
  TCB_node *cursor = chain->head;
  int i = 0;
  for (; cursor != NULL; cursor = cursor->next) {
    if (i++ == index) {
      return cursor;
    }
  }
  return NULL;
}

TCB *tcb_get_data(TCB_list *chain, int index) {
  TCB_node *cursor = chain->head;
  int i = 0;
  for (; cursor != NULL; cursor = cursor->next) {
    if (i++ == index) {
      return cursor->data;
    }
  }
  return NULL;
}