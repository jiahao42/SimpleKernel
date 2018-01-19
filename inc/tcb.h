#ifndef LAMB_KERNEL_TCB_CHAIN_H_
#define LAMB_KERNEL_TCB_CHAIN_H_

#include "type.h"

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


/* TCB node*/
extern void init_tcb_node(TCB_node *node, TCB *data);
extern TCB_node *create_tcb_node(TCB *data);
extern void destroy_tcb_node(TCB_node *node);

/* TCB chain */
extern TCB_chain *create_tcb_chain();
extern void tcb_destroy_chain(TCB_chain *chain);
extern void tcb_append(TCB_chain *chain, TCB_node *node);
extern void tcb_insert_after(TCB_chain *chain, TCB_node *pos, TCB_node *n_node);
extern void tcb_insert_before(TCB_chain *chain, TCB_node *pos, TCB_node *n_node);
extern TCB_node *tcb_get_node(TCB_chain *chain, int index);
extern TCB *tcb_get_data(TCB_chain *chain, int index);


#endif