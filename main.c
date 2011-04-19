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
COEP Satellite Project: On Board Computer

Single Stack RTOS: Main Function

Author: Nishchay Mhatre

17th September 2010 (First write)

Notes:
1. All things marked *FR have been copied as is from FreeRTOS source code by Richard 
   Barry.


#####################################################################################*/

#include<stdlib.h>				// C standard includes
#include<stdint.h>

#include"task.h"				// RTOS includes
#include"SST_config.h"				// Configuration 
#include"portmacro.h" 				// Hardware specific macros : *FR
#include"AT91SAM7X256.h"		 	// H/W includes

#include"bob_tasks.h"				// Application includes

#include"trace.h"

						/* Hardware setup macro  */ 
#define prvSetupHardware()			/*Rev 1: Change1*/ \
{\
 portDISABLE_INTERRUPTS();\
 AT91C_BASE_AIC->AIC_EOICR = 0;\
 AT91C_BASE_RTTC->RTTC_RTMR = (0x1)|(AT91C_RTTC_RTTRST);\
 pio_init();\
 AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOA);\
 AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOB);\
}


/*#####################################################################################

 Setup the timer 0 to generate the tick interrupts at the required frequency. 

*#####################################################################################*/

extern void (SST_Tick_ISR)(void); 		// Required for setup of timer interrupt
extern void (SST_Comm_ISR)(void); 		// Required for setup of comm interrupt

static void prvSetupTimerInterrupt( void )		// *FR
{
AT91PS_PITC pxPIT = AT91C_BASE_PITC;

	/* Setup the AIC for PIT interrupts.  The interrupt routine chosen depends
	on whether the preemptive or cooperative scheduler is being used. */
	
	AT91F_AIC_ConfigureIt( AT91C_ID_SYS, AT91C_AIC_PRIOR_HIGHEST, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, ( void (*)(void) ) SST_Tick_ISR );
	
	/* Configure the PIT period. */
	pxPIT->PITC_PIMR = AT91C_PITC_PITEN | AT91C_PITC_PITIEN | portPIT_COUNTER_VALUE;

	/* Enable the interrupt.  Global interrupts are disables at this point so 
	this is safe. */
    AT91C_BASE_AIC->AIC_IECR = 0x1 << AT91C_ID_SYS;
}

/*#####################################################################################

 Setup the Comm ISR to be triggered at a PIO input change interrupt. 

*#####################################################################################*/

static void prv_Setup_Comm_Interrupt(void)
{
 
 AT91C_BASE_PIOA->PIO_IER = (1 << config_PIO_INTR_PIN);		// Configure the PIOA  for interrupt
 AT91C_BASE_PIOA->PIO_IDR = (0 << config_PIO_INTR_PIN);
 
 AT91F_AIC_ConfigureIt(AT91C_ID_PIOA, AT91C_AIC_PRIOR_HIGHEST-1,config_IRQ_TYPE,(void(*)(void))SST_Comm_ISR ); 						// Configure the AIC properly
  
  AT91C_BASE_AIC->AIC_IECR = 1 << AT91C_ID_PIOA;	// Enable it

  return;
}



/*#####################################################################################
 
 Start of the RTOS loop: Idle task

##################################################################################### */ 

extern uint8_t SST_current_priority ;
uint32_t system_time;

void StartLoop()
{
 SST_current_priority = 0;			// Set current priority to idle
 
 prvSetupTimerInterrupt();			// Setup all Interrupts here
 prv_Setup_Comm_Interrupt();
 
 portENABLE_INTERRUPTS();
 
 system_time = 4;

 while(1)					// The only task , idle task, that runs in
	{					// infinite loop. Power management algorithm 
						//to be put here or in the idle hook
	 	;
	 #if configUSE_IDLE_HOOK == 1
	 idle_hook();				/* Optional specific task to do in idle mode*/ 
	 #endif
	}
 
 asm volatile ("SWI 0x1");			// SWI interrupt to trap a return from loop
}

/*#####################################################################################

Main function. Payload of boot code.

##################################################################################### */ 

extern uint8_t SST_ready_set;			// Global used for scheduling

uint32_t current;
log_record data_log[LOGSIZE];			// This is used for debug only

int main(void)
{
 update_log(MAIN, START|NOT_ISR);

 prvSetupHardware();				// Initial setup of real time clcok, AIC and  memory manager
 
 SST_ready_set =0;
 SST_task_create(one,1, NULL);			// Create tasks. Task functions
 SST_task_create(two,2, NULL);			// defined in app/bob_tasks.c
 SST_task_create(three,3, NULL);	
 
 StartLoop();					// Call Idle task

 asm volatile ("SWI 0x1");			// SWI interrupt to trap a return from loop
	
}

/*#####################################################################################
Changelog:

17th September 2010 (First write)
14th October 2010 (Rev 1: Nishchay Mhatre)

Rev1:
1. Redundant instructions from the pio configuration removed. PIO clock activation
   using the PMC_PCER done after the PIO configuration function returns

##################################################################################### */ 
