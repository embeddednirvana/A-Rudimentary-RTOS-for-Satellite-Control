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
/*###################################################################################

Data Logging File for Testing RTOS Performance: For DBG Purpose only

##################################################################################### */
#ifndef TRACE_H
#define TRACE_H

typedef struct _log
{
 uint32_t where;		// Numerical code for a job
 uint8_t  what;			
 uint32_t when;			// Timestamp
 
}log_record;			// 9 bytes of integer data


#define LOGSIZE 1024		// 9 kB large file

/* What each bit in what signifies*/ 
#define START		(1<<0)		// This is the start of the TASK/ISR
#define END		(0<<0)		// End

#define ISR		(1<<1)		// This procedure is an ISR
#define NOT_ISR		(0<<1)	


/*Defining codes*/
#define COMM	0x1 
#define MPPT	0x2 
#define BEAC	0x3 
#define DACQ	0x4
#define IDLE	0x5
#define TICK	0x6
#define ECC	0x7
#define MAIN	0x8
#define CRASH	0x9

/* Da Main Man */ 
#define update_log(whr,wht)\
{\
  if(current == LOGSIZE)	/*Stop when file is full*/ \
	asm volatile("SWI 0x3");\
  data_log[current].where = whr;\
  data_log[current].what = wht;\
  data_log[current].when = sys_get_time();\
  current++;\
}\

#endif /* TRACE_H */ 
