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
/*******************************************
coep sat file system file
File Type: - Headerfile			*
------------------------------------	*	
Author : - Rahul Bedarkar		*	
Date:-20-12-2009(Compiled)		*
					*
Notes:
1. uint32_t flag_table[4]: 
Array of four 32 bit words for checking the file allocation. Kept as a bit map. Convention:
File flag number	Word
31-0 			0
63-32			1
95-64			2
127-96			3
*
**********************************************/



#ifndef _SAT_FS_H
#define _SAT_FS_H

#ifdef _SAT_FS_H
//----------------#defines here
#include<stdio.h>
#include<stdint.h>

#define files 1/3
#define file_size 32768  //
#define messages_per_file 2  //512//64
#define message_size 512	//512
#define starting_sector 60000
#define starting_address 0x1D4C000			/* For 60,000 th sector. */
#define eom '~'
#define SP_ADD_FAT1 59000
#define SP_ADD_FAT2 80000
#define SP_ADD_FAT3 90000
#define SP_ADD_MAT_START 90001


//--------------------typedefs here
//----allocation checking macros. Note that the file allocation checking macro is useful only for the root and the 
//message allocation checking macro for the mat entry of each file---------------------------------------------



#define CLEAR_FILE_FLAG(flag_number)	root->flag_table[flag_number] = root->flag_table[(flag_number>>5)] & (0<<flag_number)
#define CLEAR_MSG_EXIST(file_number,flag_number)	mat[file_number]->exists_flag_table[flag_number] = mat[file_number]->exists_flag_table[(flag_number>>5)] & (0<<flag_number)
#define CLEAR_MSG_DELETE(file_number,flag_number)	mat[file_number]->delete_flag_table[flag_number] = mat[file_number]->delete_flag_table[(flag_number>>5)] & (0<<flag_number)
#define CLEAR_FILE_DELETE_FLAG(flag_number)	root->flag_table[flag_table] = root->flag_table[(flag_number>>5)] & (0<<flag_number)

#define SET_FILE_FLAG(flag_number)	root->flag_table[flag_number] = root->flag_table[(flag_number>>5)] | (1<<flag_number)
#define SET_MSG_EXIST(file_number,flag_number)	mat[file_number]->exists_flag_table[flag_number] = mat[file_number]->exists_flag_table[(flag_number>>5)] | (1<<flag_number)
#define SET_MSG_DELETE(file_number,flag_number)	mat[file_number]->delete_flag_table[flag_number] = mat[file_number]->delete_flag_table[(flag_number>>5)] | (1<<flag_number)
#define SET_FILE_DELETE_FLAG(flag_number)	root->flag_table[flag_number] = root->flag_table[(flag_number>>5)] | (1<<flag_number)

#define CHECK_FILE_DELETE_FLAG(flag_number)	root->flag_table[(flag_number>>5)] & (1<<flag_number)
#define CHECK_FILE_FLAG(flag_number)	root->flag_table[(flag_number>>5)] & (1<<flag_number)
#define CHECK_MSG_EXIST(file_number,flag_number)	mat[file_number]->exists_flag_table[(flag_number>>5)] & (1<<flag_number)
#define CHECK_MSG_DELETE(file_number,flag_number)	mat[file_number]->delete_flag_table[(flag_number>>5)] & (1<<flag_number)
//--------------------------------------------------------------------------------------------------------------
typedef  struct _sat_message{   //file system message struct here
	uint8_t data[message_size];      //message data will be here//Done 8_t before 16_t
	uint8_t flag;                 //message delete flag here
}message;

typedef  struct _sat_file{     //file system file struct here
	message sms[messages_per_file];        //message array 
	uint8_t flag;// file deleted flag here
}file;

 
typedef struct _sat_MAT{ //Message allocation table
	uint32_t file_number; 
	uint8_t message_number[messages_per_file];
	uint32_t exists_flag_table[8];
	uint32_t delete_flag_table[8];
	uint32_t address[messages_per_file];
	uint8_t next;
}MAT;

typedef  struct _sat_FAT{  //file allocation table for file system
	//file f[files];  // file structure to be written
	uint32_t address[files]; // sector address of file
	//char name[10][files];  //file name upto 10 bytes
	uint32_t file_number[files];// valid numbers 13-113
	uint32_t flag_table[4];//allocated or not : Note 1
        //MAT *m[files];
}FAT;

//----------------------FS API's here-------
message temp,*ptr;//buffer and pointer to buffer
file tempf,*ptrf;//buffer and pointer to buffer
 FAT *root;  // global pointer
 MAT *mat[files]; //global pointer array
 FAT root_fat;
 MAT root_mat[files];
//----------------------------------------------------------------------------
uint8_t create_file(uint32_t file_number);//makes entry in the FAT*

uint8_t delete_file(uint32_t file_number); // deletes entry from FAT*

uint8_t read_file(uint32_t file_number);  //read the file and returns the pointer to file*

uint8_t write_file(uint32_t file_number,file *f);// writes the whole file*

uint8_t read_message_next(uint32_t file_number);//reads next message in queue*

uint8_t write_message_next(uint32_t file_number,message *m);//writes next message* 

uint8_t is_file_ready2_delete(uint32_t file_number);//checks if is reday to delete*

uint8_t read_message(uint32_t file_number,uint8_t message_number);//reads message in queue*

uint8_t write_message(uint32_t file_number,message *m,uint8_t message_number);//writes message*

uint8_t delete_message(uint32_t file_number,uint8_t message_number);//deletes message*

uint8_t setup_File_System();//*sets up blank file system

uint8_t setup_FAT();//set up the FAT*

uint8_t setup_MAT();// set up the MAT*

uint8_t format_filesystem();//used to format whole file system

uint8_t check_filesystem();

uint8_t fs_mount_chip();

uint8_t setup_fat_mat();

uint8_t store_fat_mat_to_SD();

uint8_t load_fat_mat_from_SD();

uint8_t do_soft_reset();
//-------------------------------------------
#endif

#endif

