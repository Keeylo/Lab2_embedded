// os.c
// Runs on LM4F120/TM4C123/MSP432
// Lab 2 starter file.
// Daniel Valvano
// February 20, 2016

#include <stdint.h>
#include "os.h"
#include "../inc/CortexM.h"
#include "../inc/BSP.h"

// function definitions in osasm.s
void StartOS(void);

// Seems NUMTHREADS and STACKSIZE are defined in os.h?
tcbType tcbs[NUMTHREADS];
tcbType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];
int32_t count;
void (*PeriodicTask0)(void);
uint32_t p1;
void (*PeriodicTask1)(void);
uint32_t p2;

int32_t mailbox; // value for the mailbox
int32_t send; // shared value for semaphore


// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: systick, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();// set processor clock to fastest speed
  // initialize any global variables as needed
	count = 0;
  //***YOU IMPLEMENT THIS FUNCTION*****
}

// Based on NUMTHREADS and STACKSIZE, be sure to set the 16 registers
// Also be sure to set the tcb stack pointer to the correct address
// PC can't be set here, no address of the main programs
void SetInitialStack(int i){
  tcbs[i].sp = &Stacks[i][STACKSIZE-16]; // thread stack pointer
  Stacks[i][STACKSIZE-1] = 0x01000000;   // thumb bit
  Stacks[i][STACKSIZE-3] = 0x14141414;   // R14
  Stacks[i][STACKSIZE-4] = 0x12121212;   // R12
  Stacks[i][STACKSIZE-5] = 0x03030303;   // R3
  Stacks[i][STACKSIZE-6] = 0x02020202;   // R2
  Stacks[i][STACKSIZE-7] = 0x01010101;   // R1
  Stacks[i][STACKSIZE-8] = 0x00000000;   // R0
  Stacks[i][STACKSIZE-9] = 0x11111111;   // R11
  Stacks[i][STACKSIZE-10] = 0x10101010;  // R10
  Stacks[i][STACKSIZE-11] = 0x09090909;  // R9
  Stacks[i][STACKSIZE-12] = 0x08080808;  // R8
  Stacks[i][STACKSIZE-13] = 0x07070707;  // R7
  Stacks[i][STACKSIZE-14] = 0x06060606;  // R6
  Stacks[i][STACKSIZE-15] = 0x05050505;  // R5
  Stacks[i][STACKSIZE-16] = 0x04040404;  // R4
}


// Based on NUMTHREADS and STACKSIZE, create each thread in a linked list (tcbs) and their 16 registers
// Also be sure to set the RunPt as well
// PC can be set here, address of each main program provided

//******** OS_AddThreads ***************
// Add four main threads to the scheduler
// Inputs: function pointers to four void/void main threads
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(void(*thread0)(void), void(*thread1)(void),
	void(*thread2)(void), void(*thread3)(void)){
// initialize TCB circular list
// initialize RunPt
// initialize four stacks, including initial PC
  //***YOU IMPLEMENT THIS FUNCTION*****
		int32_t status = StartCritical();
		tcbs[0].next = &tcbs[1]; // set the linked list of threads
		tcbs[1].next = &tcbs[2];
		tcbs[2].next = &tcbs[3];
		tcbs[3].next = &tcbs[0];
		
		SetInitialStack(0);
		Stacks[0][STACKSIZE - 2] = (int32_t)(thread0);
		
		SetInitialStack(1);
		Stacks[1][STACKSIZE - 2] = (int32_t)(thread1);
		
		SetInitialStack(2);
		Stacks[2][STACKSIZE - 2] = (int32_t)(thread2);
		
		SetInitialStack(3);
		Stacks[3][STACKSIZE - 2] = (int32_t) (thread3);
		RunPt = &tcbs[0];
		EndCritical(status);
		return 1;               // successful
}

 

//******** OS_AddThreads3 ***************
// add three foregound threads to the scheduler
// This is needed during debugging and not part of final solution
// Inputs: three pointers to a void/void foreground tasks
// Outputs: 1 if successful, 0 if this thread can not be added
int OS_AddThreads3(void(*task3)(void),
                 void(*task4)(void),
                 void(*task5)(void)){ int32_t status;
  status = StartCritical();
  tcbs[0].next = &tcbs[1]; // 0 points to 1
  tcbs[1].next = &tcbs[2]; // 1 points to 2
  tcbs[2].next = &tcbs[0]; // 2 points to 0
  SetInitialStack(0); Stacks[0][STACKSIZE-2] = (int32_t)(task3); // PC
  SetInitialStack(1); Stacks[1][STACKSIZE-2] = (int32_t)(task4); // PC
  SetInitialStack(2); Stacks[2][STACKSIZE-2] = (int32_t)(task5); // PC
  RunPt = &tcbs[0];       // thread 0 will run first
  EndCritical(status);
  return 1;               // successful
}
                 
//******** OS_AddPeriodicEventThreads ***************
// Add two background periodic event threads
// Typically this function receives the highest priority
// Inputs: pointers to a void/void event thread function2
//         periods given in units of OS_Launch (Lab 2 this will be msec)
// Outputs: 1 if successful, 0 if this thread cannot be added
// It is assumed that the event threads will run to completion and return
// It is assumed the time to run these event threads is short compared to 1 msec
// These threads cannot spin, block, loop, sleep, or kill
// These threads can call OS_Signal
int OS_AddPeriodicEventThreads(void(*thread1)(void), uint32_t period1,
  void(*thread2)(void), uint32_t period2){
  //***YOU IMPLEMENT THIS FUNCTION*****
		int32_t status = StartCritical();
		PeriodicTask0 = thread1;
		p1 = period1;
		PeriodicTask1 = thread2;
		p2 = period2;
		EndCritical(status);
		return 1;
}

//******** OS_Launch ***************
// Start the scheduler, enable interrupts
// Inputs: number of clock cycles for each time slice
// Outputs: none (does not return)
// Errors: theTimeSlice must be less than 16,777,216
void OS_Launch(uint32_t theTimeSlice){
  STCTRL = 0;                  // disable SysTick during setup
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
  STRELOAD = theTimeSlice - 1; // reload value
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
  StartOS();                   // start on the first task
}
// runs every ms
void Scheduler(void){ // every time slice
  // run any periodic event threads if needed
  // implement round robin scheduler, update RunPt
  //***YOU IMPLEMENT THIS FUNCTION*****
	count = (count+1)%p2; // 1-99 then back to 0 for the 100th iteration = 100 iterations
	
	if(count%p1 == 0) {
		(*PeriodicTask0)();
	}
	
	if(count%p2 == 0) {
		(*PeriodicTask1)();
	}
	
	RunPt = (*RunPt).next; // RunPt now points to whatever the next pointer points to (since RunPt is only a pointer it can't pointer to another pointer)
}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t *semaPt, int32_t value){
  //***YOU IMPLEMENT THIS FUNCTION*****
	DisableInterrupts(); // So it doesn't interrupt this operation
	*semaPt = value; 
	EnableInterrupts(); // Because we are now done with this operation
}

// ******** OS_Wait ************
// Decrement semaphore
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t *semaPt){
	DisableInterrupts(); // So it doesn't interrupt this operation
	while((*semaPt) == 0) { // if no resource
		EnableInterrupts(); // Enable interrupts for the moment, so other tasks run
		DisableInterrupts(); // Once this is scheduled again, disable interrupts for a time to check the resource again
	}
	(*semaPt)--; // Decrement semaphore
	// (*semaPt) = (*semaPt) - 1;
	EnableInterrupts(); // Once we perform the operation, allow interruptions
}

// ******** OS_Signal ************
// Increment semaphore
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Signal(int32_t *semaPt){
//***YOU IMPLEMENT THIS FUNCTION*****
	DisableInterrupts(); // So it doesn't interrupt this operation
	(*semaPt)++;
	// (*semaPt) = (*semaPt) + 1;
	EnableInterrupts(); // Because we are now done with this operation
}




// ******** OS_MailBox_Init ************
// Initialize communication channel
// Producer is an event thread, consumer is a main thread
// Inputs:  none
// Outputs: none
void OS_MailBox_Init(void){
  // include data field and semaphore
  //***YOU IMPLEMENT THIS FUNCTION*****
	OS_InitSemaphore(&send, 0); // only 1 semaphore since we are dealing with event thread with main threads
	mailbox = 0; // no value yet
}

// ******** OS_MailBox_Send ************
// Enter data into the MailBox, do not spin/block if full
// Use semaphore to synchronize with OS_MailBox_Recv
// Inputs:  data to be sent
// Outputs: none
// Errors: data lost if MailBox already has data
int32_t lost = 0;
void OS_MailBox_Send(uint32_t data){
  //***YOU IMPLEMENT THIS FUNCTION*****
	if(send) {
		lost++;
	} else {
		mailbox = data; // send old data, don't overwrite with new data even if available
		OS_Signal(&send);
	}
}

// ******** OS_MailBox_Recv ************
// retreive mail from the MailBox
// Use semaphore to synchronize with OS_MailBox_Send
// Lab 2 spin on semaphore if mailbox empty
// Lab 3 block on semaphore if mailbox empty
// Inputs:  none
// Outputs: data retreived
// Errors:  none
uint32_t OS_MailBox_Recv(void){
  //***YOU IMPLEMENT THIS FUNCTION*****
	OS_Wait(&send);
  return mailbox;
}


