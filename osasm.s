;/*****************************************************************************/
; OSasm.s: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123/MSP432
; Lab 2 starter file
; February 10, 2016
;


        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
        EXPORT  StartOS
        EXPORT  SysTick_Handler
        IMPORT  Scheduler


SysTick_Handler                ; 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID   I                  ; 2) Prevent interrupt during switch
	PUSH {R4-R11}			   ; 3) Save R4-R11 I believe
	LDR R0, =RunPt			   ; 4) Load run pointer into a register for further computation
	LDR R1, [R0]			   ; 5) Load the tcb/stack pointer by dereferencing the run pointer
	STR SP, [R1]			   ; 6) Store the current stack pointer address into the old tcbs stack pointer, this saves stack location
	;LDR R1, [R1, #4]
	;STR R1, [R0]
	PUSH {R0, LR}			   ; 7) Push an 8-byte aligned entry onto the stack while also saving the return address, getting ready to branch
	BL Scheduler			   ; 8) Branching to function call in C, this is the scheduler that will perform the thread switch
	POP {R0, LR}			   ; 8.5) Pop the return address and the other register used for alignment, so we restore state before the function call
	LDR R1, [R0]			   ; 8.75) Load the new tcb/stack pointer into R1
	LDR SP, [R1]			   ; 8.8) Load in the new stack pointer of the new tcb thread
	POP {R4-R11}			   ; 8.9) Pop these registers from the new tcb to relinquish them to the CPU for computation
    CPSIE   I                  ; 9) tasks run with interrupts enabled
    BX      LR                 ; 10) restore R0-R3,R12,LR,PC,PSR

StartOS
	LDR R0, =RunPt			   ; 1) Load the run pointer
	LDR R2, [R0]			   ; 2) Load the stack pointer/tcb
	LDR SP, [R2]			   ; 3) Load the stack pointer/tcb address into the CPU stack pointer
	POP {R4-R11}			   ; 4) Relinquish these registers to the CPU, still don't know what's really special about them
	POP {R0-R3}				   ; 5) Again I don't know what is so special about these besides just being place holders
	POP {R12}				   ; 6) For the third and final time, why do I care about this?
	ADD SP,SP,#4				   ; 7) Skip the LR return address for some reason
	POP {LR}				   ; 8) Pop LR so CPU knows the return address
	ADD SP,SP,#4				   ; 9) Disregard PSR? Tf we do this for?
    CPSIE   I                  ; Enable interrupts at processor level
    BX      LR                 ; start first thread

    ALIGN
    END
