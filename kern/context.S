#include  "kernel_hwdep.h"
.globl  SaveContext
.globl  LoadContext
.globl  Get_psr
.globl  Set_psr
.globl Timer0Int
.globl isr_off
.globl isr_on
.type    SaveContext,%function
.type    LoadContext,%function
.type    Get_psr,%function
.type    Set_psr,%function
.type    Timer0Int, %function
.type    isr_off, %function
.type    isr_on, %function

.p2align 2 // force 4 byte alignment

//        ORG		0x100  
//****************************************************************************
// Timer0Int - Shell for timer interrupt.  
//       1. Save context to Running->Context 
//       2. Sign interrupt
//       3. Branch to Timer interrupt in C
//       4. Load context to Running->Context 
//       5. Move the LinkReg minus offset to PC 
//       6. Copy SPSR to the CPSR 
//       7. Clear the interrupt flags
//****************************************************************************            
Timer0Int: 
    stmdb SP!,{r0,r1,LR}
    //-savecontext--
    ldr r0,=Running	            // Load address to context
    ldr r0,[r0]
    add r0,r0,#4                
    stmia r0,{r1-r12}	            // Save registers r1-r12
    nop
    nop
    nop  
          
    mrs r1,SPSR                     // Load SPSR into r1
    str r1, [r0,#56]                // Saves SPSR
    sub r0,r0,#4                    // Makes r0 point to context->r0
    ldr r1,[SP]                     // Loads r0 from stack
    str r1,[r0]		            // Saves r0. 
    ldmia SP!,{r1,r2,r3}
    sub r1,LR,#4                    // Find orginal LR 
    str r1,[r0,#56]
    mrs r1,CPSR
    orr r1,r1,#1                    // Change mode to Supervisor. Bit 0 = 1.  
    msr CPSR_c,r1              
    mov r1,SP
    str r1,[r0,#52]                 // Save stackpointer to TCB->SP
         
    add r0,PC,#4                    // Prepare for jump
    mov LR,r0
    bl TimerInt                      // Branch to Timer interrupt in C-file
        
    ldr r0,=Running
    ldr r0,[r0]		
    ldr r1, [r0,#52]                // Catch Running-> SP
    stmda SP!,{r1}                  // and put it on temporary stack.
    ldr r1,[r0,#8]                  // Fetch r2's value.TCB->context[2]
    stmda SP!,{r1}                  // and push to stack.    
    ldr r1,[r0,#4]                  // Fetch r1's value.TCB->context[1]
    stmda SP!,{r1}                  // and push to stack       
    ldr r1,[r0]                     // Fetch r0's value.TCB->context[0]
    stmda SP!,{r1}                  // and push stack.   
       
        
    add r0,r0,#4            
    ldmia r0!,{r1-r12}^             // Restore values for r1-r12        
    ldr LR, [r0,#4]                 // Restore value for LR (PC)
    cmp r14,#0
    beq trap
    ldr r0,[r0,#8]                  // Load value for SPSR
    msr SPSR_c,r0                                
      
//-Enable timer interrupt-         
    ldr r0,=0x100                   // rINTMSK = 0x100
    ldr r2,=0x7ffc008               // TODO =rINTMSK                         
    str r0, [r2]
        
//----- Enable timer and clear counter--,
    ldr r0,=0xc8                    // rTCON0 =  0xc8                       
    ldr r2,=0x7ff9003               // TODO =rTCON0
    strb r0,[r2]
        
//-----rINTPND clear pending
    ldr r0,=0x00             
    ldr r2,=0x7ffc004               // TODO =rINTPND
    strb r0,[r2]
        
    mrs r1,CPSR
    orr r1,r1,#1                    // Change mode to Supervisor. Bit 0 = 1.  
    msr CPSR_c,r1        
    mrs r0,SPSR
    bic r0,r0,#128                  // Clear IRQ disable bit. Bit 7.       
    bic r0,r0,#4                    // Change mode to Supervisor. Bit 3 = 0. 
    msr SPSR_c, r0      
    nop 
    nop
    nop     
    ldmib   SP!,{r0,r1,r2}     
    ldr SP,[r13,#4] 
    subs PC,r14,#0


//****************************************************************************
// void isr_on(void) - EnableInterrupt
//   Clear interrupt request and
//   Interrupt disable flag = 0  
//****************************************************************************
isr_on:
    stmdb SP!,{r12}  
    mrs r12,cpsr
    bic r12,r12,#192
    msr cpsr_c,r12
    ldmia   SP!,{r12} 
    bx  lr	
//****************************************************************************
// uint Get_psr(void), Get IRQ flag value
// void Set_psr(uint), Set IRQ flag value
//****************************************************************************
Get_psr:
    mrs r0,cpsr
    bx  lr
  
Set_psr:
    msr cpsr_c,r0
    bx  lr
  
//****************************************************************************
// void isr_off(void)
// DisableInterrupt flag = 1
// Clear the interrupt flags in PSR 
//****************************************************************************
isr_off:
    stmdb SP!,{r12}
    mrs r12,cpsr
    orr r12,r12,#192
    msr cpsr_c,r12
    mrs r12,cpsr
    ands r12,r12,#192
    beq isr_off
    ldmia   SP!,{r12}
    bx  lr
        

// SP is the stack register a shortcut for typing r13. 
// LR is the link register a shortcut for r14. 
// And PC is the program counter a shortcut for typing r15.
//****************************************************************************
//  void SaveContext(void)
//***************************************************************************
SaveContext:      
    /*
     * The ! means "Register write-back": 
     * the base register is used to calculate the address of the transfer, and is updated.
     * kind of like ! in Ruby
     * STM"DB": Decrement address Before each transfer
     */
    /*
    +------------------+  <- low address
    |    0x00000000    |
    +------------------+  <- 
    |    0x00000000    |
    +------------------+  <- SP = SP - 8
    |        r1        |
    +------------------+  <- SP - 4
    |        r0        |
    +------------------+  <- SP
    |      stack       |
    +------------------+  <- high address
    */
    stmdb SP!,{r0,r1}                   // SP -= 8;

    ldr r0,=Running	                    // Load address to context
    ldr r0,[r0]                         // Get the value in that address r0

    add r0,r0,#4                        // r0: offset 4 = context[1]
    // STMIA = Store Multiple Increment After, r0 as base address register
    stmia r0,{r1-r12}	                // Save registers r1-r12 to context[1-12]
    nop
    nop
    nop    
    
    /* 
     * Store return address, stands for the address of the next instruction in the context
     * LR = Link Register for return address
     */
    mov r1,LR
    str r1,[r0,#52]                  // Save LR to TCB->PC(r0: offset 4+52)
          
    /*
     * STR instructions store a word to memory.
     * LDR instructions load a word from memory.
     */
    // mrs = Move to ARM register from system coprocessor register.
    // CPSR = Current Program Status Register
    mrs r1,CPSR                      // Load CPSR into r1
    str r1, [r0,#56]                 // and save to TCB->SPSR(r0: offset 4+56)
    sub r0,r0,#4                     // Makes r0 point to context[0](r0: offset 0)
    ldr r1,[SP]                      // Get original r0 value, it's on stack.
    str r1,[r0]		                 // Saves r0 to context[0]
         
    add r1,SP,#8                     // Fetch Stackpointer
    str r1,[r0,#52]                  // and save to TCB->SP  	  
    ldmia SP!,{r0,r1}                // Restore r0, r1
    mov PC,LR                        // Return to C-program
    
//****************************************************************************
//  void LoadContext(void)
//***************************************************************************    
LoadContext:

    ldr r0,=Running                 // Load Running to context
    ldr r0,[r0]                     // Evaluate pointer *Running to get context

/*
+------------------+  <- low address
|    0x00000000    |
+------------------+  <- 
|         SP       |
+------------------+  <- SP - 8: SP
|  TCB->context[1] |
+------------------+  <- SP - 4: TCB->context[1]
|  TCB->context[0] |
+------------------+  <- SP
|      stack       |
+------------------+  <- high address
*/
    ldr r1, [r0,#52]                // Catch Running-> SP
    sub SP,SP,#8	                // Find a unused stack area
    stmda SP!,{r1}                  // and put SP on the temporary stack 
    ldr r1,[r0,#4]                  // Fetch r1's value.TCB->context[1]
    stmda SP!,{r1}                  // push r1's value to stack       
    ldr r1,[r0]                     // Fetch r0's value.TCB->context[0]
    stmda SP!,{r1}                  // push r0's value to stack      

    add r0,r0,#4                    // r0 point to TCB->context[1], r0: offset 4
    ldmia r0!,{r1-r12}^             // Restore values for r1-r12 from context[1-12]

    ldr r0,=Running
    ldr r0,[r0]                     // Evaluate pointer *Running to get context
    add r0,r0,#56                   // r0: offset 4+56 = SPSR
    ldr r14,[r0]                    // Load SPSR to r14

    cmp r14,#0                      // If SPSR is 0
    beq trap
    ldr r0,[r0,#8]                  // Load value for SPSR
    cmp r0,#0                       // If SPSR = 0, first loading.
    beq skipSPSR                    // then skip loading SPSR.
    // msr = Move to system coprocessor register from ARM register.
    msr CPSR_c,r0                   // else CPSR = TCB->SPSR
skipSPSR:          

    ldr r0,=Running
    ldr r0,[r0]    
    add r0,r0,#56                   // r0: offset 56 = PC
    ldr r14,[r0]                    // Load the function to r14(LR)

    ldmib SP!,{r0,r1}               // Restore r0 and r1
    ldr SP,[r13,#4]                 // 
    mov PC, r14                       // movs PC,r14, Branch to Running task
trap:
    b .
/*
r15 is the program counter.
r14 is the link register. 
    (The BL instruction, used in a subroutine call, stores the return address in this register).
r13 is the stack pointer. 
    (The Push/Pop instructions in "Thumb" operating mode use this register only).
r12 is the Intra-Procedure-call scratch register.
r4 to r11: used to hold local variables.
r0 to r3: used to hold argument values passed to a subroutine, 
    and also hold results returned from a subroutine.
*/
