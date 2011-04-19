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

/*#################################################################################################
CCOMSAT Project OnBoard Computer
Low Level Hardware Initialization Routine

31st December 2009 (first write)
10 Jan 2010 (rev 1)
Changes:
1. Put in the FIQ haw initialization.
2. FIQ mode registers are initialized with the values needed in the handler so as to speed up the handling.

26 Feb 2010 (rev 2)
Changes:
1. Apparently the inlining of the code this way is wrong (messes up GCC's register allotment) and this file had not 
been compiled before. A great blunder.So now the inlining has been commented out till an alternative is found.

1 March 2010 
Bug fix: The pointer to the AIC registers was uninitialized therefore the 'segfault' causing the ABT.

Coded by:Nishchay Mhatre

Notes:
This code initializes the clock sources of the microcontroller. Called after the vector setup and 
memory remap is done. The fastcode, data, stack section and the BSS section have not been initialized 
in RAM yet. Hence this acts as a leaf function and makes no other function calls, not even to the
hardware drivers, avoiding the call overhead. Note that interrupts have not been yet enabled as of 
yet.
The PLL clock is used as the source, keeping Main Clock at 18 MHz and master clock 48MHz. Flash cycles
set at 32MHz.

##################################################################################################*/

#include "AT91SAM7X256.h"
#define MAINCK	18432000
#define MCK1	47923200
#define MCK2	23961600
#define MCK3	11980800
#define MCKmin	10000000
void low_level_hw_init()	/*M-05*/ 
{
    AT91PS_PMC pPMC;
    AT91PS_PIO pPIOA;
    AT91PS_AIC pAIC;
   
   /*Initializing the hardware register pointers*/
   pPIOA = AT91C_BASE_PIOA;
   pPMC = AT91C_BASE_PMC;
   pAIC = AT91C_BASE_AIC;

   /* Set flash wait sate FWS and FMCN One wait state and 32 MHz flash speed at 48 MHz */
    AT91C_BASE_MC->MC_FMR = ((AT91C_MC_FMCN) & ((MCK1 + 500000)/1000000 << 16))| AT91C_MC_FWS_1FWS;

    AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;  /* Disable the watchdog */

    /* Enable the Main Oscillator:
    * set OSCOUNT to 6, which gives Start up time = 8 * 6 / SCK = 1.4ms
    * (SCK = 32768Hz)
    */
    pPMC->PMC_MOR = ((6 << 8) & AT91C_CKGR_OSCOUNT) | AT91C_CKGR_MOSCEN;
    while ((pPMC->PMC_SR & AT91C_PMC_MOSCS) == 0) 
    {;/* Wait the startup time */
    }

    /* Set the PLL and Divider:
    * - div by 5 Fin = 3,6864 =(18,432 / 5)
    * - Mul 25+1: Fout = 95,8464 =(3,6864 *26)
    * for 96 MHz the error is 0.16%
    * Field out NOT USED = 0
    * PLLCOUNT pll startup time estimate at : 0.844 ms
    * PLLCOUNT 28 = 0.000844 /(1/32768)
    */
    pPMC->PMC_PLLR = ((AT91C_CKGR_DIV & 0x06)
                      | (AT91C_CKGR_PLLCOUNT & (28 << 8))
                      | (AT91C_CKGR_MUL & (6 << 16)));
    while ((pPMC->PMC_SR & AT91C_PMC_LOCK) == 0) 
    	; /* Wait the startup time */
    	
    while ((pPMC->PMC_SR & AT91C_PMC_MCKRDY) == 0) 
    	;

    /* Select Master Clock and CPU Clock select the PLL clock / 2 */
    pPMC->PMC_MCKR =  AT91C_PMC_PRES_CLK_2;
    while ((pPMC->PMC_SR & AT91C_PMC_MCKRDY) == 0) 
    	;

    pPMC->PMC_MCKR |= AT91C_PMC_CSS_PLL_CLK;
    while ((pPMC->PMC_SR & AT91C_PMC_MCKRDY) == 0)
    	;
   
   pPIOA->PIO_PER = (1<<29);
   pPIOA->PIO_PPUDR = (1<<29);
   pPIOA->PIO_PDR = (1<<29);		/*Relinquish PIO control of that pin*/
   pPIOA->PIO_ASR = (1<<29);		/*Assign the pin PA29 to the AIC function that is receiving the FIQ signal*/

    pAIC->AIC_SMR[AT91C_ID_FIQ] = AT91C_AIC_SRCTYPE_EXT_LOW_LEVEL;		/*FIQ signal Configured as 
    											external high level */
    pAIC->AIC_IECR = (1 << AT91C_ID_FIQ);		/*FIQ signal enabled*/
							/*Note that FIQ cannot be handled till F bit is cleared at the Processor level*/
  
  // asm volatile 
  // ( "MSR	CPSR_c,(FIQ_Mode|I_Bit|F_Bit)	\n\t"		/*Initializing values in the FIQ regs for the handler*/
  //   "LDR	r8,#0xFFFFF43C		\n\t"			/* in advance for faster handling of the FIQ*/
  //   "LDR	r11,#0xFFFFF638		\n\t"
  // );
  
  return;

}

