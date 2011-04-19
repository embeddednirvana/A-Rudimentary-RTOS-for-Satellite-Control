/*********************************
Coep Sat File system File
OC Team				|				
File Type: File System		|
------------------------------	|
Author:- Rahul Bedarkar	
Date :-	 21-12-2009(Compiled)	        |
				|
*********************************/

#include<malloc.h>
#include<string.h>
#include"sat_fs.h"
#include<stdint.h>
#include "sd.h"
SD sd;

uint8_t ready;
uint8_t buffer[message_size];

uint8_t read_file(uint32_t file_number)  //read the file and returns the pointer to file
{
	message m;
	uint8_t i;	
	
	if(CHECK_FILE_FLAG(file_number))	//
	{	
		for(i=0;i<messages_per_file;i++)
		{
			if(read_message(file_number,i) == 0x00);	
			{
				m=*ptr;
				tempf.sms[i]=m;	
			}
		}		
	tempf.flag=0x11; //file exists
	ptrf=&tempf;
	return 0x00;

	}
	else
	{
		return 0x11;
	}	
			
}

uint8_t write_file(uint32_t file_number,file *f)// writes the whole file
{
	file temp;
	message data,*d;
	uint8_t i;		
	
	if(!CHECK_FILE_DELETE_FLAG(file_number))
	{
		i=0;
		do{
			temp=*f;
			data=temp.sms[i];
			d=&data;		
				
	
		}while(write_message(file_number,d,i++)&& i<messages_per_file);	
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
uint8_t is_file_ready2_delete(uint32_t file_number)
{
	uint8_t i;
	i=0;
	do
	{
	}while(CHECK_MSG_DELETE(file_number,i));	

	if(i==messages_per_file)
	{
		SET_MSG_DELETE(file_number,i);
		if(delete_file(file_number))
		{
			return 0x00;//file is ready 2 delete & deleted succssfully
		}
		else
			return 0x01;//file is ready but not deleted
	}	
	else
		return 0x11;//file is not ready 2 delete
}

uint8_t delete_message(uint32_t file_number,uint8_t message_number)//deletes the message
{
	if(!CHECK_MSG_EXIST(file_number,message_number))
	{
		return 0x01;//file doesnot exists	
	}
	else if(CHECK_MSG_DELETE(file_number,message_number))

	{
		return 0x11;//file already deleted	
	}
	else
	{
		SET_MSG_DELETE(file_number,message_number);
		if(is_file_ready2_delete(file_number))//check all messages are deleted or not
			return 0x00;//file & message both deleted
		else
			return 0x10;//message deleted but not file
	}
	
}

uint8_t read_message(uint32_t file_number,uint8_t message_number)//reads particular message in queue
{
	uint32_t address;
	uint8_t i,length;
	if(CHECK_MSG_DELETE(file_number,message_number))//checking sms to read is not marked as delete
	{
		address=mat[file_number]->address[message_number];//getting address of the sms to read
		
		length=sd_read_block(&sd,address,buffer);
		if( length == message_size )
		{
			for(i=0;i<length;i++)
			{		
				temp.data[i]=buffer[i];
			}
			temp.flag=0x11;//message exisits

			if(delete_message(file_number,message_number))//call to delete sms
						;
			else
				return 0xf0;
			
			ptr=&temp;
		
			return 0x00;					
		}
		else
			return 0x0f;//Unable to Read Message
	}		
	else
	{
		return 0xff; //if sms is deleted send NULL
	}
}
uint8_t read_message_next(uint32_t file_number)//reads next message in queue
{
	uint32_t address;
	uint8_t i,next,length;

	next=mat[file_number]->next;//getting sms no to read	

	if(!CHECK_MSG_DELETE(file_number,next))//checking sms to read is not marked as delete
	{
		address=mat[file_number]->address[next];//getting address of the sms to read

		mat[(int)file_number]->next++;//pointing to next sms
		
		length=sd_read_block(&sd,address,buffer);
		if( length == message_size )
		{
			for(i=0;i<length;i++)
			{	
				temp.data[i]=buffer[i];
			}
			temp.flag=0x11;//message exisits
			ptr=&temp;
		
			return 0x00;					
		}
		else
			return 0x0f;//unable to read message	
	}	
	else
	{
	return 0xff; //if sms is deleted send NULL
	}
}

uint8_t write_message(uint32_t file_number,message *m,uint8_t message_number)//writes message at particular position 
{
	uint32_t address;
	uint8_t i;
	message temp,*ptr;

	ptr=m;
	temp=*ptr;
	if(!CHECK_MSG_EXIST(file_number,message_number))//checking message location is empty
	{
		address=mat[file_number]->address[message_number];
		i=0x00;
		while(i < message_size && (temp.data[i] != eom)) 	
		{
			buffer[i]=temp.data[i];
			i++;
		}
		//return sd_write_block(&sd,address,buffer);
		
	}
	else
		return 0xff;
}
uint8_t write_message_next(uint32_t file_number,message *m)//writes next message 
{
	uint32_t address;
	uint8_t i,number;
	message temp,*ptr;// should this not be commented out now?

	ptr=m;
	temp=*ptr;
	i=mat[file_number]->next;
	
	while(CHECK_MSG_EXIST(file_number,i))//getting last address of sms to write
			i++;
	number=i;
	
	address=mat[file_number]->address[i];
	i=0x00;
	while( temp.data[i] != eom )
	{
		buffer[i]=temp.data[i];
		i++;
	}
	
	return sd_write_block(&sd,address,buffer);
}

uint8_t setup_FAT()   //makes default entries in the FAT,fills sector address,file number,name,allcoation flag
{
	uint8_t i;	
	root->address[0]=starting_address;//address of the sector no. 60,000
	root->file_number[0]=starting_sector;
	CLEAR_FILE_FLAG(0);	
	for(i=1;i<files;i++)
	{
		root->file_number[i]=root->file_number[i-1]+64;
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

uint8_t fs_mount_chip(SD sd)
{
	int j;
    for(j=0;j<message_size;j++)buffer[j]=0xCC;
   ready = 0;
    /* Set some reasonable values for the timeouts.  */
    sd.write_timeout = 1000;
    sd.read_timeout = 1000;
    sd.busy_flag = 0;
    for (j=0; j<SD_INIT_TRY && ready != 1; j++)
    {
        spi_initialize();
	ready = sd_initialize(&sd);
    }
	return 0;
}

uint8_t setup_File_System()//sets up the basic Tables like FAT,MAT
{
	unsigned int i;
	uint8_t status=0x00;
	root=&root_fat;
	setup_FAT();	
	for(i=0;i<files;i++)
		mat[i]= root_mat+i;
	setup_MAT();

	if(!fs_mount_chip(sd))
		if(check_filesystem())
			 status=0xff;
	return status; //return 0xff on successfully setting up file system
}

uint8_t format_filesystem()
{
	return 1;//need to write stuff here
}

uint8_t check_filesystem()
{
	uint8_t status=0x00;
	/*
	length=_fs_read_id();
	if(length == 3)
		if(buffer[0] == 0x01)
			if(buffer[1] == 0x02)
				if(buffer[2] == 0x16)
					status=0xff;
	*/ 	//Need to write stuff here
	return status;
}				

