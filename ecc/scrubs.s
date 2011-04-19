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
CCOMSAT Project On Board Computer 
Data Scrubbing function for error correction of the code in the program memory

28th May 2010 (First Write)
Nishchay Mhatre

Notes:
1. This is called by the RTOS periodically.
2. Uses the hamming error correction function ECC_TMR_2 which accepts as parameters 
	- The address instruction to be corrected in the r0
	- The base address of the check bytes in the other 3 argument registers
3. Incrementing the addresses and proceeding from the start of the code section of the RAM to the end
   is the job of this function.
4. Strategy:
   Check bytes are calculated by analysis program run on the host machine and made into binary, raw data
   files which are programmed into on chip flash at porting time by the host machine. The starting points
   of these files(which are just arrays) are hardcoded into this code.
#################################################################################################*/	
	.extern __text_start__
	.extern __text_end__
	
	.equ __base_check_1, 0x108000
	.equ __base_check_2, 0x10A000
	.equ __base_check_3, 0x10C000
	
	.extern ECC_TMR_2

	.equ	OFFSET,	1000		/*Arbit: This should determined separately*/ 
 	
	.text
	.code 32
	.section .text.fastcode		/*Special section*/

	.global DATA_SCRUB
	.func	DATA_SCRUB

DATA_SCRUB:				/*Initialization*/
	STMFD	sp!,{lr}		/*Push link register on stack*/
	LDR	r4,=__text_start__	/*Load start address. R4 is now the address reg*/
	LDR	r11,=__text_end__	/*r11 now has the Addr of the last insr to test*/
	LDR	r1,=__base_check_1	/*Pass the addresses of chkbytes to the ECC fn*/
	LDR	r2,=__base_check_2
	LDR	r3,=__base_check_3
	
Begin:
	LDR	r0,[r4]			/*r0 is for passing instr to ECC function and for receiving corrected instr */
	BL	ECC_TMR_2		/*Call function ECC with TMR on chk bits, passing to them the current instr  						  and three chkbyte addresses corresponding to that instr*/
	
	STR	r0,[r4]			/*Put the corrected instr back into its RAM location*/
	CMP	r4,r11

	ADDCC	r4,#0x4			/*Next instruction *Word* */
	ADDCC	r1,#0x1			/*and corresponding chk *Bytes* */
	ADDCC	r2,#0x1
	ADDCC	r3,#0x1
	BCC	Begin			/*Branch if r4 < r11*/

Exit:
	LDMFD	sp!,{lr}		/*Pop LR from stack*/
	MOV	pc,lr			/*Bye*/
	
	.size	DATA_SCRUB, . - DATA_SCRUB
	.endfunc
	.end
