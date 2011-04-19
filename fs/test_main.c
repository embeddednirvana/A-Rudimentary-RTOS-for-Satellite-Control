
/*##################################################################################### 
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
##################################################################################### */ 
/*########################################################################

File system configuration file
PIO controller config with external flash
--------------------------------------------------
Author 	: Rahul Bedarkar
Date	: 20-12-2009 (Compiled)

###########################################################################*/

#include"AT91SAM7X256.h"
#include <stdint.h>
#include "driver.h"
#include "sd.h"
#include "spi.h"
#include "sat_fs.h"
#include<stdint.h>

uint8_t start_sat_FS()
{
	return(setup_File_System());
}

uint8_t data[messages_per_file][message_size];
//uint8_t data[message_size];
uint8_t  return_val;
extern SD sd;
int main()
{
	int i=0,j=0;
	uint32_t address;
	/* Initialization sub-routines go here. */
	pio_init();	//Needed by all peri so donot move it
	spi_initialize(); // same as above		

	//Following 4 lines need to be moved to fs_mount_chip() in sat_fs.c
	
	/*sdc.write_timeout = 1000;
        sdc.read_timeout = 1000;
        sdc.busy_flag = 0;
	sd_initialize(&sdc);		*/

	file f;
	f.flag=0x11;

	for(i=0;i<messages_per_file;i++)
	{
		for(j=0;j<message_size;j++)
		{
			f.sms[i].data[j]=0x17;
		}
		//f.sms[i].data[j]=eom;
	}

	//for(i=0;i<message_size;i++)data[i] = 0xAA;

	start_sat_FS();
	create_file(0);
	//create_file(1);
	//create_file(2);
	//SET_MSG_EXIST(0,0);
	
	for(i=0;i<messages_per_file;i++)
		SET_MSG_EXIST(0,i);
	/*for(i=0;i<messages_per_file;i++)
		SET_MSG_EXIST(1,i);
	for(i=0;i<messages_per_file;i++)
		SET_MSG_EXIST(2,i);
	*/

	
	return_val=write_file(0,&f);
	sd_delay(15);
	//return_val=read_file(0);
	
	/*
	return_val=write_message(0,&msg,0);	
	sd_delay(15);// need rest of 15 for SD/SPI i dont know
	return_val=read_message(0,0);
	
	for(i=0;i<message_size;i++)
	data[i]=ptr->data[i];
	*/
	
	/*for(i=0;i<messages_per_file;i++)
	{
		for(j=0;j<message_size;j++)
		{
			data[i][j]=ptrf->sms[i].data[j];
		}

	}*/
	delete_file(0);

	//store_fat_mat_to_SD();
	/*address=(SP_ADD_FAT1 << 9);
	sd_read_block(&sd,address,data); // read FAT from SD
	sd_delay(15); // needs to be withdrawn
	*/
	//do_soft_reset();

	return_val=read_file(0);
	
	for(i=0;i<messages_per_file;i++)
	{
		for(j=0;j<message_size;j++)
		{
			data[i][j]=ptrf->sms[i].data[j];
		}

	}
		
	return 0;
}
