OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_entry)


MEMORY
{
	FLASH (rx) : ORIGIN = 0x00100000, LENGTH = 256K
	RAM (rwx) : ORIGIN = 0x00200000, LENGTH = 64K
}

C_STACK_SIZE = 0;	
IRQ_STACK_SIZE = 1024;
FIQ_STACK_SIZE = 0;
UND_STACK_SIZE = 512;
ABT_STACK_SIZE = 512;
SVC_STACK_SIZE = 1024;
USR_STACK_SIZE = 0;

HEAP_SIZE = 0;	/*Mr.Heap*/

__stack_end__ = 0x00200000 + 64K - 4;

SECTIONS

{
 .reset : { 			/*Ze bootup code*/
	*boot.o (.text)
	*low_level_init.o (.text)
      	. = ALIGN(0x4);
      	} >FLASH

 .ramvect : {			/*Reserving first 40 bytes for vector table*/
 	__ram_start = .;
	. += 0x40;
 	} >RAM

  .fastcode : {
  	__fastcode_load__ = LOADADDR (.fastcode); 	/*LMA is in flash text section*/
	__fastcode_start__ = .;				/*VMA is the current location, ie after vector table*/
	*(.glue_7t) *(.glue_7)			/*veneer functions for ARM-Thumb interworking, compiler generated*/
	*(.text.fastcode) 			/* All the functions compiled with fastcode attribute*/		
						/*More functions to be put in yet*/
	. = ALIGN(0x4);
	__fastcode_end__ = .;			
	} >RAM AT>FLASH				/*Link to VMA in RAM and LMA in flash and load at LMA*/

  .text : {
  	CREATE_OBJECT_SYMBOLS
  	__text_load__ = LOADADDR (.text); 	/*LMA is in flash text section*/
	__text_start__ = .;				/*VMA is the current location, ie after vector table*/
	*(.text .text.* .gnu.linkonce.t*)
	*(.glue_7t) *(.glue_7)			/*In fastcode but just in case*/
	*(.rodata .roadata.* .gnu.linkonce.t*)
	. = ALIGN(0x4);
	__text_end__ = .;			
	} >RAM AT>FLASH

  
  .data : {
  	__data_load__ = LOADADDR (.data);
	__data_start__ = .;
	*(.data .gnu.linkonce.t*)
	. = ALIGN(0x4);
	__data_end__ = .;
	} >RAM AT>FLASH

  .output : { 
        __output_start = .;
	*parity.o (.data)
	. = ALIGN (4);
	} >FLASH

  .bss : {
  	__bss_start__ = .;
	*(.bss .gnu.linkonce.t*)
	. = ALIGN(0x4);
	__bss_end__ = .;
	} >RAM

 .heap : {
 	__heap_start__ = .;
	. += HEAP_SIZE;
	. = ALIGN(0x4);
	__heap_end__ = .;
 	} >RAM AT>RAM		/*redundant looking AT>RAM is required due to a code sourcery bug*/

 .stack : {			/*Arranging stacks in what is expected to be decreasing order of size*/
 	__stack_start__ = .;

	. += C_STACK_SIZE;
	. = ALIGN(0x4);
	__c_stack_top__ = .;
	
	. += IRQ_STACK_SIZE;
	. = ALIGN(0x4);
	__irq_stack_top__ = .;
	
	. += SVC_STACK_SIZE;	/*Note that SWI runs in SVC mode and so uses the SVC stack*/
	. = ALIGN(0x4);
	__svc_stack_top__ = .;

	. += ABT_STACK_SIZE;
	. = ALIGN(0x4);
	__abt_stack_top__ = .;
 
	. += UND_STACK_SIZE;
	. = ALIGN(0x4);
	__und_stack_top__ = .;
	
	. += FIQ_STACK_SIZE;
	. = ALIGN(0x4);
	__fiq_stack_top__ = .;
	
	. += USR_STACK_SIZE;
	. = ALIGN(0x4);
	__usr_stack_top__ = .;



 	} >RAM AT>RAM		/*redundant looking AT>RAM is required due to a code sourcery bug*/

 _end = .;
}
