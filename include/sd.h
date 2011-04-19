/*##################################################################################### 
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
##################################################################################### */ 
/* Author : Shravan Aras. 
 * CoEP BOB project.
 * Ack : These comands have been extracted from Michigan Sate University 2004 application note 
 * 	 from Dept. of Electrical and Computer Eng.
 * Date : 26.05.10
 * Stores a basic list of comands which are specified by the SD protocol. 
 * Revision 1 : New response called SR has been added. This will store the reponse after a successful CSD command.
 * */


typedef struct _sd{
	uint32_t write_timeout;		/* Clock cycles after which writing should timeout. */
	uint32_t read_timeout;		/* Clock cycles after which reading should timeout. */
	uint8_t busy_flag;		/* Is set saying that the sd card interface is busy. */
}SD;

#define SD_BLOCKSIZE 512	
#define SD_BLOCKSIZE_NBITS 9

#define R1 1
#define R1B 2
#define R2 3
#define R3 4
#define SR 5			/* Added as revision 1. */
#define MSK_IDLE          0x01
#define MSK_ERASE_RST     0x02
#define MSK_ILL_CMD       0x04
#define MSK_CRC_ERR       0x08
#define MSK_ERASE_SEQ_ERR 0x10
#define MSK_ADDR_ERR      0x20
#define MSK_PARAM_ERR     0x40
#define SD_TOK_READ_STARTBLOCK    0xFE
#define SD_TOK_WRITE_STARTBLOCK   0xFE
#define SD_TOK_READ_STARTBLOCK_M  0xFE
#define SD_TOK_WRITE_STARTBLOCK_M 0xFC
#define SD_TOK_STOP_MULTI         0xFD

/* Error token is 111XXXXX */
#define MSK_TOK_DATAERROR         0xE0

/* Bit fields */
#define MSK_TOK_ERROR             0x01
#define MSK_TOK_CC_ERROR          0x02
#define MSK_TOK_ECC_FAILED        0x04
#define MSK_TOK_CC_OUTOFRANGE     0x08
#define MSK_TOK_CC_LOCKED         0x10

/* Mask off the bits in the OCR corresponding to voltage range 3.2V to
 *  * 3.4V, OCR bits 20 and 21 */
#define MSK_OCR_33        0xC0
/* Number of times to retry the probe cycle during initialization */
#define SD_INIT_TRY 50
/* Number of tries to wait for the card to go idle during initialization */
#define SD_IDLE_WAIT_MAX      100
/* Hardcoded timeout for commands. 8 words, or 64 clocks. Do 10
 *  * words instead */
#define SD_CMD_TIMEOUT    100

/******************************** Basic command set **************************/
/* Reset cards to idle state */
#define CMD0 0
#define CMD0_R R1

/* Read the OCR (MMC mode, do not use for SD cards) */
#define CMD1 1
#define CMD1_R R1

/* Card sends the CSD */
#define CMD9 9
//#define CMD9_R R1
#define CMD9_R SR

/* Card sends CID */
#define CMD10 10
#define CMD10_R R1

/* Stop a multiple block (stream) read/write operation */
#define CMD12 12
#define CMD12_R R1B

/* Get the addressed card’s status register */
#define CMD13 13
#define CMD13_R R2

/***************************** Block read commands **************************/
/* Set the block length */
#define CMD16 16
#define CMD16_R R1
/* Read a single block */
#define CMD17 17
#define CMD17_R R1
/* Read multiple blocks until a CMD12 */
#define CMD18 18
#define CMD18_R R1
/***************************** Block write commands *************************/
/* Write a block of the size selected with CMD16 */
#define CMD24 24
#define CMD24_R R1
/* Multiple block write until a CMD12 */
#define CMD25 25
#define CMD25_R R1
/* Program the programmable bits of the CSD */
#define CMD27 27
#define CMD27_R R1
/***************************** Write protection *****************************/
/* Set the write protection bit of the addressed group */
#define CMD28 28
#define CMD28_R R1B
/* Clear the write protection bit of the addressed group */
#define CMD29 29
#define CMD29_R R1B
/* Ask the card for the status of the write protection bits */
#define CMD30 30
#define CMD30_R R1
/***************************** Erase commands *******************************/
/* Set the address of the first write block to be erased */
#define CMD32 32
#define CMD32_R R1
/* Set the address of the last write block to be erased */
#define CMD33 33
#define CMD33_R R1
/* Erase the selected write blocks */
#define CMD38 38
#define CMD38_R R1B
/***************************** Lock Card commands ***************************/
/* Commands from 42 to 54, not defined here */
/***************************** Application-specific commands ****************/
/* Flag that the next command is application-specific */
#define CMD55 55
#define CMD55_R R1
/* General purpose I/O for application-specific commands */
#define CMD56 56
#define CMD56_R R1
/* Read the OCR (SPI mode only) */
#define CMD58 58
#define CMD58_R R3
/* Turn CRC on or off */
#define CMD59 59
#define CMD59_R R1
/***************************** Application-specific commands ***************/
/* Get the SD card’s status */
#define ACMD13 13
#define ACMD13_R R2
/* Get the number of written write blocks (Minus errors ) */
#define ACMD22 22
#define ACMD22_R R1
/* Set the number of write blocks to be pre-erased before writing */
#define ACMD23 23
#define ACMD23_R R1
/* Get the card’s OCR (SD mode) */
#define ACMD41 41
#define ACMD41_R R1
/* Connect or disconnect the 50kOhm internal pull-up on CD/DAT[3] */
#define ACMD42 42
#define ACMD42_R R1
/* Get the SD configuration register */
#define ACMD51 42
#define ACMD51_R R1

/******************** Macros which will control the PIO pins. *******************/
#define SPI0_CS_ASSERT AT91C_BASE_PIOA->PIO_ODSR &= (~(0x1<<13))
#define SPI0_CS_DEASSERT AT91C_BASE_PIOA->PIO_ODSR |= (0x1<<13)
/******************* Stores a few definations for the spi peripheral ************/
#define SPI0 AT91C_BASE_SPI0

/********************* Error Bits(Only for testing.)***********************/
#define E_INI 			(0x1<<0)
#define E_BLOCKLENGTH 		(0x1<<1)
#define E_READCMD 		(0x1<<2)
#define E_READTIMEOUT 		(0x1<<3)
#define E_READTOKEN 		(0x1<<4)
#define E_WRITECMD 		(0x1<<5)
#define E_CMDTIMEOUT 		(0x1<<6)
#define E_RESERVED		(0x1<<7)

/* Prototypes of functions. */
void spi_initialize();
uint32_t sd_initialize(SD *sd);
uint32_t sd_read_block(SD *sd,uint32_t blockadd,uint8_t *data);\
//! This function is used to read from multiple blocks located in cosecutive memory locations.
//! ARG0 : Pointer to the sd card pointer.
//! ARG1 : Address of the starting block.
//! ARG2 : Buffer in which the read data is stored.
//! ARG3 : Number of consecutive blocks the user wished to read.
uint32_t sd_read_multi_block(SD *sd,uint32_t blockadd,uint8_t *data,uint8_t blocks);
uint32_t sd_write_block(SD *sd,uint32_t blockadd,uint8_t *data);
//! This function is used to write to multiple blocks located in cosecutive memory locations.
//! ARG0 : Pointer to the sd card pointer.
//! ARG1 : Address of the starting block.
//! ARG2 : Data to be written into the block.
//! ARG3 : Number of consecutive blocks the user wishes to write.
uint32_t sd_write_multi_block(SD *sd,uint32_t blockadd,uint8_t *data,uint8_t blocks);
uint32_t is_card_busy(SD *sd);
uint32_t sd_send_command(SD *sd,uint8_t cmd,uint8_t response_type);
void sd_delay(uint16_t time);			/* Function prototype changed on 17-09-10. */
void sd_packarg(uint8_t *arguments,uint32_t value);
int sd_set_block_length(SD *sd,uint32_t length);
int sd_get_CSD(SD *sd);
