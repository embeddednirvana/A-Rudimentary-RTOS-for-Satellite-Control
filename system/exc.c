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
High Level Exception Handlers and Interrupt Service Routines.

Nishchay Mhatre
18th Sept 2010

Notes:
1. Code reused from previous builds of freeRTOS. Modifications are the components needed 
   for the Single Stack version

2. ** Important **
  In ARM , an interrupt causes a change of stack pointer to the IRQ stack pointer.
  It should be taken care of or else the notion of the single stack will be lost.
  The stack pointer of the SVC mode (in which the tasks will run), should be
  copied to the IRQ stack mode on interrupt entry. Or another register should be
  used as stack pointer. But the latter option seems to be impossible for
  compiler generated code.

  Otherwise the IRQ stack can be given half of the total stack space. The ISR's
  and all that they call will run on that stack and seamlessly return to the
  original stack when the IRQ frame is cleared (IRET action). 

  Note again that IRET is different from EOI. IRQ priority level stage ends at
  EOI regardless of what is on which stack.

3. Template for ISR in the single stack system 

void isr() __attribute__ (interrupt("IRQ"));	//  Attribute makes GCC build a proper prolouge-epilogue
{
 uint8_t saved_priority;	// Has to be there
 SST_ISR_enter();
 
 //ISR body. This may be a simple hardware action or a deferring of action to
 //another task by posting an event.

 SST_ISR_exit();

 return;			//
}


##################################################################################### */ 

#include"AT91SAM7X256.h"
#include<stdint.h>
#include"task.h"
#include"SST_config.h" 
#include"portmacro.h" 
#include"driver_pio.h" 
#include"trace.h" 

extern log_record data_log[LOGSIZE];
uint32_t extern current;

uint8_t extern SST_current_priority;		//This is a global for priority level of the currently running entit

#define IRQ_PRIORITY 255			// Highest Priority
			

/*##################################################################################### 

// Macros for Single Stack specific Interrupt entry and exit

##################################################################################### */ 

#define EOI_command()				/*End Of Interrupt*/ \
{ asm volatile \
	(\
	"STMFD	sp!,{r1,r6}		\n\t" /*Saving the regs*/ \
	"LDR     r6,=0xFFFFF130		\n\t" /*Address of the EOI */\
      	"STR     r1,[r6]		\n\t" /*Anything can be written into the EOI reg*/ \
	"LDMIA	sp!,{r1,r6}		\n\t" \
	);\
}

#define make_irq_nestable() 		/*This saves IRQ mode SPSR to provide for nesting*/ \
{asm volatile (\
		"MRS r1,CPSR	\n\t"\
		"MRS r2,SPSR	\n\t"\
		"PUSH {r1,r2}	\n\t"\
	      );}

#define restore_irq_status()		/*The complementary procedure to the above*/ \
{asm volatile ("POP {r1,r2}	\n\t"\
		"MSR cpsr, r1	\n\t"\
		"MSR spsr, r2	\n\t"\
	      );}

#define SST_ISR_enter()			/*Save the state required by the SST*/ \
{\
SST_saved_priority = SST_current_priority;\
SST_current_priority = IRQ_PRIORITY;\
make_irq_nestable();				/*Save Status registers*/ \
portENABLE_INTERRUPTS();\
}

#define SST_ISR_exit()\
{\
portDISABLE_INTERRUPTS();\
restore_irq_status();				/*Restore status registers */\
EOI_command();					/* End of Interrupt command*/\
SST_current_priority = SST_saved_priority;\
SST_Scheduler();				/*Call scheduler*/\
}						/* No explicit IRQ enable here 
						since return from ISR is immediately 
						after this*/ 


/*##################################################################################### */ 
__attribute__ ((section(".text.fastcode"))) void C_ANO_HANDLER(uint32_t addr,uint32_t instr,uint32_t mode)
{
 addr = addr+0;
 return;
}

#define KEY 0xA5

__attribute__ ((section(".text.fastcode"))) void C_SOFT_RESET(void)
{
 update_log(CRASH,(START|ISR));
 AT91C_BASE_RSTC->RSTC_RCR = ((KEY<<24)|(1<<0));
 return;
}

/*##################################################################################### 

Declare the ISR's with proper attributes

##################################################################################### */ 

void SST_Tick_ISR() __attribute__ ((interrupt("IRQ")));
void SST_Comm_ISR() __attribute__ ((interrupt("IRQ")));


/*##################################################################################### */
/* System Tick Interrupt */

extern uint32_t system_time;		// System time in ticks

void SST_Tick_ISR()
{
 uint8_t SST_saved_priority;

 static uint32_t task_ctr_1;		// Counter for a particular task. One
 					//for each periodic task

 SST_ISR_enter();
 
 update_log(TICK,(START|ISR));
 
 system_time++;				// Increment number of ticks
 task_ctr_1++;				

 #if (configUSE_TICK_HOOK == 1)		// Optional processing to be done every tick
 tick_hook();
 #endif


 if(task_ctr_1 == TASK_PERIOD_1)	// In order of priority,
 	{				// check readiness of periodics
	 task_ctr_1 = 0;
	 SST_post_event(1,1);		// For task 1
	}

 
 update_log(TICK,(END|ISR));
 
 SST_ISR_exit();
 return;
}

/*##################################################################################### */
/* Communication ISR*/

void SST_Comm_ISR()
{
 uint8_t SST_saved_priority;
 uint8_t A1, dummy_usart_buffer;

 SST_ISR_enter();
 
 update_log(COMM,(START|ISR));
 
 A1 = dummy_usart_buffer;
 
 SST_post_event(1,2);		// For task 2
 
 pio_finish_irq();

 update_log(COMM,(END|ISR));
 
 SST_ISR_exit();
 return;
}
