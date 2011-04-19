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
/*################################################################################################
COEP Satellite Team On Board Computer

Header file for Task API of the Single Stack RTOS

##################################################################################################*/

#ifndef TASK_H
#define TASK_H

#include<stdint.h>
#define  EVENT_Q_MAX 16			// Supports queuing of 16 async events

typedef void(*task)(void*) ;		// Void * is for the structure we may need to pass as parameter to the function.


					// Task Control Block


typedef struct _tcb			// Note 1 
{
 task f;				// Task function 
 uint8_t priority;			// For scheduling
 
 uint8_t eventq[16];			//event queue, circular, static length

 uint8_t head;				// First free  space in the q
 uint8_t tail;				// Last occupied space in the q
 
 void*param;				// Parameter for function call
 void* extra;				// Structure extension. Programmer controlled. Future scope
} TCB;

					// Task API declarations


void SST_task_create(task f, uint32_t priority, void*parameter); 

void SST_Scheduler();

void SST_post_event(uint8_t e, uint32_t p);	// e= event number, p = priority 

#endif	/*TASK_H*/  
