/*_____________________________________________________________________________________________ 
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

_____________________________________________________________________________________________ */ 


/*#################################################################################################
Real time timer device driver functions:
Nishchay Mhatre 
13/11/09 (first write)
23/12/09 (Rev1)

Changes:

Notes:
1.Returns the real time.

2.Sets the parameters which the rtt will follow while running. This includes enabling and disabling
interrupts on counts and alarms and on incrementing it. Also supplied to this is the prescaler value
on which the time of counting up depends.parameters set by the calling function and structure pointer
passed

3.Sets the alarm value.

4.Disable interrupt. Passed parameter is integer 1 for alarm and integer 2 increment interrupt.

5.Start the timer. Then check value to see if it is still zero and defer error to calling task if rtt 
has started.

6.Check the rtt status. The status register is cleared on a read therfore both bits are stored and 
whichver is needed is used.

7.With respect to errata sheet (page 644, product datasheet, the s/w must not poll the status
registers for the Alarm and RTCINC events. Since a read of the SR at the time these bits are set 
will cause event loss.
#################################################################################################*/
#include<stdint.h>
#include"AT91SAM7X256.h"
#include"rtt.h"
/*struct _rttMode
{
uint16_t rt_prescaler;
uint8_t incr_intr_enable;
uint8_t alm_intr_enable;
uint32_t alarm;
};*/
/* 1 */
uint32_t sys_get_time()
{
 uint32_t value = AT91C_BASE_RTTC->RTTC_RTVR;
 value = AT91C_BASE_RTTC->RTTC_RTVR;
 return value;
}

/* All the following are deprecated as of Dec 2010*/


/* 2 */
/*
uint32_t sys_rtt_mode_set(struct _rttMode *mode)
{
 	AT91C_BASE_RTTC->RTTC_RTMR = ((AT91C_RTTC_RTTINCIEN & ( (mode->incr_intr_enable)<<17) )|(mode->rt_prescaler<<0)|(AT91C_RTTC_ALMIEN & ( (mode->alm_intr_enable)<<16) ) );
 
 if(mode->alm_intr_enable==1)
 	{ 
	 AT91C_BASE_RTTC->RTTC_RTAR = mode->alarm;
	}
 
 
 return 0;
}
*/

/* 3 */
/*
uint32_t sys_rtt_alm_set(uint32_t alarm)
{
 AT91C_BASE_RTTC->RTTC_RTAR = alarm;
 return 0;
}
*/
/* 4 */
/*
uint32_t sys_rtt_int_disable(uint8_t intr)
{
 if (intr==1)
 	AT91C_BASE_RTTC->RTTC_RTMR |= (AT91C_RTTC_ALMIEN & (0<<16));
 
 else if (intr==2)
 	AT91C_BASE_RTTC->RTTC_RTMR |= (AT91C_RTTC_RTTINCIEN & (0<<17));

 return;
}
*/
/* 5 */
/*
uint32_t sys_rtt_start()
{
 AT91C_BASE_RTTC->RTTC_RTMR = AT91C_RTTC_RTTRST;
 if(sys_get_time()!=0)
 	return 0;
 else 
 	return RTT_START_FAILED;

 return;
}
*/
/* 6 */
/*
uint32_t sys_rtt_status_chk(uint8_t status_of)
{
 static uint8_t rtt_status ;
 rtt_status = AT91C_BASE_RTTC->RTTC_RTSR & 0x3;
 if (status_of == 1)
 	return(rtt_status & 0x1);
 else if (status_of ==2)
 	return(rtt_status & 0x2);

 return;
}*/
