The first building block for combining embedded hardware devices on the TM4C123 to a custom operating system. 
We modify files osasm.s and os.c. 

**os.c:**
- Designed a bare bones thread control block structure
- Have functions to set up each TCBs stack and next reference in a circular linked list
- Create very basic semaphore logic for communication
- Set the scheduler to handle periodic threads at intervals on top of being a basic round-robin
- Have a very basic producer-consumer system for sending newly created data

**osasm.s:**
- Implemented StartOS to schedule the first thread in the TCB list,
  relinquishing registers, the Link register, PSR, and Program counter to the CPU
- Implemented the SysTick_Handler ISR to context switch, saving state of the current running thread,
  calling the scheduler to decide the next thread to run, and relinquishing the stack contents to the CPU for computation

This accomplishes a very basic albeit inefficient entry for RTOS that would run an accelerometer sensor, light sensor, microphone sensor, 
buttons and LEDs, and an LCD chart for data activity.

Testing consisted of using Keil uVisions debugger and watch window as well as my own logic analyzer to track pin activity.  
