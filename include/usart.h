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
#ifndef USART_H
#define USART_H

#include"AT91SAM7X256.h"
#include<stdint.h> 
					
// USART H/W settings

#define MCK 	47923200		// Main clock
#define BR 	9600			// Desired Baud Rate
#define BRD 	(MCK/(8*BR))		// Baud Rate Divisor as defined by SAM7 datasheet

#define MSIG 	config_PIO_INTR_PIN	
#define ACK 	0x16			
#define BSIG 	config_PIO_INTR_PIN 

#define EOM	0x9E


void usart_init_normal_async (AT91S_USART *pUSART) ;
void usart_enable(AT91S_USART *pUSART);
uint8_t usart_getc (AT91S_USART *pUSART);  /* Read Character from Serial Port */
void  usart_putc(AT91S_USART *pUSART,uint8_t ch) ;

#endif /*USART_H*/ 
