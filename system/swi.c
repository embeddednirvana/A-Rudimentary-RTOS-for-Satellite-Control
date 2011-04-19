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
typedef void(*Handler)(void);
/*############################  Define all the swi functions #######################################*/
extern void vPortYieldProcessor(void);

void Return_Case(void)__attribute__((interrupt("SWI"),naked));
void Alloc_Fault(void)__attribute__((interrupt("SWI"),naked));
void Terminate(void)__attribute__((interrupt("SWI"),naked));

void Return_Case(void)
{
 C_SOFT_RESET();		// Reset the controller if the main loop crashes.
 return;
}

void Alloc_Fault(void)		// Not really needed now, only for old RTOS
				//where dynamic memory was being used
{
 while(1)
 	;
}
void Terminate(void)
{
 while(1)
 	;
}
 
 void vPortYieldProcessor(void)			// Note 1
 {
  return;
 }
/*############################### Define the SWI_TABLE  ############################################*/

Handler SWI_TABLE[]=
{
 /*0*/vPortYieldProcessor,
 /*1*/Return_Case,
 /*2*/Alloc_Fault,
 /*3*/Terminate	//For now this does the same thing as terminate analysis
};

/*############################### Generic SWI Handler ############################################*/
__attribute__ ((section(".text.fastcode"))) void C_SWI_HANDLER (uint32_t number) 
{
 SWI_TABLE[number]();
 return;
}

