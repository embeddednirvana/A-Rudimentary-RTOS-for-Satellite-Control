/*_____________________________________________________________________________________________ 

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

_____________________________________________________________________________________________ */ 

/***********************************************************
 *	******		******		******		   *
 *	*    *		*    *		*    *		   *
 *	*    *		*    *          *    *  	   *
 *	******		*    *          ******             *
 *	*    *   ***	*    *	 ***	*    *		   *
 *	*    *   ***    *    *   ***    *    *             *
 *	******   ***    ******   ***    ******             *
 *    							   *  	
 *    CoEP CUBE SATELLITE PROJECT PHERIPHERAL DRIVERS.     *
 *						           *
 * Date : 14.11.09					   *
 * Revissed : 27.12.09	                                   *
 * 							   *
 * Desc : General PIO Driver.  				   *
 * Author : Shravan Aras.				   *	
 *							   *
 **********************************************************/

#include <stdint.h>
#include "AT91SAM7X256.h"
#include "driver_pio.h"

/*The very first function called before any of the pio features are put to use. */
void pio_init(){
	/*Configure the pins required by the external peripheral devices. */
	PIOA->PIO_PDR = RESERVEDA;
	PIOA->PIO_ASR = PA_PORTA;
	PIOA->PIO_BSR = PB_PORTA;
	PIOB->PIO_PDR = RESERVEDB;
	PIOB->PIO_ASR = PA_PORTB;
	PIOB->PIO_BSR = PB_PORTB;
	//PIOB->PIO_BSR = PB_PORTB;
	/*Set port A for input. Keep input glitch filter on and pull-up registers also on. */
	PIOA->PIO_ODR = 0xFFFFFFFF;
	PIOA->PIO_PPUER = 0xFFFFFFFF;
	PIOA->PIO_IFER = 0xFFFFFFFF;	
	/*Set port B for output.Initialy the outputs on all the pins will be 0v. */
	PIOB->PIO_OER = 0xFFFFFFFF;
	PIOB->PIO_OWER = 0xFFFFFFFF;
	PIOB->PIO_ODSR = 0x00000000;
}

/*Function to output onto a port. 
 * logic : Anything other than 0 is treated true.*/
uint32_t pio_output(PORT *port,uint32_t logic){
	if(logic)PIOB->PIO_ODSR |= port->pins;
	else PIOB->PIO_ODSR &= ~port->pins;
	return 0; 										/*The port output configuration is successful. */
}

/*Function to check for input on the port. */
uint32_t pio_input(PORT *port){
	if((~PIOA->PIO_PDSR) & port->pins == port->pins)return 1;
	return 0;										/*No input was triggered at this end. */
}

/* Added new to version 1.1
 * Function to return the status of all the input lines.*/
uint32_t pio_getVal(PORT *port){
	return PIOA->PIO_PDSR;	
}
