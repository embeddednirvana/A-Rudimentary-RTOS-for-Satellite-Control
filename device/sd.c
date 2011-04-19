/*_____________________________________________________________________________________________
Copyright 2011 Shravan Aras

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
/* Author : Shravan Aras.
 * Date : 26.05.10
 * Last Modified : 22-09-10
 * CoEP Sat project.
 * SD 1.0/2.0 protocol implementation over SPI.
 * Bug Revisions :
 * Revision 1 (29-05-10) : Indicated by the Revision 1 tag. 
 * Revision 2 (30-05-10) : Indicated by the Revision 2 tag.
 * Revision 3 (30-05-10) : A read needs write. I have supporting documentation for this experimental results.
 * Revision 4 (15-09-10) : Major changes were changing the order of the array writing and reading , adding sd_delay()'s 
 * 			   to synchronize the the card's various operation accoring to the product manual timing diagram.
 * Revision 5 (18-09-10): Sucessive reads and write and are now possible. A delay of minimum 10 clock cycles should be given 
 *                        after the sd_write_block command , before any other command can be invoked.
 * Revision 6 (22-09-10): Tagged with the name revision 6 kindly search in the code.                       
 */

#include<stdint.h>
#include"AT91SAM7X256.h"
//#include"driver.h"
#include"spi.h"
#include"sd.h"

uint8_t response[8];	/* Stores the ack/response given by the SD card. */
//uint8_t temp_response[5];
unsigned short global_buffer;
uint8_t arguments[4];	/* Stores the 4 byte argument which is passed after initial frame. */
uint8_t error;		/* Added only for testing reasons to see where the error has occured. */
uint8_t asert;		

/* Code to initialize the SPI peripheral. */
void spi_initialize(){
	
	AT91C_BASE_PIOA->PIO_OER = (0x1<<13);		/* SS pin has been given control to PIO. */
	AT91C_BASE_PIOA->PIO_OWER = (0x1<<13);		
	//AT91C_BASE_PIOA->PIO_PPUDR = (0x1<<13);	/* Removed as of revision 1. */ 
	//AT91C_BASE_PIOA->PIO_PPUDR = (0x1<<18);
	SPI0_CS_DEASSERT;				/* Pull the chip select pin high. */

	SPI_Configure(SPI0,4,AT91C_SPI_MSTR|SPI_PCS(1)|AT91C_SPI_PS_FIXED); 	/* Configure the SPI Peripheral. */
	/* Character length : 8 bits.
	 * Baud Rate : 400kHz
	 */
//	SPI_ConfigureNPCS(SPI0,1,AT91C_SPI_CSAAT|AT91C_SPI_BITS_8|SPI_SCBR(5000,48000000));	
	SPI_ConfigureNPCS(SPI0,1,AT91C_SPI_CSAAT|AT91C_SPI_BITS_8|(0xFF<<8)|AT91C_SPI_CPOL);

	SPI_Enable(SPI0);	/* Enable the SPI Peripheral device. */

	error = 0x00;		/* Reset the error register. */
	
	asert = 0;
}

uint32_t sd_initialize(SD *sd){
	uint8_t i;

	sd->busy_flag = 1;	/* The SD card interface is busy. */
	
	for(i=0;i<4;i++)arguments[i] = 0;	
	for(i=0;i<5;i++)response[i] = 0;	

	//sd_delay(100);
	SPI0_CS_ASSERT;		/* Select the card. */
	sd_delay(100);		/* Delay for 80 (74 recomended) clock cycle. SD circuitry power up time. */
	SPI0_CS_DEASSERT;	
	sd_delay(2);		/* 16 clock cycle delay. */

	SPI0_CS_ASSERT;		/* added on 17-10-2010. */

	/* Put this card into idle mode. */
	if(sd_send_command(sd,CMD0,CMD0_R)==0){
		error |= E_INI;	
		SPI0_CS_DEASSERT;	/* Added on 17-10-2010. */
		return 0;	/* Bail out something went wrong here. */
	}


	/* Keep initializing the card until it goes into idle state or times out. */
	i=0;
	do{
		i++;
		/* Send a command saying that the next command is going to an application specific command. */
		if(sd_send_command(sd,CMD55,CMD55_R)==1){
			/* Send a command to initialize the SD card. ACMD41 */
			 sd_send_command(sd,ACMD41,ACMD41_R);

		}
		else{
			/* Because the CMD55 command was not accepted there is no point in going ahead. */
			i=SD_IDLE_WAIT_MAX;
		}
	}while((response[0]&MSK_IDLE)==MSK_IDLE && i<SD_IDLE_WAIT_MAX);

	if(i>=SD_IDLE_WAIT_MAX){
		error |= E_INI;
		SPI0_CS_DEASSERT;	/* Added on 17-10-2010. */
		return 0;	/* Exit as something has gone wrong. */
	}


	/* It is recommended that we check the OCR register to see if the voltage of the processor 
	 * is supported by the SD card. But as this is not a generic application code we do not need 
	 * to test for this condition hence we skip the CMD58 command.
	 */

	/* Set the input output block size. */
	if(sd_set_block_length(sd,SD_BLOCKSIZE)==0){
		error |= E_BLOCKLENGTH;
		SPI0_CS_DEASSERT;	/* Added on 17-10-2010. */
		return 0;	/* Damm !! Something went wrong.... Stop ! Stop ! :D */
	}

	SPI0_CS_DEASSERT;	/* Added on 17-10-2010. */

	return 1;	/* Yeppi initialization of the card is over :P we can now move on to writing and reading data. */
}

/* Function to read a block  of data from the SD card.
 * The data is stored in the passed buffer and a terminating \0 character is added at the end. */
uint32_t sd_read_block(SD *sd,uint32_t blockadd,uint8_t *data){
	uint8_t i,tmp;
	int32_t j;

	while(SPI_IsFinished(SPI0));		/* Timeout should be addded here but later. */
	
	SPI0_CS_ASSERT;	/* Added on 17-10-2010. */

	sd_packarg(arguments,blockadd);
	if(!sd_send_command(sd,CMD17,CMD17_R))return 0;

	sd_delay(1);					/* Added on 15-09-2010. */

	//temp_response[0] = response[0];		/* Added for testing. */

	if(response[0] != 0){
		error |= E_READCMD;
		SPI0_CS_DEASSERT;	/* Added on 17-10-2010. */
		return 0;		/* Some error has taken place like invalid address etc. */
	}


	//SPI0_CS_ASSERT;

	/* Now keep polling until data is received or until time-out. */
	i=0;
	do{
		i++;
		SPI_Write(SPI0,1,0xFF);
		tmp = SPI_Read(SPI0);
	}while(tmp==0xFF && i<=sd->read_timeout);

	if(i>sd->read_timeout){
		SPI0_CS_DEASSERT;
		error |= E_READTIMEOUT;	
		return 0;
	}
	//while(is_card_busy(sd)!=0xFF);		/*Wait here till the bus is not busy. */

	//temp_response[0] = tmp;
	/* Check to see if the token received was an error token . 
	 * And error character is detected by the first 3 bits of the response going low.*/
	if((tmp & MSK_TOK_DATAERROR) == 0){
		/* The proto reqs. one byte to be clocked out before stoping. */
		//SPI_Write(SPI0,1,0xFF);
		//SPI0_CS_DEASSERT;
		error |= E_READTOKEN;
		//return 0;	/* Return with an error saying somthing has gone wrong. */
	}


	/* If we have reached so far it means there was no error and we can successfuly read SD_BLOCKSIZE bytes. */
	/* Store it in MSB first fashion. */
	data[0] = tmp;

	/* Drop the 1st byte it is always FE (Revision 6). */	

	for(j=0;j<SD_BLOCKSIZE;j++){
		SPI_Write(SPI0,1,0xFF);
		data[j] = SPI_Read(SPI0);
	}

	sd_delay(4);				/* added on 18-09-10 */

	SPI0_CS_DEASSERT;

	return 1;	/* ALLL IS WELL .... */

}

/* Function to read multiple blocks of data. */
uint32_t sd_read_multi_block(SD *sd,uint32_t blockadd,uint8_t *data,uint8_t blocks){
return 0;// Added by RB
}

/* Function to write a block of data to the SD card. 
 * Only the first SD_BLOCKSIZE bits will be sent. 
 * Giving it a data of less that SD_BLOCKSIZE bytes can lead to undefined results.
 * Incase more than SD_BLOCKSIZE bytes are given only the first SD_BLOCKSIZE bytes are written.
 */
uint32_t sd_write_block(SD *sd,uint32_t blockadd,uint8_t *data){
	int32_t j;
	
	//blockadd <<= SD_BLOCKSIZE_NBITS; 	/* Adjust the address to a linear boundary. */
	
	//while(SPI_IsFinished(SPI0));		/* No timeout meassure added for this polling at this stage. */

	SPI0_CS_ASSERT;				/* Added on 17-10-2010. */

	sd_packarg(arguments,blockadd);		
	if(sd_send_command(sd,CMD24,CMD24_R)==0)return 0;	/* Bail out something went wrong. */
	
	if(response[0] != 0){
		error |= E_WRITECMD;
		SPI0_CS_DEASSERT;				/* Added on 17-10-2010. */
		return 0;	/* Some error has taken place.*/
	}

	//SPI0_CS_ASSERT;

	//SPI_Read(SPI0);			/* Wait idle for 8 clock cycles. */
	sd_delay(2);

	SPI_Write(SPI0,1,SD_TOK_WRITE_STARTBLOCK);	/* Send the start token to indicate start of the block. */

	for(j=0;j<SD_BLOCKSIZE;j++){			
		SPI_Write(SPI0,1,data[j]);
	}

	SPI_Write(SPI0,1,0x95);				/* Send a dummy CRC just to end the proto*/

	//temp_response[0] = SPI_Read(SPI0);		/* Added on 18-09-10 to study the response of the system. */
	sd_delay(2);					/* Added on 18-09-10 to omit the response given by the card. */

	while(is_card_busy(sd)!=0xFF);	/* Wait till the card gets out from its busy state. */


	//sd_send_command(sd,CMD13,CMD13_R);
	//for(j=0;j<5;j++)temp_response[j] = response[j];	

	sd_delay(4);			/* Drop the response 15-09-2010. We may record this response later. */

	SPI0_CS_DEASSERT;

	/* The card will now become busy in programming these characters
	 * We do not wait in this and return immediately. The is_card_busy function can be used to determine whether the chip is busy or not. */


	return 1;		/* ALLL IS WELL .... */
}

/* Function to write multiple consequtive blocks. */
uint32_t sd_write_multi_block(SD *sd,uint32_t blockadd,uint8_t *data,uint8_t blocks){

	uint32_t i,j;

	while(SPI_IsFinished(SPI0));	/* Wait till the spi peripheral has finished doing whatever it was doing before. */

	SPI0_CS_ASSERT;						/* Make the CS line low. */

	sd_packarg(arguments,blockadd);		
	if(sd_send_command(sd,CMD25,CMD25_R)==0)return 0;	/* Bail out something went wrong. */

	/* Add response error check code here. */

	sd_delay(2);
	
	j=0;
	for(i=1;i<=blocks;i++){
		SPI_Write(SPI0,1,SD_TOK_WRITE_STARTBLOCK);	/* Send the start token to indicate start of the block. */
		for(;j<(SD_BLOCKSIZE*i);j++){			
			SPI_Write(SPI0,1,data[j]);
		}
		SPI_Write(SPI0,1,0x95);				/* Send the respective CRC. */
		sd_delay(1);					/* Drop the R1 response for now. Should be analysed in the final implementation. */
		while(is_card_busy(sd)!=0XFF);			/* Wait for the card to become idle before writing another data block to it. */
	}	
	sd_delay(2);						/* Nwr time delay value. */
	SPI_Write(SPI0,1,SD_TOK_STOP_MULTI);			/* Send the stop transfer token. */
	sd_delay(1);						/* Nbr time delay value. */
	while(is_card_busy(sd)!=0XFF);

	SPI0_CS_DEASSERT;

	sd_delay(4);						/* Nds experimental value. */

	return 1;						/* No error worked just fine. */
}

/* Will return 0 if the chip is busy and any other non-zero value if the card is not busy. */
uint32_t is_card_busy(SD *sd){
	SPI_Write(SPI0,1,0xFF);
	return(SPI_Read(SPI0));
}

/* Function which is used to send commands. */
uint32_t sd_send_command(SD *sd,uint8_t cmd,uint8_t response_type){
	int8_t i;
	uint8_t response_length=0;
	uint8_t tmp_buffer;
	uint8_t buffer[5];	/* Stores the PDC buffer. */

	//SPI0_CS_ASSERT;
	//sd_delay(100);		

	cmd = (cmd & 0x3F)|0x40;		/* Convert the command into 01XXXXXX format. */	
	buffer[0] = cmd;
	SPI_Write(SPI0,1,cmd);				/* Write the command. */
	
	/* Send the arguments. */
	for(i=3;i>=0;i--){
		SPI_Write(SPI0,1,arguments[i]);		/* MSB goes first. */
		//buffer[4-i] = arguments[i];
	}

	/* The CRC is not important in SPI mode but it is required before the CMD0 command as prior
	 * to this command the SD is in 1-bit SD Mode or 4-bit SD Mode.
	 */
	SPI_Write(SPI0,1,0x95);		/* As CMD0 takes no arguments its CRC value is a constant 0x95.*/
	//buffer[4] = 0x95;

	//SPI_WriteBuffer(SPI0,buffer,5);

	/* Determine the response length. */
	/* Revision 2 : switch(reponse_length) was replaced by switch(reponse_type) */

	/* Remove this dam stupid switch case by #defining the values directly. */

	switch(response_type){
		case R1:	/* Do nothing R1 and R1B have the same response. */
		case R1B:	/* R1 busy response. */
			response_length = 1;			/* Change made here according to revision 4. */
			break;
		case R2:
			response_length = 2;
			break;
		case R3:
			response_length = 6;
			break;
		case SR:
			response_length = 16;
		default:
			break;
	}

	//sd_delay(8);						/* Date 16-10-2010. */ 

	/* Wait for a response. 
	 * The response is detected by the start bit. */
	i=0;
	do{
		i++;
		tmp_buffer = SPI_Read(SPI0);	/* Testing */
		SPI_Write(SPI0,1,0xFF);		/* Added while testing revision 3 */
		//SPI_ReadBuffer(SPI0,buffer,1);
	}while(((tmp_buffer & 0x80)!=0) && i<SD_CMD_TIMEOUT); 		/* Refer to revision 5 here. */	
	//while(((tmp_buffer & 0x80)!=0));	

	//global_buffer = tmp_buffer;

	/* Check to see if the request timed-out. */
	if(i>=SD_CMD_TIMEOUT){
		SPI0_CS_DEASSERT;		
		error |= E_CMDTIMEOUT;
		return 0;		/* Stop the initiliazation something has gone wrong. */
	}

	for(i=response_length-1;i>=0;i--){
		response[i] = tmp_buffer;
		SPI_Write(SPI0,1,0xFF);
		tmp_buffer = SPI_Read(SPI0);
	}

	/* Special handling code to handle the R1 busy state. We must wait till the first non-zero character appears. 
	 * That is the response idle bit is reset.
	 * The R1B response can be issued during hot swapping of cards ... but it space only aliens can do it :P So after testing remove this part.
	 * No time-out condition is set here. To be discused with nsm wether to put a time-out check here also?
	 */
	if(response_type == R1B){
		do{
			SPI_Write(SPI0,1,0xFF);
			tmp_buffer = SPI_Read(SPI0);
		}while((tmp_buffer & 0xFF) != 0xFF); 	/* Do until the bus does not get back to in-active state. */

		SPI_Write(SPI0,1,0xFF);		/* Make sure the buss is made high after the busy state ends. */
	}

	//SPI0_CS_DEASSERT;

	return 1;				/* Done phew !! no problem :). */
}

/* Function which clocks out a delay. */
void sd_delay(uint16_t time){
	uint8_t i;
	for(i=0;i<time;i++){
		SPI_Write(SPI0,1,0xFF);		/* Write in-active data on the buss. */
	}
}

/* Function to packetize the argument. 
 * MSB goes into 3 and LSB goes into 0*/
void sd_packarg(uint8_t *arguments,uint32_t value){
	arguments[3] = (uint8_t)(value>>24);
	arguments[2] = (uint8_t)(value>>16);
	arguments[1] = (uint8_t)(value>>8);
	arguments[0] = (uint8_t)(value>>0);	/* Written for formality sake has no effect except add ROL cmd. */
}

/* Function to set the block length. */
int sd_set_block_length(SD *sd,uint32_t length){
	sd_packarg(arguments,length);	
	return(sd_send_command(sd,CMD16,CMD16_R));
}

/* Test function to get the cards CSD register contents to study them. */
int sd_get_CSD(SD *sd){
	//uint8_t tmp;
	///int8_t index;

	//sd_send_command(sd,CMD9,CMD9_R);
	//for(index=0;index<16;index++)sd_CSD[index] = response[index];
/*
	SPI0_CS_ASSERT;

	* Poll until some data does not start coming. *
	
	do{
		SPI_Write(SPI0,1,0xFF);
		tmp = SPI_Read(SPI0);
	}while(tmp != 0xFF);

	asert = 0x1;		* If it has come here it means that things have gone well so far. *

	for(index=15;index>=0;index--){
		sd_CSD[index] = tmp;
		SPI_Write(SPI0,1,0xFF);
		tmp = SPI_Read(SPI0);
	}

	SPI0_CS_DEASSERT;
*/
	return 1;	
}

