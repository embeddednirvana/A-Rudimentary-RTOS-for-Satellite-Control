/*##################################################################################### 
Copyright 2011  Shravan Aras, Nishchay Mhatre 

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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ielf.h"

/* Argument list.
* 1-> input file.bin.
* 2-> start offset.
* 3-> The output file.bin.
* 4-> The size of that section.
*/

/* Global function to store the array of parity bits. */
uint8_t parity[200];	 /* MAlloc this. */

int main(int argc,char *argv[]){
	FILE *file_read,*file_write;
	int a=0;
	uint8_t buffer;
	uint32_t buffer_inst;
	int count=0;

	file_read = fopen(argv[1],"r");
	/* Send the dam pointer to that binary location. */
	fseek(file_read,atoi(argv[2]),SEEK_SET);
	/* Now start reading the contents from that size till the end and dump everything into the second bin file. */
	file_write = fopen(argv[3],"w");

	for(a=0;a<atoi(argv[4]);a++){
		fread(&buffer,sizeof(char),1,file_read);
		fwrite(&buffer,sizeof(char),1,file_write);
	}

	/* Close all the files. */
	fclose(file_read);
	fclose(file_write);	

	file_read = fopen(argv[3],"r");
	for(a=0;a<atoi(argv[4])/4;a++){
		fread(&buffer_inst,sizeof(uint32_t),1,file_read);
		parity[a] = get_parity(buffer_inst); 
		count++;
	}
	printf("%d",count);
	dumpParity(count);
}

/* Code to make that automated C file. */
int dumpParity(int count){
	FILE *file;
	int a=0,b=0;
	
	file = fopen(PARITY_FILE,"w");
	/* Start dumping things into .c file. */
	fprintf(file,"/*This is an auto-generated file. Do not play with it :P*/\n\n");
	fprintf(file,"#include <stdint.h>\n\n");
	for(b=0;b<3;b++){	
		fprintf(file,"\nuint8_t data_parity%d[]={",b);
		for(a=0;a<count;a++){
			fprintf(file,"%d",parity[a]);
			if(a<count-1)
				fprintf(file,",");
		}
		fprintf(file,"};");
	}
}

/* NSM's code to calculate the parity bits. */
uint8_t get_parity(uint32_t instr)
{
 uint8_t alias[] ={3,5,6,7,9,10,11,12,13,14,15,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,33,34,35,36,37,38};
 int i;
 int temp;
 uint8_t parity;

 parity = 0x0;

 for(i=0;i<32;i++)
 	{
	 if ( instr & (1<<i) ) 
	 	parity = parity ^ alias[i];
	}
 return parity;
}
