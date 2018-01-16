.globl   SaveContext
.globl   LoadContext
.p2align 2
.type    SaveContext,%function
.type    LoadContext,%function

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
