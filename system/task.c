/*##################################################################################### 
Copyright 2011 Nishchay Mhatre 

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
##################################################################################### */ 

/*#####################################################################################
COEP Satellite Project OC

Experimental Single Stack RTOS
Task Creation and Management and Scheduler

Author: Nishchay Mhatre 

Notes:
1. The Task Table of these structures must be created by the programmer in the
main function before starting the loop. So the number of entries is up to the
programmer and ends up as static in the application. Perhaps in a future
system of greater complexity , we can make this more dynamic.

Task table must be declared as:

	TCB SST_TTBL[N_TASKS]	, where N_TASKS is #define-d in the config file

In the task table the numbers TCB's are created in order of priority. For
simplicity. 

2.
Creation of task: Done in main by programmer before starting the main loop. 
f 		: function pointer to the task function
priority 	: self explantory.
parameter	: structure for packing parameters to the function

3. 
Deletion of task: None in current code. Future scope.

#####################################################################################*/ 

#include"task.h"
#include"SST_config.h" 
#include"portmacro.h" 

#define Q_FULL 2			//Placeholders
#define Q_EMPTY 2

// Global task table
TCB SST_TTBL[NTASKS+1];			// Rev 1, Change 1. NTASKS+1 is because we start task priorities from 1

// Priority of tasks

uint8_t SST_current_priority;               //This is a global for prioritylevel of
                                             //the currently running entity

// Data Structure for Tracking Ready Tasks;
uint8_t SST_ready_set;

/*##################################################################################### 

Operations on the TCB:(Note 3)

##################################################################################### */

void SST_task_create(task f, uint32_t priority, void*parameter)	// Note 2 
{
 SST_TTBL[priority].f = f;
 SST_TTBL[priority].priority = priority;	// Is this necessary? 
 SST_TTBL[priority].param = parameter;
 SST_TTBL[priority].head = 0;		
 SST_TTBL[priority].tail = 0;
 SST_ready_set |= (0<<priority);		// This may change subject to
 						//changes in ready set structure
 return;
}

/*##################################################################################### 

Event Management 

##################################################################################### */

#define eventq_add(x, tcb)		/* eventq_add and eventq_rem  */\
{\
 tcb.tail = ((tcb.tail+1)&(0xF));	/*Modulo 16 is the same as &0xF*/ \
 if ( tcb.tail==tcb.head )\
 	asm volatile ("SWI 0x3 \n\t");	\
 else\
 	{\
	 tcb.eventq[tcb.tail] = x;\
	}\
}

#define eventq_rem(x,tcb)\
{\
 if ( tcb.tail==tcb.head )\
 	asm volatile ("SWI 0x3 \n\t");	\
 else\
 	{\
 	 tcb.head = ((tcb.head+1)&(0xF)); /*advance head*/  \
	 x = tcb.eventq[tcb.head];\
	}\
}

/*##################################################################################### 

				The Scheduler

##################################################################################### */ 
uint8_t log2table[] =
{
 0,0,1,1,		// 0-3
 
 2,2,2,2,		// 4-7
 
 3,3,3,3,3,3,3,3,	// 8-15
 
 4,4,4,4,4,4,4,4,	//16-31
 4,4,4,4,4,4,4,4,		
 
 5,5,5,5,5,5,5,5,	//32-63
 5,5,5,5,5,5,5,5,		
 5,5,5,5,5,5,5,5,			
 5,5,5,5,5,5,5,5,			

 6,6,6,6,6,6,6,6,	//64-127
 6,6,6,6,6,6,6,6,		
 6,6,6,6,6,6,6,6,		
 6,6,6,6,6,6,6,6,		
 6,6,6,6,6,6,6,6,		
 6,6,6,6,6,6,6,6,		
 6,6,6,6,6,6,6,6,		
 6,6,6,6,6,6,6,6,	

 			//128-255
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7,		
 7,7,7,7,7,7,7,7
};

void SST_Scheduler()
{
 uint8_t saved_priority, new_priority;
 uint8_t e;						// Event number

 saved_priority = SST_current_priority; 

 while ((new_priority = log2table[SST_ready_set]) > SST_current_priority)
 	{
	 eventq_rem(e, SST_TTBL[new_priority]);		// Remove the event from the task's q
	 
	 if(SST_TTBL[new_priority].head == SST_TTBL[new_priority].tail)	// i.e. q is now empty
		 SST_ready_set &= (0<<new_priority);	// remove the flag from the ready set
	 
	 SST_current_priority = new_priority ;		// New is the new
	 						// current, since a new and 
							// interruptible task may now commence
	 
	 portENABLE_INTERRUPTS();			// Task should be interruptible

	 SST_TTBL[new_priority].f(SST_TTBL[new_priority].param);	// Call the test

	 portDISABLE_INTERRUPTS();			
	}

 SST_current_priority = saved_priority;			// Restore priority

 return;
}

/*#####################################################################################

Post event

##################################################################################### */ 
void SST_post_event(uint8_t e, uint32_t p)	// e= event number, p = priority 
{
 static uint8_t was_empty_flag;
 portDISABLE_INTERRUPTS();			// Critical section starts
 
 if(SST_TTBL[p].head == SST_TTBL[p].tail)	// q is empty
 	was_empty_flag = 1;

 eventq_add(e, SST_TTBL[p]);			// Post event to task
 
 if(was_empty_flag == 1)			// First event in the queue
 	{
	 SST_ready_set |= (1<<p);		// Set the bit in the ready set
	 					// variable, that corresponds to the
						// number of the task just made ready by
						// the posting of an event;
	 
	 was_empty_flag = 0;			// Next event to be posted goes
	 					// into an occupied queue. So
						// there is no need to
						// manipulate ready set again,
						// since task is already ready
						// to go.
	 }
	 
 SST_Scheduler();				// Call scheduler at every event that is posted. 
 						// True to the RTOS full preemptive principle
	
 portENABLE_INTERRUPTS();			// Critical section ends
 return;					
}

/*#################################################################################
Changelog:

Revision: 18th September 2010 (First write)
Rev1: 11th October 2010
Rev2: 14th October 2010
Rev1: 
1. SST_TTBL, the global task table declaration brought here from main. Extern
declaration removed since it is now redundant.


Rev2:
##################################################################################*/
