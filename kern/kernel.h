#ifndef LAMB_KERNEL_H_
#define LAMB_KERNEL_H_

#include "inc/type.h"
#include "inc/tcb.h"
#include "inc/test.h"
#include "inc/error.h"
#include "kern_util.h"
#include "kernel_hwdep.h"

// Debug option
//#define       _DEBUG

/*********************************************************/
/** Global variabels and definitions                     */
/*********************************************************/

#include <stdlib.h>

#define TRUE 1
#define FALSE !TRUE

#define RUNNING 1
#define INIT !RUNNING

#define FAIL 0
#define SUCCESS 1
#define OK 1

#define DEADLINE_REACHED 0
#define NOT_EMPTY 0

#define SENDER +1
#define RECEIVER -1

typedef enum {
  success = 0,
} exception;

typedef int action;

struct l_obj; // Forward declaration

// Message items
typedef struct msgobj {
  char *pData;
  exception Status;
  struct l_obj *pBlock;
  struct msgobj *pPrevious;
  struct msgobj *pNext;
} msg;

// Mailbox structure
typedef struct {
  msg *pHead;
  msg *pTail;
  int nDataSize;
  int nMaxMessages;
  int nMessages;
  int nBlockedMsg;
} mailbox;

// Generic list item
typedef struct l_obj {
  TCB *pTask;
  uint nTCnt;
  msg *pMessage;
  struct l_obj *pPrevious;
  struct l_obj *pNext;
} listobj;

// Generic list
typedef struct {
  listobj *pHead;
  listobj *pTail;
} list;

// Function prototypes

// Task administration
extern uint tick_counter;
extern TCB *Running;
extern TCB_list* ready_list;
extern uint kernel_mode;

int init_kernel(void);
exception create_task(void (*body)(), uint d);
void terminate(void);
void run(void);

// Communication
mailbox *create_mailbox(uint nMessages, uint nDataSize);
int no_messages(mailbox *mBox);

exception send_wait(mailbox *mBox, void *pData);
exception receive_wait(mailbox *mBox, void *pData);

exception send_no_wait(mailbox *mBox, void *pData);
int receive_no_wait(mailbox *mBox, void *pData);

// Timing
// exception wait(uint nTicks);
void set_ticks(uint no_of_ticks);
uint ticks(void);
uint deadline(void);
void set_deadline(uint nNew);

// Interrupt
extern void isr_off(void);
extern void isr_on(void);
extern void SaveContext(void); // Stores DSP registers in TCB pointed to by Running
extern void LoadContext(void); // Restores DSP registers from TCB pointed to by Running

#endif
