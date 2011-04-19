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
/*##################################################################################################################
CCOMSAT Project On Board Computer

Exception Handler Routines.

31st December 2009 (first write)
4th March (test with RTOS)

Changes:
1. In FIQ Handler (line 129)
	- AND r10,r9,LSR#26 
	+ EOR r10,r9,LSR#25
	The previous was not mapping the input properly to output
2. FIQ mask changed from 0xF to 0x6... makes no difference
3. In IRQ handler:(line 106)
	- LDR pc,[r12]
	+ MOV pc,r12	
	The previous caused an Abort since the contents of the vector address is an instruction and not a location
	Silly programmer error.
4. In SWI, DABT, ANO handlers, there was no "mov lr,pc" before the branch to the C handlers resulting in no change of
   mode and stack pointers upon return since C handlers returned directly to mainline using "B LR"
5. In DABT and ANO handlers the CPSR is being passed into the C function but SPSR should be passed since the mode of the
   interrupted state is important.
   Both 4 and 5 Fixed (8 March 2010, Test with RTOS)

Coded by: Nishchay Mhatre

Notes:
1. Protect mode is kept enabled during test phase as JTAG debugging will be needed and in this case the JTAG should 
not read the IVR, thus making it interfere with the software. But therefore in this mode the IVR has to be explicitly
written so that the interrupt is acknowledged by the AIC, Any value will do. So value Dummy is used.

2. Interrupts are turned off when the IRQ exception is acknowledged by the processor in hardware itself. These have to
Turned on manually by the exception handler. Thus they are turned on only after the Interrupt has been acknowledged as 
a vector. Only then the nesting is enabled. The stack frame saved is suitable for the nesting.

3. We use here the IRQ mode stack itself and not the system mode stack for the sake of compartmentalization.
On this stack registers r0-r7 are pushed alongwith r12 and lr. SP is not pushed because it will be properly decremented
by a nested interrupt too. But SPSR is pushed in below the other registers by keeping a frame pointer sp+10*4

4. The interrupts are turned back off when the interrupt source of the interrupt is to be cleared. If this is done in a
nested interrupt then the interrupt one level higher than this gets its mode back anyway after return as the spsr is 
restored, so there is no loss of context.

5. The Anomaly handler is unique as it handles three kinds of exceptions. Undefined, data abort and prefecth abort 
At the bottom of the abt stack is used as a store for the address of the instruction which caused the fault.
The exception handler checks the value in this location to see if this is the first handling of this fault or the 
second. Now if this same instruction causes another fault, the function C_SOFT_RESET is called which resets the CPU from
software in order to reload the execution. This is like a software watchdog.

6. Undef and prefetch share the ANO_handler since their return instructions and so the LR are the same when they try to
return to the errant instruction. Whereas Data abort has a different value

7. The C handler for the anomaly receives the arguments 1.Address of errant instr,2.Instruction itself 3.Mode of 
exception in the registers r0,r1,r2

8. Infinite ISR bug. (aka Ghanchakkar):
   Anatomy of the Ghanchakkar Bug.
   When the call to a C ISR is made or an SWI is called from the SWI handler, the assembly instruction
   sequence is:

   mov	lr,pc
   mov	pc,<register with function pointer>

   C interrupt handlers call the scheduler before return. 
   
   vTaskSwitchContext();	<--- Call to scheduler 
   portRESTORE_CONTEXT();	<---- Macro

   The macro portRESTORE_CONTEXT restores the context of the highest priority task that is ready to 
   run (task Alpha). 
   This is accessed from the pxCurrentTCB which is manipulated accordingly by the scheduler to reflect the TCB
   of the current task Alpha.
   
   This gives rise to 2 cases:
	1. Higher priority task is ready to run:
		- That task is restored on return from ISR.
		*** ISR Never returns to the IRQ Handler ***
	2. Task that was interrupted is itself Task Alpha:
		- In this case a proper return to IRQ handler is warranted. But this does not happen as in 
		portRESTORE_CONTEXT (portISR.c) , the last instruction is :
		SUBS	pc,lr#4
		
		This decrements lr by 4 and puts it in pc.
		Recap to our original C ISR call: mov lr,pc
		If this instruction is at location X then pc = X+8
		Then LR = X+8
		But after the portRESTORE, LR = X+4 
		So pc <-- X+4
		That means we return to the instruction after mov pc,lr
		i.e.: MOV	pc, <register with ISR>

		And it all starts again: In infinite loop.
   This is why,in the current build, everything works fine till both tasks are delayed and its the turn of 
   idle task to run. So the IRQ to interrupt this is the tick which does not find anything higher and so makes the 
   bad return to X+4.

######################################################################################################################*/
/*M-03*/
/*Standard definitions*/

	.equ	IRQ_Off,	0x80
	.equ	FIQ_Off,	0x40

	.equ	USR_Mode,	0x10
	.equ	FIQ_Mode,	0x11
	.equ	IRQ_Mode,	0x12
	.equ	SVC_Mode,	0x13
	.equ	ABT_Mode,	0x17
	.equ	UND_Mode,	0x1B
	.equ	SYS_Mode,	0x1F
	
	.equ	VECTOR,		0xFFFFF100
	.equ	END_OF_IRQ,	0xFFFFF130
	.equ	DUMMY,		0x1337
	
	.equ	FIQ_MASK,	0xF
	.equ	FIQ_IP_PORT,	0xFFFFF43C
	.equ	FIQ_OP_PORT,	0xFFFFF638
	.equ	PIO_OWER,	0xFFFFF6A0

	.extern C_SWI_HANDLER
	.extern C_ANO_HANDLER
	.extern	C_SOFT_RESET
	.extern C_FIQ

/*Start of code*/
	.text
	.code 32
	.section .text.fastcode
/*################################################################################################################*/
/*IRQ Handler	M-06 */
/*################################################################################################################*/
	.global IRQ_Handler
	.func	IRQ_Handler
IRQ_Handler:
	
	SUB	sp,sp,#4			/*Space kept for spsr. STacking policy, Note 3*/
	STMFD	sp!,{r0-r7,r12,lr}		/* Store registers on the stack */
						
	MRS	r7,spsr				/*Stack SPSR also to enable nesting*/
	STR	r7,[sp,#10*4]

	LDR	r0,=VECTOR			/*Address of AIC_IVR*/
	LDR	r1,=DUMMY
	
	LDR	r12,[r0]			/*Get ISR vector from the AIC*/
	STR	r1,[r0]				/*Dummy write for Protect mode. Note 1.*/	

	MSR	CPSR_c,	#(IRQ_Mode|FIQ_Off)		/*FIQ and IRQ both enabled. Nestable now. Note 2*/
	
	ADD	lr,pc,#4			/* Note 8*/
	MOV	pc,r12				
	
	MSR	CPSR_c,	#(IRQ_Mode|IRQ_Off|FIQ_Off)	
						/*Turn off interrupts just */
						/*before the critical op of clearing the IRQ*/
						/*Note 4*/
	LDR	r6,=END_OF_IRQ			/*AIC register signalling end of Intr*/ 
	STR	r1,[r6]				/*Write anything*/

	LDR	r7,[sp,#10*4]			/*Doubtful whether or not to use register writeback here*/
	MSR	spsr,r7				/*Restore spsr*/
	LDMFD	sp!,{r0-r7,r12,lr}		/*Restore the other regs*/
	ADD	sp,sp,#4
	
	SUBS	pc,lr,#4			/*Return from ISR. Note 4*/

	.size	IRQ_Handler, . - IRQ_Handler
	.endfunc

/*################################################################################################################*/
/*FIQ Handler	M-07*/
/*################################################################################################################*/
	.global FIQ_Handler
	.func	FIQ_Handler

FIQ_Handler:					/*No stack operations. Banked registers r8-r12 rock! */
	LDR	r8,=FIQ_IP_PORT			/*Load address of input port into r8*/
	LDR	r9,[r8]				/*Read input*/
	
	LDR	r8,=PIO_OWER			/*Load address of OWER into r8*/
	MOV	r12,#0x6			/*Enable pin 1 and 2 in OWER*/
	STR	r12,[r8]

	MOV	r9,r9,LSR #25
	EOR	r10,r9,#FIQ_MASK		/*XOR the result with the mask*/
	LDR	r11,=FIQ_OP_PORT		/*Load o/p port address into r11*/	
	STR	r10,[r11]			/*Output send*/

	SUBS	pc,lr,#4			/*Vaapis*/

	.size	FIQ_Handler, . - FIQ_Handler
	.endfunc

/*################################################################################################################*/
/*SWI Handler*/
/*################################################################################################################*/
	.global SWI_Handler
	.func	SWI_Handler
SWI_Handler:
	
	SUB	sp,sp,#4			/*Space kept for spsr*/
	STMFD	sp!,{r0-r12,lr}		/* Store all the system mode registers on the stack */
	MRS	r7,spsr				/*Stack SPSR also to allow for nesting later*/
	STR	r7,[sp,#14*4]
	
	LDR	r0,[lr,#-4]			/*Load SWI instruction into the r0*/
	BIC	r0,r0,#0xFF000000		/*Top eight bits masked off*/
	
	MOV	lr,pc				
	BL	C_SWI_HANDLER			/*Enter the C handler of the swi which return to next addr and
						  has number passed as argument in r0*/	

	LDR	r7,[sp,#14*4]
	MSR	spsr,r7
	LDMFD	sp!,{r0-r12,lr}		/*Unstack em*/
	ADD	sp,sp,#4			
	
	MOVS	pc,lr				/*Vaapis*/

	.size	SWI_Handler, . - SWI_Handler
	.endfunc

/*################################################################################################################*/
/*UNDEF handler*/
/*################################################################################################################*/
	.global	UND_Handler
	.func	UND_Handler

UND_Handler:

	MSR	CPSR_c,#(UND_Mode|IRQ_Off|FIQ_Off)
	B	ANO_Handler

	.size	UND_Handler, . - UND_Handler
	.endfunc

/*################################################################################################################*/
/*Prefectch Abort  handler*/
/*################################################################################################################*/
	.global	PABT_Handler
	.func	PABT_Handler

PABT_Handler:
	
	MSR	CPSR_c,#(ABT_Mode|IRQ_Off|FIQ_Off)
	B	ANO_Handler

	.size	PABT_Handler, . - PABT_Handler
	.endfunc

/*################################################################################################################*/
/*Data Abort  handler M-08*/
/*################################################################################################################*/
	.global	DABT_Handler
	.func	DABT_Handler

DABT_Handler:					/*Note 6*/
	
	MSR	CPSR_c,#(ABT_Mode|IRQ_Off|FIQ_Off)	

	SUB	sp,sp,#4			/*Space kept for spsr and addr of exception causing instruction. This
						Should be 8 but the very bottom, where the errant ins is kept should
						not be lost between exceptions*/

	STMFD	sp!,{r0-r12,lr}			/* Store all the system mode registers on the stack */
	MRS	r7,spsr				/*Stack SPSR also to allow for nesting later*/
	STR	r7,[sp,#14*4]

	SUB	r0,lr,#8			/*Address of ins that caused the exception*/
	LDR	r1,[r0]				/*The culprit*/
	LDR	r3,[sp,#15*4]			/*The instruction on the stack*/
	CMP	r1,r3				/*See if the same instruction has repeated*/
	BNE	C1

	MOV	lr,pc
	BAL	C_SOFT_RESET			/* Soft reset C function in case of second same exception*/

C1:	
	STR	r1,[sp,#15*4]			/*The instr that caused THIS exception is stored*/
	MRS	r2,SPSR
	AND	r2,r2,#0x1F			/*Mode stored in R2*/

	MOV	lr,pc
	BL	C_ANO_HANDLER			/*Handle the exception in C. R0 has the errant address and R1 the instr*/

	LDR	r7,[sp,#14*4]
	MSR	spsr,r7
	LDMFD	sp!,{r0-r12,lr}			/*Unstack em*/
	ADD	sp,sp,#4			/*Now only the bad instruction is on the stack*/
	
	SUBS	pc,lr,#8				/*Vaapis*/
	
	.size	DABT_Handler, . - DABT_Handler
	.endfunc

/*################################################################################################################*/
/*Anomalous condition (ANO) Handler: M-08*/

/*################################################################################################################*/
	.global ANO_Handler
	.func	ANO_Handler
ANO_Handler:					/*Note 5 */
	SUB	sp,sp,#4			/*Space kept for spsr and addr of exception causing instruction. This
						Should be 8 but the very bottom, where the errant ins is kept should
						not be lost between exceptions*/

	STMFD	sp!,{r0-r12,lr}			/* Store all the system mode registers on the stack */
	MRS	r7,spsr				/*Stack SPSR also to allow for nesting later*/
	STR	r7,[sp,#14*4]

	SUB	r0,lr,#4			/*Address of ins that caused the exception*/
	LDR	r1,[r0]				/*The culprit*/
	LDR	r3,[sp,#15*4]			/*The instruction on the stack*/
	CMP	r1,r3				/*See if the same instruction has repeated*/
	BNE	C2

	MOV	lr,pc
	BAL	C_SOFT_RESET			/* Soft reset C function in case of second same exception*/

C2:	
	STR	r1,[sp,#15*4]			/*The instr that caused THIS exception is stored*/
	MRS	r2,SPSR
	AND	r2,r2,#0x1F			/*Mode stored in R3*/
	
	MOV	lr,pc
	BL	C_ANO_HANDLER			/*Handle the exception in C. R0 has the errant address and R1 the instr*/

	LDR	r7,[sp,#14*4]
	MSR	spsr,r7
	LDMFD	sp!,{r0-r12,lr}			/*Unstack em*/
	ADD	sp,sp,#4			/*Now only the bad instruction is on the stack*/
	
	SUBS	pc,lr,#4			/*Return to the same instruction that caused the exc*/
	
	.size	ANO_Handler, . - ANO_Handler
	.endfunc
/*################################################################################################################*/
