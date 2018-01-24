#ifndef KERNEL_HWDEP_H
#define KERNEL_HWDEP_H

#include "inc/type.h"

#define CSR_BIT 0x80
#define ISR_OFF 0x80
#define ISR_ON 0x0

/*MED SFR_STARTADDRESS(special function register)       0x7ff0000*/
#define rTEST (*(volatile unsigned*)(0x7ff0000))

/*------------- Timer 0 -----------------------*/
#define rTDAT0 (*(volatile unsigned short*)(0x7ff9000))
#define rTPRE0 (*(volatile unsigned char *)(0x7ff9002))/* Prescale timer 8-9, ~400 ms*/
#define rTCON0 (*(volatile unsigned char *)(0x7ff9003))

/*------------ Interrupt Control-------------- */
#define rSYSCON (*(volatile unsigned char *)(0x7ffd003))
#define rINTMOD (*(volatile unsigned*)(0x7ffc000))
#define rINTPND (*(volatile unsigned*)(0x7ffc004))
#define rINTMSK (*(volatile unsigned*)(0x7ffc008))

extern uint tick_counter;
//void Init_IRQ_TINT0(void);
unsigned int set_isr( unsigned int newCSR );
void TimerInt();
extern unsigned int Get_psr(void);
extern void Set_psr(unsigned int PSR);

#endif
