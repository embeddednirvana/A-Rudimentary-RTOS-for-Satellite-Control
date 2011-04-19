/*##################################################################################### 
Copyright 2011 Shravan Aras

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
/* Author : Shravan Aras.
 * Revised : 12.2.10  
 * USART1 pins have been added. 
 * Revised : 26.05.10
 * SPI Chip select pin (SS) has been given back to the pio so it can now be controlled manually.
 * Revised : 28.05.10 (R3)
 * On AT91SAM7X-EK PA13 is connected to the internal flash memory card detector hence we use it to manually
 * assert / deassert the chip.
 * PA12 has been connected internally to Atmel Serial Flash memory via the J19 jumper. Hence it 
 * cannot be used with the board.
 * */ 
/*------------------------- PIO DRIVERS. -------------------------------------------------*/

#ifndef DRIVER_PIO_H
#define DRIVER_PIO_H

#define PIOA AT91C_BASE_PIOA
#define PIOB AT91C_BASE_PIOB

/*Reserved pins are those pins which are to be multiplexed with external peripheral devices.*/
/*Multiplexing on port A. */
/* (0x1<<12) has been removed on 26.05.10 as per revision 2. */
/* (0x1<<13) has been removed on 28.05.10 as per revision 3. */
#define RESERVEDA (0x1<<29)|(0x1<<30)|(0x1<<14)|(0x1<<15)|(0x1<<16)|(0x1<<17)|(0x1<<18)|(0x1<<5)|(0x1<<8)|(0x1<<6)|(0x1<<7)|(0x1<<9)
/*Multiplexing on port B. */
#define RESERVEDB (0x1<<19)|(0x1<<20)|(0x1<<21)|(0x1<<22)|(0x1<<25)|(0x1<<24)|(0x1<<23)|(0x1<<26)
/*The value to be stored in the peripheral select registers. */
#define PA_PORTA (0x1<<0)|(0x1<<4)|(0x1<<9)|(0x1<<6)|(1<<29)|(1<<30)
#define PA_PORTB (0x1<<10)
#define PB_PORTA 0x00000000
#define PB_PORTB (0x1<<6)

typedef struct _pio_port{
	uint32_t pins;			/*Stores the 32-bit hex. pins to be allocated to this peripheral device. */
}PORT;

/*Prototype of the functions. */
uint32_t pio_output(PORT*,uint32_t logic);
uint32_t pio_input(PORT*);
void pio_init();


#define pio_finish_irq()/*This currently works only for PIOA as address of PIO_ISR is coded into it*/ \
{\
 asm volatile(  "PUSH	{R1,R2}	\n\t"\
 		"LDR	R1,=0xFFFFF44C\n\t"\
		"LDR	R2, [R1]\n\t"\
		"POP	{R1,R2}\n\t"\
	     );\
}

#endif /*DRIVER_PIO_H*/ 
