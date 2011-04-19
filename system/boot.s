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

/*################################################################################################################
CCOMSAT Project OnBoard Computer.
Bootup Sequence for the first build.

29 December 2009 (first write)
5  January 2010 (rev 1 with JTAG debug)
14th October 2010 (rev 2: Modifications for the single Stack RTOS )
Author: Nishchay Mhatre

Notes:
1. Meant to boot from the on chip flash that starts out at address 0x00100000, since that is how we have configured
the microcontroller to boot from using SAM-BA to load the image. Since this code is at the start of the Flash, we set
the exception  vectors here at the start.
2. The exception vectors might be loaded into RAM which is one MB away but to be on the safe side, we use LDR PC and
not Branch instrution for vector table.
3. The temporary vector table points only to reset because these exceptions might occur. Maybe by radiation or 
something. In that case, its better to reset than to loop infinitely.
5. For debugging only. Remove before final build
6. This subroutine conforms to APCS, passing the vector and handler addresses as args in regs 0 and 1 and takes back 
the resultant vectoring instructions.
7. Software interrupts are not yet defined. So this number is arbit

Changes: rev 1 (Nishchay M)

1. - ldr r0,_remap : this bug caused a prefetch abort by loading the value at address instead of address. As a res
ult, the program counter was loaded with an instruction instead of the address of one, causing a prefetch abort since
the address was not legal for the program.
fix: + ldr r0,=_remap

2. -bl _install_vector_add : This caused the relative address of the label reset to be loaded and a relative address 
constructed out of it. Which is wrong. Causes a wrong vector address. 

not changed:	LDR	r1,=__ram_start	
 	-	LDR	r0,=_reset		
	+	LDR	r0,=RESET_VECTOR_ABSOLUTE

3. The opcode of MOV pc.#0x100000 (reset vector) is strored directly into the RAM. Opcode was calculated. This was found 
to be the only way because the install_vector_ins function worked only with the addresses in RAM and reset is loaded 
in flash the LMA only is to be used for the access to this vector. Also Branch instruction works for PC relative
branches only and the LDR pc, pc is also program counter relative. Since absolutely address is used MOV-ing it into
PC is the simplest solution. Now vectoring works fine.

rev 2: (Nishchay M)

1. The following instruction results in a direct branch to the address in the AIC's Interrupt Vector Register. This 
   starts the ISR.  
	LDR	PC,[PC,#-0xF20]
   As such, the machine instruction for this is found to be 
	0xE51FFF20 
   This is defined as IRQ_VECTOR and stored at the IRQ vector word in the Exception vector table. The Low level IRQ_Handler 
   (defined in system/exceptions.s is thus deprecated for this version of the system software and should not be used. 
   It does not support nesting.)
 
 EOC
################################################################################################################*/



	.equ	I_Bit,	0x80				/*Standard definitions: Modes */
	.equ	F_Bit,	0x40

	.equ	USR_Mode,	0x10
	.equ	FIQ_Mode,	0x11
	.equ	IRQ_Mode,	0x12
	.equ	SVC_Mode,	0x13
	.equ	ABT_Mode,	0x17
	.equ	UND_Mode,	0x1B
	.equ	SYS_Mode,	0x1F

	.equ	STACK_PATTERN,	0xBABAABAB		/* Note 5 */
	.equ	RETURN_CASE,	0x0			/* Arbit number for now M-00 */

	.equ	CHECK_REMAP,	0x0B0B1337		/* Ram remap check word */
	.equ	BAL_OPCODE,	0xEA000000		/* Opcode of BAL instruction */
	.equ	RESET_VECTOR_ABSOLUTE,	0xE3A0F601	/* Instuction MOV pc,#100000 */

	.equ	IRQ_VECTOR,	0xE51FFF20		/* rev 2 change 1 */

	.extern		PABT_Handler			/*Declaration of the other handlers*/	
	.extern		UND_Handler		
	.extern		DABT_Handler		
	.extern		SWI_Handler
	.extern		IRQ_Handler
	.extern		FIQ_Handler
	.extern		__ram_start
	.extern		__stack_end__

							/*Start of code*/
	.text
	.code 32
	
	.global _entry
	.func	_entry


_entry:							/*Temporary boot time vector table. Note 3*/
	B	_reset					/* 0x00 Start of memory space. Branch to reset handler*/
	B	_reset					/* UNDEF vector  */
	B	_reset					/* SWI vector */
	B	_reset					/* Prefetch abort vector*/
	B	_reset					/* Data abort vector*/
	B	_reset					/* Reserved */
	B	_reset					/* IRQ vector */
	B	_reset					/* FIQ vector */
	
	.string "CSAT2010"				/* Just an identifier for the project*/
	.align 4

							/* Reset Handler. Enter the C code that will only set up the 
							critical hardware and then perform the Memory remap */

_reset:
	LDR	r1,=__ram_start				/*R2 holds reset vector address*/
	LDR	r0,=RESET_VECTOR_ABSOLUTE		/*Rev 1Change 2*/	
	STR	r0,[r1]					/*Install vector in proper location*/
	ADD	r1,r1,#4				/*Increment vector by 4*/	

	LDR	r0,=UND_Handler				/*Break 1*/
	BL	_install_vector_ins	

	LDR	r0,=SWI_Handler
	BL	_install_vector_ins

	LDR	r0,=PABT_Handler
	BL	_install_vector_ins

	LDR	r0,=DABT_Handler
	BL	_install_vector_ins

	LDR	r5,=CHECK_REMAP				/*For the reserved addr*/
	STR	r5,[r1]
	ADD	r1,r1,#4				/*IRQ vector address: 0x14+4 =0x18*/
	
	LDR	r0,=IRQ_VECTOR				/* Rev2 change 1*/
	STR	r0,[r1]					/*Install vector in proper location*/
	ADD	r1,r1,#4				/*Increment vector by 4*/	
			
	LDR	r0,=FIQ_Handler
	BL	_install_vector_ins

	LDR	r0,=_remap				/*Rev 1 change 1*/
	MOV	lr,r0
	LDR	sp,=__stack_end__
	B	low_level_hw_init

_remap:					
	MOV	r1,#0x14				/*Location of the reserved word in RAM*/
	MOV	r3,#0x1
	MOV	r4,#0xFFFFFF00				/*Memory remap control register*/
	LDR	r2,[r1]					/*Read from 0x14*/
	CMP	r2,r5					/*r5 still has the check value*/
	STRNE	r3,[r4]					/*Send REMAP command.*/
	B	_raminit				/*memory remap swaah: */


_install_vector_ins:					/* Note 6 */
	SUB	r2,r0,r1				/* r2 = handler - vector */
	SUB	r2,r2,#0x8				/*Account for the pipeline*/
	MOV	r2,r2,LSR #2				/*r2 shifted right by 2*/
	BIC	r2,r2,#0xFF000000			/*Clear topmost byte*/
	ORR	r0,r2,#BAL_OPCODE			/*OR with the BAL opcode. Vector is ready. Returned in R0*/
	STR	r0,[r1]					/*Install vector in proper location*/
	ADD	r1,r1,#4				/*Increment vector by 4*/
	MOV	pc,lr					/*return*/

							/* Initialization of RAM by copying the RAM mapped 
							sections from their LMA in ROM to the VMA in RAM*/

_raminit:
	LDR	r0,=__fastcode_load__			/* Fastcode section. Exception handlers and other fast RAM functions*/
	LDR	r1,=__fastcode_start__
	LDR	r2,=__fastcode_end__
1:
	CMP	r1,r2
	LDMLTIA	r0!,{r3}
	STMLTIA	r1!,{r3}
	BLT	1b
	
	LDR	r0,=__text_load__			/* Text section also loaded in RAM for faster operation. 25/6/2010 */
	LDR	r1,=__text_start__
	LDR	r2,=__text_end__
2:
	CMP	r1,r2
	LDMLTIA	r0!,{r3}
	STMLTIA	r1!,{r3}
	BLT	2b
	
	LDR	r0,=__data_load__			/* Data section of the programs */
	LDR	r1,=__data_start__
	LDR	r2,=__data_end__
3:
	CMP	r1,r2
	LDMLTIA	r0!,{r3}
	STMLTIA	r1!,{r3}
	BLT	3b
	
	LDR	r1,=__bss_start__			/* Zero initialize the BSS section*/
	LDR	r2,=__bss_end__
	MOV	r3,#0
4:
	CMP	r1,r2
	STMLTIA	r1!,{r3}
	BLT	4b
	
	LDR	r1,=__stack_start__			/* Fill stack with debug pattern */
	LDR	r2,=__stack_end__
	LDR	r3,=STACK_PATTERN
5:
	CMP	r1,r2
	STMLTIA	r1!,{r3}
	BLT	5b
							/* Initialization of the Stack Pointers for 
							different modes */

	MSR	CPSR_c,#(IRQ_Mode | I_Bit | F_Bit)
	LDR	sp,=__irq_stack_top__

	MSR	CPSR_c,#(FIQ_Mode | I_Bit | F_Bit)
	LDR	sp,=__fiq_stack_top__
	
	MSR	CPSR_c,#(SVC_Mode | I_Bit | F_Bit)
	LDR	sp,=__svc_stack_top__
	
	MSR	CPSR_c,#(SYS_Mode | I_Bit | F_Bit)
	LDR	sp,=__c_stack_top__
	
	MSR	CPSR_c,#(ABT_Mode | I_Bit | F_Bit)
	LDR	sp,=__abt_stack_top__
	
	MSR	CPSR_c,#(UND_Mode | I_Bit | F_Bit)
	LDR	sp,=__und_stack_top__
	
							/* Enable the FIQ in SVC mode*/

	MSR	CPSR_c,#(SVC_Mode | I_Bit | F_Bit)	/*Temporary disable of FIQ for test purposes: 9-3-2010*/

							/* Enter user Mode and user code main function */
	LDR	r7,=main
	MOV	lr,pc
	BX	r7
							/* Software interrupt, just in case theres a return */
	SWI	RETURN_CASE				/* Note 7 */
	
	.size	_start, . - _start
	.endfunc
	
	.end
