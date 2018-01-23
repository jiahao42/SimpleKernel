/* 2002-11-19 Wolfgang Svensson*/
#include <stdlib.h>
#include "kernel_hwdep.h"

/*-------------------------------------------------------------------------*/
/* uint set_isr( uint newCSR )  - Change interrupt ON/OFF                  */
/*	ints ON/OFF on entry						   */
/*	ints ON/OFF on exit						   */
/* HW dependent	  							   */
/* Argument: New CSR							   */
/* Returns: Old CSR							   */
/*-------------------------------------------------------------------------*/

unsigned int set_isr( unsigned int newCSR ) {
	unsigned int oldCSR;
	volatile unsigned int PSR;
	PSR = Get_psr();
	oldCSR = PSR & CSR_BIT; //Get bit 7, IRQ
	PSR = PSR & (~CSR_BIT);
	PSR = PSR | newCSR;
	Set_psr(PSR);
	return oldCSR;
}


void timer0_start(void)
{
/*Turn off all interrupt, just in case Bit 6, GIE=0, pp15-6 */
  rSYSCON&= ~(0x40); 
  rINTMSK =0x00;
  rINTPND=0x00; 

/* Configure interrupt: Reset counter and enable counter interval mode operation 8-6 */
  rTCON0 =  0x40;
  rTCON0 =  0x80; 

/*timer input clock freq. = MCLK/ (TPRE0 +1) 8-5 
 Internal clock 50 MHz -> Timer 0 period 25 ns - ~20 ms.
See Prescale timer 8-9*/ 
  rTPRE0 = 0x3f;
  rTDAT0 = 0x1e01;

/* "IRQ" - not "FIRQ" , Reset pp11-3*/
  rINTMOD = 0x00000000;	

/* Activate INT_TMC0, match capture, Bit 8, pp11-7  and
Activate interrupt globally. Bit 6, GIE=1, pp15-6*/
  rINTMSK = 0x100; 
  rSYSCON |= 0x40;
}
