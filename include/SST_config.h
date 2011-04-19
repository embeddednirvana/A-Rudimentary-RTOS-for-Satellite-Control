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
COEP Satellite Project OnBoard Computer

Configuration File for Single Stack RTOS

Author: Nishchay Mhatre 

Rev: 11th October 2010 (first write)

Notes:
Current configurable components are:
1. Number of tasks
2. Use of Idle Hook
3. Use of Tick Hook

##################################################################################### */ 

#ifndef SST_CONFIG__H
#define SST_CONFIG__H

#define NTASKS	3	// Number of tasks
#define configUSE_IDLE_HOOK	1	// Use idle hook function
#define configUSE_TICK_HOOK	1	// Use tick hook function
#define TASK_PERIOD_1		100	// Number of ticks at which a periodic
					// task should be called. One macro for
					// each such task.

#define configCPU_CLOCK_HZ          ( ( uint32_t ) 47923200 )
#define configTICK_RATE_HZ          ( ( portTickType ) 10 )

#define config_IRQ_TYPE AT91C_AIC_SRCTYPE_EXT_LOW_LEVEL		// Type of Signal for
								// externall interrupts

#define config_PIO_INTR_PIN	12

#endif /* SST_CONFIG__H */
