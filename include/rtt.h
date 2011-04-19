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
#include<stdint.h>
#define RTT_START_FAILED 5

struct _rttMode
{
uint16_t rt_prescaler;
uint8_t incr_intr_enable;
uint8_t alm_intr_enable;
uint32_t alarm;
};

/*Real Time Timer*/
uint32_t sys_get_time();

uint32_t sys_rtt_mode_set(struct _rttMode *mode);

uint32_t sys_rtt_alm_set(uint32_t alarm);

uint32_t sys_rtt_int_disable(uint8_t intr);

uint32_t sys_rtt_start();

uint32_t sys_rtt_status_chk(uint8_t status_of);
