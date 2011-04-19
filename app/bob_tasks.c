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
_____________________________________________________________________________________________ */ 
#include"bob_tasks.h" 
#include<stdint.h> 
#include"trace.h"

extern log_record data_log[LOGSIZE];
extern uint32_t current;

void one(void*p)
{
 make_task_nestable();
 int dacq;
 dacq = 1337;
 update_log(DACQ,(START|NOT_ISR));
 restore_task_status();
 update_log(DACQ,(END|NOT_ISR));
 return;
}

void two(void*p)
{
 make_task_nestable();
 int comm_ctr;
 comm_ctr = 2412;
 update_log(COMM,(START|NOT_ISR));
 restore_task_status();
 update_log(COMM,(END|NOT_ISR));
 return;
}

void three(void*p)
{
 make_task_nestable();
 restore_task_status();
 return;
}

void tick_hook()
{
 make_task_nestable();
 restore_task_status();
 static unsigned int ticks;
 ticks ++;
}

void idle_hook()
{
 static unsigned int idling;
 idling++;
}
