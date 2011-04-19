
/*##################################################################################### 
Copyright 2011 Rahul Bedarkar .

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
/*********************************
Coep Sat File system File
OC Team				|				
File Type: File System		|
------------------------------	|
Author:- Rahul Bedarkar	
Date :-	 21-12-2009(Compiled)	        |
				|
*********************************/

/* Imp Modifications (Shravan Aras) :
 * 20.10.10 - incorrect use of SD card API : fixed.
 * 05-12-10 - fs_mount_chip(SD sd) has been changed to fs_mount_chip() because the sd variable has now been declared global.
 */

/* File system refined and optimized, useless APIS are removed (Rahul B.) : 10-12-10 */

/* Corrected Persistant storing and loading functions of FAT And MAT : Rahul B. 13-12-10*/

//#include<string.h>
#include"sat_fs.h"
#include<stdint.h>
#include"AT91SAM7X256.h"
#include "sd.h"
SD sd;

uint8_t ready; // of SD card
uint8_t buffer[message_size];// shared buffer of data to used for read of write
uint32_t add_temp = 0;				/* Added for testing. */
uint8_t return_set;				/* Added for testing */

uint8_t read_file(uint32_t file_number)  //read the file and returns the pointer to file
{
	message m;
	uint8_t i;	
	
	if(CHECK_FILE_FLAG(file_number))	//
	{	
		for(i=0;i<messages_per_file;i++)
		{
			if(read_message(file_number,i));	
			{
				m=*ptr;
				tempf.sms[i]=m;	//storing the message
			}
			sd_delay(15);
		}		
	tempf.flag=0x11; //file exists
	ptrf=&tempf;
	return 0x00; // read file correctly
	}
	else
	{
		return 0x11;// file doesnot exists
	}	
}

uint8_t write_file(uint32_t file_number,file *f)// writes the whole file
{
	file temp;
	message data,*d;
	uint8_t i;		

	if(CHECK_FILE_FLAG(file_number)) // check file delete flag was der before
	{
		i=0;
		while(i < messages_per_file)	
		{
			temp=*f;
			data=temp.sms[i];
			d=&data;		
			if (!write_message(file_number,d,i))
				break;
			sd_delay(15);
		}
	if(i==messages_per_file)
		return 0x00;//write ok;
	else
		return 0x11;	//partial write	
	}
	else
	{
		return 0x01;//file write problem
	}
}

uint8_t is_file_ready2_delete(uint32_t file_number)// This need to be withdrawn
{
	uint8_t i=0;

	while(i < messages_per_file)
	{
		if(!CHECK_MSG_DELETE(file_number,i))
			break;
	}

	if(i==messages_per_file)
	{
		SET_MSG_DELETE(file_number,i);
		if(delete_file(file_number))
		{
			return 0x00;//file is ready 2 delete & deleted succssfully
		}
		else
			return 0x01;//file is ready but not deleted i dont what is problem
	}	
	else
		return 0x11;//file is not ready 2 delete
}

uint8_t delete_message(uint32_t file_number,uint8_t message_number)//deletes the message
{
	/*if(!CHECK_MSG_EXIST(file_number,message_number))
	{
		return 0x01;//sms doesnot exists	
	}
	else if(CHECK_MSG_DELETE(file_number,message_number))
		{
			return 0x11;//sms already deleted	
		}
	else // its time to delete the sms 
	{
		SET_MSG_DELETE(file_number,message_number);

		//if(is_file_ready2_delete(file_number))//check all messages are deleted or not
		//	return 0x00;//file & message both deleted
		//else
		//	return 0x10;//message deleted but not file
	}*/
SET_MSG_DELETE(file_number,message_number);
return 0;
}

uint8_t read_message(uint32_t file_number,uint8_t message_number)//reads particular message in queue
{
	uint32_t address;
	uint8_t ret_val;// of read_low_level

	if(!CHECK_MSG_DELETE(file_number,message_number))//checking sms to read is not marked as delete
	{
		address=mat[file_number]->address[message_number];//getting address of the sms to read
		add_temp = address;

		ret_val=sd_read_block(&sd,address,temp.data);
		temp.flag=0x11;//message exisits

		if(delete_message(file_number,message_number))//call to delete sms
				;
		else
			return 0xf0; // can not delete sms I dont no what is problem
	
		ptr=&temp;

	return ret_val;// Read sms correctly					
	}
	else
		return 0x0f;//Unable to Read Message
		
}

uint8_t read_message_next(uint32_t file_number)//reads next message in queue
{
	uint8_t next;
	next=mat[file_number]->next;//getting valid sms no to read	
	
	return read_message(file_number,next); // Very easy na!
}

uint8_t write_message(uint32_t file_number,message *m,uint8_t message_number)//writes message at particular position 
{
	uint32_t address;

	ptr=m;
	temp=*ptr;

	if(!CHECK_MSG_EXIST(file_number,message_number))//checking message location is empty
	{
		address=mat[file_number]->address[message_number];
		return sd_write_block(&sd,address,temp.data); // returns 1 0r 0
	}
	else
		return 0xff; // Error msg exists can not overwrite the message
}

uint8_t write_message_next(uint32_t file_number,message *m)//writes sms to next valid position
{
	uint8_t next;

	next=mat[file_number]->next;
	return write_message(file_number,m,next); //very easy na!
}

uint8_t setup_FAT()   //makes default entries in the FAT,fills sector address,file number,name,allcoation flag
{
uint8_t i;

root->address[0]=starting_address;//address of the sector no. 60,000
root->file_number[0]=starting_sector;
CLEAR_FILE_FLAG(0);	

for(i=1;i<files;i++)
{
	root->file_number[i]=root->file_number[i-1]+messages_per_file;
	root->address[i]=root->file_number[i]<<9;
	CLEAR_FILE_FLAG(root->file_number[i]);
}	
return 0x0;
}

uint8_t setup_MAT() //setting up blank MAT just as Format
{
uint8_t i,j;

for(i=0,j=0;i<files;i++)		// maybe we should add j=0 here in intialization step
{
	mat[i]->message_number[0]=i;
	mat[i]->next=0;
		CLEAR_MSG_EXIST(i,j);
		mat[i]->address[0]=root->address[i];			
	for(j=1;j<messages_per_file;j++)
	{
		mat[i]->message_number[j]=i;
		CLEAR_MSG_EXIST(i,j);
		mat[i]->address[j]=(mat[i]->address[j-1])<<9;   //address of next message.
		
	}			
}	
return 0;
}

uint8_t create_file(uint32_t file_number)
{

	if(!CHECK_FILE_FLAG(file_number))
	{
		SET_FILE_FLAG(file_number);
		return 0x00;
	}
	else
		return 0x11; //file already exists!

}

uint8_t delete_file(uint32_t file_number)
{
	if(CHECK_FILE_DELETE_FLAG(file_number))//chekcing file is marked to delete
	{
		CLEAR_FILE_FLAG(file_number);//making file flag ready to create
		return 0;
		
	}
	else	
		return 0x11;
}

uint8_t fs_mount_chip()	// Starts the SD card and 4 lines of code must be here
{

 sd.write_timeout = 1000;
 sd.read_timeout = 1000;
 sd.busy_flag = 0;
 ready=sd_initialize(&sd);

return ready;
}

uint8_t setup_File_System()	//sets up the basic Tables like FAT,MAT
{
	//unsigned int i;
	uint8_t status=0x00;

	/*root=&root_fat;
	setup_FAT();	
	for(i=0;i<files;i++)
		mat[i]= root_mat+i;
	setup_MAT();
	*/

	if(!fs_mount_chip())
		if(check_filesystem())
			status=0xff;

	return_set=setup_fat_mat(); 

	return status; //return 0xff on successfully setting up file system
}

uint8_t format_filesystem()	//Does this need actuallly?
{
	return 1;	//need to write stuff here
}
uint8_t read_write_test()
{
	// needs stuff here SA
	return 0;
}
uint8_t bad_sector_test()
{
	// need to write code,first research!
	return 0;
}
uint8_t fat_mat_check()
{
	return 0;
}
uint8_t check_filesystem()	//Need to write stuff here,decided on 10-12-10.
{
	bad_sector_test();	// Does bad sector testing need to write
	read_write_test();	//Does the read write test of higher address of SD, need to write (SA)
	fat_mat_check(); 	//
	// Do reset controller register's checking for loading FAT MAT from file or init them
	return 0;
}
uint8_t reset_fat_mat() // resets mat and fat only first time
{
	uint8_t i;
	root=&root_fat;
	setup_FAT();	
	for(i=0;i<files;i++)
		mat[i]= root_mat+i;
	setup_MAT();
	return 0;
}
uint8_t store_fat_mat_to_SD()// stores the Fat and mat in SD at higher address. Need to be called from idle task.
{
	/* Size of FAT to be written into SD sector is 40 bytes only.*/

	uint32_t address,temp;
	uint8_t data[message_size];
	uint16_t i,j,shift=0,x;
	uint16_t next=0;
	uint32_t MAT_START;

	MAT_START=SP_ADD_MAT_START; //Starting address sector where mats will be stored contigiously

	/* Following 3 loops stores the FAT into array */

	for(i=0;i<files;i++) // Stores the address of the files of FAT in array to be written
	{
		for(j=0;j<4;j++)
		{
			address=(root->address[i]) >> shift;
			temp=address & 0xff;
			data[next++]=(uint8_t)temp;
			shift +=8;
		}
		shift=0;
	}

	shift=0;

	for(i=0;i<files;i++) // Stores the numbers of the files of FAT in array to be written
	{
		for(j=0;j<4;j++)
		{
			address=(root->file_number[i]) >> shift;
			temp=address & 0xff;
			data[next++]=(uint8_t)temp;
			shift +=8;
		}
		shift=0;
	}
	shift=0;
	for(i=0;i<4;i++) // Stores the flag_table of the files of FAT in array to be written
	{
		for(j=0;j<4;j++)
		{
			address=(root->flag_table[i]) >> shift;
			temp=address & 0xff;
			data[next++]=(uint8_t)temp;
			shift +=8;
		}
		shift=0;
	}
	
	for(i=next;i<message_size;i++)	// zero paddding
		data[i]=0;

	/* Sector is ready to be written into SD Card At Special address*/

	 address = (SP_ADD_FAT1 << 9 );	 
	 sd_write_block(&sd,address,data); // FAT1 is written
	 sd_delay(15); // SD card is taking rest wait

	 address = (SP_ADD_FAT2 << 9 );	 	
	 sd_write_block(&sd,address,data); // FAT2 is written
	 sd_delay(15); // SD card is taking rest wait
	
	 address = (SP_ADD_FAT3 << 9 );	 
	 sd_write_block(&sd,address,data); // FAT3 is written
	 sd_delay(15); // SD card is taking rest wait

	/* Its time to write MATS into SD Card */
	
	shift=0;
	next=0;
	for(x=0;x<files;x++)
	{
		for(i=0;i<4;i++)
		{
			address= (mat[x]->file_number) >> shift;
			temp= address & 0xff;
			data[next++]= (uint8_t)temp;
			shift += 8;
		} // File Number is written
		shift=0;
		
		data[next++]=(mat[x]->next); // next ptr is done

		for(i=0;i<messages_per_file;i++) // message number is written
		{
			data[next++]= mat[x]->message_number[i];
		}
		
		for(i=0;i<8;i++) // Stores exists flad table
		{	
			for(j=0;j<4;j++)
			{
				address=(mat[x]->exists_flag_table[i]) >> shift;
				temp=address & 0xff;
				data[next++]=(uint8_t)temp;
				shift +=8;
			}
			shift=0;
		}

		shift=0;
		
		for(i=0;i<8;i++) // Stores delete flag table
		{	
			for(j=0;j<4;j++)
			{
				address=(mat[x]->delete_flag_table[i]) >> shift;
				temp=address & 0xff;
				data[next++]=(uint8_t)temp;
				shift +=8;
			}
			shift=0;
		}
		
		shift=0;

		for(i=0;i<messages_per_file;i++) // Stores address of messages
		{	
			for(j=0;j<4;j++)
			{
				address=(mat[x]->address[i]) >> shift;
				temp=address & 0xff;
				data[next++]=(uint8_t)temp;
				shift +=8;
			}
			shift=0;
		}

		for(i=next;i<message_size;i++)
			data[i]=0x0;

		/* MAT for this itration's file is ready to written into SD */ 
		
		address= MAT_START << 9;
		sd_write_block(&sd,address,data);
		sd_delay(15);// Sd is taking rest
		MAT_START++;
		shift=0;
		next=0;
		
	}
	
	return 0;
}		
uint8_t load_fat_mat_from_SD()// Loads the Fat and mat from SD in case of reset
{

	uint32_t address,temp;
	uint8_t data[message_size];
	uint16_t i,j,shift=0,x;
	uint16_t next=0;
	uint32_t MAT_START=SP_ADD_MAT_START;

	/* Read FAT From any special address */

	address=(SP_ADD_FAT1 << 9);
	sd_read_block(&sd,address,data); // read FAT from SD
	sd_delay(15); // needs to be withdrawn
	/* Take FAT into <main> memory */
	
	address=0;
	for(i=0;i<files;i++) /* loads Addresses of file into  FAT */
	{
		for(j=0;j<4;j++)
		{
			temp= data[next++] << shift;
			address= address | temp;
			shift += 8;
		}
		root->address[i]= address;
		shift= 0;
	}

	address=0;
	shift=0;
	for(i=0;i<files;i++) /* loads no of files into  FAT */
	{
		for(j=0;j<4;j++)
		{
			temp= data[next++] << shift;
			address= address | temp;
			shift += 8;
		}
		root->file_number[i]= address;
		shift= 0;
	}

	address=0;
	shift=0;
	for(i=0;i<4;i++) /* loads flag_tables of file into  FAT */
	{
		for(j=0;j<4;j++)
		{
			temp= data[next++] << shift;
			address= address | temp;
			shift += 8;
		}
		root->flag_table[i]= address;
		shift= 0;
	}

	/* Fat is loaded into memory but not MAT let us do it */

	
	shift=next=0; // (optimized);

	for(x=0;x<files;x++)
	{
		address=MAT_START << 9;
		sd_read_block(&sd,address,data); // For each file read mat

		address=0;
		shift=0;
		for(i=0;i<4;i++) // loads file number
		{
			temp= data[next++] << shift;
			address= address | temp;
			shift += 8;
		}
		mat[x]->file_number=address; // loads file number

		mat[x]->next= data[next++];	// loads next ptr;

		for(i=0;i<messages_per_file;i++) // loads message number
			mat[x]->message_number[i]= data[next++];
		
		shift=0;
		address=0;
		for(i=0;i<8;i++) /* loads exists flag  */
		{
			for(j=0;j<4;j++)
			{
				temp= data[next++] << shift;
				address= address | temp;
				shift += 8;
			}
			mat[x]->exists_flag_table[i]= address;
			shift= 0;
		}

		shift=0;
		address=0;
		for(i=0;i<8;i++) /* loads delete flag  */
		{
			for(j=0;j<4;j++)
			{
				temp= data[next++] << shift;
				address= address | temp;
				shift += 8;
			}
			mat[x]->delete_flag_table[i]= address;
			shift= 0;
		}

		shift=0;
		address=0;
		for(i=0;i<messages_per_file;i++) /* loads Addresses messages*/
		{
			for(j=0;j<4;j++)
			{
				temp= data[next++] << shift;
				address= address | temp;
				shift += 8;
			}
			mat[x]->address[i]= address;
			shift= 0;
		}
		next=0;
	}


	return 0;
}
uint8_t setup_fat_mat() // decides wheather to load from sd card or init to zero
{
	uint32_t reset_type;
	reset_type=AT91C_BASE_RSTC->RSTC_RSR & (0x00000700);

	if(reset_type == AT91C_RSTC_RSTTYP_WATCHDOG || reset_type == AT91C_RSTC_RSTTYP_SOFTWARE || reset_type == AT91C_RSTC_RSTTYP_BROWNOUT )
	{
		load_fat_mat_from_SD();
		return 0x04;
	}
	else
	{
		reset_fat_mat();
		return 0x05;
	}
	return 0;
}
uint8_t do_soft_reset()
{
	AT91C_BASE_RSTC->RSTC_RCR |= (0xA5 << 24) | ( 1 << 0 );
	return 0;
}
