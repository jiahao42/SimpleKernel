
#ifndef KERNEL_H
#define KERNEL_H


// Debug option
//#define       _DEBUG

/*********************************************************/
/** Global variabels and definitions                     */
/*********************************************************/

#include <stdlib.h>
#ifdef texas_dsp

#define CONTEXT_SIZE    34-2 

#else

#define CONTEXT_SIZE    13 
#define STACK_SIZE      100
#endif

#define TRUE    1
#define FALSE   !TRUE

#define RUNNING 1
#define INIT    !RUNNING

#define FAIL    0
#define SUCCESS 1
#define OK              1

#define DEADLINE_REACHED        0
#define NOT_EMPTY               0

#define SENDER          +1
#define RECEIVER        -1


typedef int             exception;
typedef int             bool;
typedef unsigned int    uint;
typedef int 			action;

struct  l_obj;         // Forward declaration

// Task Control Block, TCB
#ifdef texas_dsp
typedef struct
{
	void	(*PC)();
	uint	*SP;
	uint	Context[CONTEXT_SIZE];
	uint	StackSeg[STACK_SIZE];
	uint	DeadLine;
} TCB;
#else
typedef struct
{
        uint    Context[CONTEXT_SIZE];        
        uint    *SP;
        void    (*PC)();
        uint    SPSR;     
        uint    StackSeg[STACK_SIZE];
        uint    DeadLine;
} TCB;
#endif

// Message items
typedef struct msgobj {
        char            *pData;
        exception       Status;
        struct l_obj    *pBlock;
        struct msgobj   *pPrevious;
        struct msgobj   *pNext;
} msg;

// Mailbox structure
typedef struct {
        msg             *pHead;
        msg             *pTail;
        int             nDataSize;
        int             nMaxMessages;
        int             nMessages;
        int             nBlockedMsg;
} mailbox;


// Generic list item
typedef struct l_obj {
         TCB            *pTask;
         uint           nTCnt;
         msg            *pMessage;
         struct l_obj   *pPrevious;
         struct l_obj   *pNext;
} listobj;


// Generic list
typedef struct {
         listobj        *pHead;
         listobj        *pTail;
} list;


// Function prototypes


// Task administration
int             init_kernel(void);
exception	create_task( void (* body)(), uint d );
void            terminate( void );
void            run( void );

// Communication
mailbox*	create_mailbox( uint nMessages, uint nDataSize );
int             no_messages( mailbox* mBox );

exception       send_wait( mailbox* mBox, void* pData );
exception       receive_wait( mailbox* mBox, void* pData );

exception	send_no_wait( mailbox* mBox, void* pData );
int             receive_no_wait( mailbox* mBox, void* pData );


// Timing
exception	wait( uint nTicks );
void            set_ticks( uint no_of_ticks );
uint            ticks( void );
uint		deadline( void );
void            set_deadline( uint nNew );

//Interrupt
extern void     isr_off(void);
extern void     isr_on(void);
extern void     SaveContext( void );	// Stores DSP registers in TCB pointed to by Running
extern void     LoadContext( void );	// Restores DSP registers from TCB pointed to by Running

#endif
