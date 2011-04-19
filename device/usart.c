/*_____________________________________________________________________________________________ 
Copyright 2011 Rahul Bedarkar

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
_____________________________________________________________________________________________ */ 
/*########################################################################################
CCOMSAT  Project OnBoard Computer
USART Driver AT91SAM7x 
Author:- Rahul S.  Bedarkar

Revised: March 18th 2010 (Nishchay Mhatre)
##########################################################################################*/


#include<stdint.h>
#include"AT91SAM7X256.h"
#include"SST_config.h" 
#include"usart.h" 

__attribute__((section(".text.fastcode"))) void usart_init_normal_async (AT91S_USART *pUSART) 
{                   

 uint32_t US_PINS,US_ID;

 if(pUSART == AT91C_BASE_US0)
 	{
	 US_PINS=0x1F;
	 US_ID = 6;
	}
 else 
 	{
         US_PINS =0x3E0;
         US_ID =7;
      	}

 AT91C_BASE_PIOA->PIO_PDR |= US_PINS;		/*First 10 pins PA0-PA9 disabled from PIO as these are for USART*/
 AT91C_BASE_PIOA->PIO_ASR |= US_PINS;		/*Assigned to peripheral control*/

  pUSART->US_CR = (AT91C_US_RSTRX |          /* Reset Receiver      */

                  AT91C_US_RSTTX |          /* Reset Transmitter   */

                  AT91C_US_RXDIS |          /* Receiver Disable    */

                  AT91C_US_TXDIS           /* Transmitter Disable */
		 );
		

  pUSART->US_MR = ((0<<8) 		 |  /* Asynchronous Mode */
		  
		  AT91C_US_CHMODE_NORMAL |  /*Normal channel mode*/

		  AT91C_US_OVER		 |  /* 8x oversampling */	  
		  
		  AT91C_US_USMODE_NORMAL |  /* Normal mode */
		  
		  AT91C_US_CLKS_CLOCK    |  /* Clock = MCK */

                  AT91C_US_CHRL_8_BITS   |  /* 8-bit Data  */

                  AT91C_US_PAR_NONE      |  /* No Parity   */

                  AT91C_US_NBSTOP_1_BIT   /* 1 Stop Bit  */

		 );

  pUSART->US_BRGR = BRD;                    // Baud Rate Divisor = 0x270 = d624 = (MCK)/(8x9600) 
  					    // as 9600 baud is needed

 pUSART->US_IER = 0x0;			   // No interrupts enabled yet

 AT91C_BASE_PMC->PMC_PCER = (1<<US_ID);		/*Start USART clock*/	
 return;
}


__attribute__((section(".text.fastcode"))) void usart_enable(AT91S_USART *pUSART)
{
 pUSART->US_CR = (0<<5)|(0<<7);		/*RSDIS and TXDIS cleared*/
 pUSART->US_CR = AT91C_US_RXEN | AT91C_US_TXEN ;	/*Enable transmitter and receiver*/
 //AT91C_BASE_PMC->PMC_PCER = (1<<US_ID);		/*Start USART clock*/	
 return;
}


__attribute__((section(".text.fastcode"))) void  usart_putc(AT91S_USART *pUSART,uint8_t ch) 

{

	while (!(pUSART->US_CSR & AT91C_US_TXRDY))
		;   /* Wait for Empty Tx Buffer */
	
	(pUSART->US_THR = ch);                 /* Transmit Character */
	return;
}	


uint8_t usart_kbhit(AT91S_USART *pUSART) /* returns true if character in receive buffer */

{

	if ( pUSART->US_CSR & AT91C_US_RXRDY) {

		return 1;

	}

	else {

		return 0;

	}

}



__attribute__((section(".text.fastcode"))) uint8_t usart_getc (AT91S_USART *pUSART)  /* Read Character from Serial Port */

{    

  while (!(pUSART->US_CSR & AT91C_US_RXRDY))	
  	;   /* Wait for Full Rx Buffer */

  return (pUSART->US_RHR);                      /* Read Character */

}

									
