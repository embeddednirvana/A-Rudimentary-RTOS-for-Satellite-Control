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
CCOMSAT Project On Board Computer

Error correction of instructions using Hamming (38,32) for single bit errors and 
Triple Modular redundancy in the Check bytes.

18th May 2010 (First Write)
Nishchay Mhatre

Notes:
1. Registers 0-4 and 11 are used by the calling function. R5-R10 are available for use in this
function. If called by the exception handler, the handler will have to stack all of its own regs,
which it does anyway.
2. At the end of the parity check on the 32 bits of the instruction the answer got is the position
number alias and not true position of bit. Alias function is ready. Takes a complexity which is 
deterministic and takes same number of cycles for all errors from bit 0 to 31 . But this is not 
optimal. Due to heavy use of conditionals, the non executed components become NOP's.
There are no branches so no pipeline flushes here. A tradeoff study is needed. 25th May 2010. 
Executed using conditionals on 26th May. Need further optim, but in later versions after version 

4. BUG: If r4(parity) EOR r1(chkbyte) == 0 then, the instr is error free and the shift and EOR need
not be done.  The reverse alias is also  needed here. <Removed on 25th May, same day as discovered>
5. Too many B instructions. Scope for optimisation.
6. For detailed description of the program logic, refer to ecc_doc.txt in the /documentation 

###############################################################################################*/

/*Definitions*/
	
	.data

array_alias:			
	.byte 3, 5,6,7,9,10,11,12,13,14,15, 17,18,19,20,21,22,23,24,25,26,27,28,29,30,31, 33,34,35,36,37,38

	.text
	.code 32
	.section .text.fastcode

	.global ECC_TMR_2
	.func	ECC_TMR_2

ECC_TMR_2:			/*Currently assuming that the exception handler has left instruction in r0 */
	
	
	LDRB	r5,[r1]			/*Check byte 1 LDRB needed here*/
	LDRB	r6,[r2]			/*First redundant byte*/
	EOR	r10,r5,r6		
	CMP	r10,#0
	BEQ	L2

	LDRB	r7,[r3]			/*Second redundant byte*/
	EOR	r10,r5,r3		
	CMP	r10,#0
	BNE	L3			/*Check byte 1 and 2nd red byte are not same, meaning the two red bytes are in
					  Majority and so one of them should be used*/

L2:	MOV	r10, r5			/*Check byte 1 is part of majority,so use it*/
	B	Parity_Calc

L3:	MOV	r10, r7			/*Use 2nd red byte*/
	
	/*EC starts here*/
/* R0-R3: Arguments. R4,R11: Used by the caller. R5-R9 free from this point. R10: Check byte */

Parity_Calc: 

	LDR	r5,=array_alias		/*Base*/
	MOV	r6,#0			/*Initialize Index*/
	MOV	r7,#0			/*R7: register tracking participating chk bits ,initialize to zero*/
	MOV	r9,#0x1

L4:	MOV	r8,r9,LSL r6		/* r8 <-- (1<< r6) */
	AND	r8,r0,r8		/* instr & (1<<index) */
	CMP	r8,#0			
	BEQ	L5
	LDRB	r8,[r5,+r6]		/*r6 <-- alias(index)*/

	EOR	r7,r7,r8		/* XOR the participating bit register with the number of the tested bit */		

L5:	ADD	r6,#1			/*index ++ */
	CMP	r6,#32
	BLT	L4

	/*The last step: Parity check*/

Parity_Check:

	EOR	r8, r10, r7		/* r6 <-- chk_byte XOR parity_byte */
	MOVNE	pc,lr			/*Return to caller without change since instruction is correct*/
	
	CMP	r8,#0x20	/*Doubtful, can the previous CMP influence the CS in the next iteration?*/
	SUBCS	r8,r8,#0x7
	EORCS	r0, r9, LSL r8	
	MOVCS	pc,lr			/*Return to caller*/
	
	CMPCC	r8,#0x10
	SUBCS	r8,r8,#0x6
	EORCS	r0, r9, LSL r8	
	MOVCS	pc,lr			/*Return to caller*/
	
	CMPCC	r8,#0x08
	SUBCS	r8,r8,#0x5
	EORCS	r0, r9, LSL r8	
	MOVCS	pc,lr			/*Return to caller*/
	
	CMPCC	r8,#0x04
	SUBCS	r8,r8,#0x4
	EORCS	r0, r9, LSL r8	
	MOVCS	pc,lr			/*Return to caller*/
	
	CMPCC	r8,#0x2
	SUBCS	r8,r8,#0x3
	EORCS	r0, r9, LSL r8	

/*Register r0 now contains the corrected instruction. It has to be put back into the execution. Not figured this out 
yet. Either put it in its RAM addr or into the pipeline. But this does not work on ROM address*/

	MOV	pc,lr			/*Return to caller*/
	
	.size	ECC_TMR_2, . - ECC_TMR_2
	.endfunc

	.end

