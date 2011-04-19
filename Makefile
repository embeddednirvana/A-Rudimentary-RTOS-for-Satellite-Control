###################################################################################### 
#	 Copyright 2011 Nishchay Mhatre 

#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#	http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.

##################################################################################### */ 

#Makefile for SST Integrated  build on ARM7-TDMI based AT91SAM 7X256

# Toolchain definition
TC=arm-none-eabi

# Toolchain depedent GNU build sofware
COMPILE=arm-none-eabi-gcc
LOAD=$(TC)-ld
ASSEMBLE=$(TC)-as
DEBUG=$(TC)-gdb
PROFILE=$(TC)-prof
OBJCOPY=$(TC)-objcopy

#Linker script
LDSCRIPT=sst.ld

# Target Machine Specifications
CPU=arm7tdmi
ARCH=armv4t

# Build directories

BLDDIR= .
BINDIR = ./bin 

APPSRC=$(BLDDIR)/app
FS=$(BLDDIR)/fs
INC=$(BLDDIR)/include
SYSTEM=$(BLDDIR)/system

# Compiler options

DBG=-g
PROF=-pg
OPTIM=-mlow-irq-latency

CFLAGS=	$(DBG)\
	-mcpu=$(CPU) \
	-marm \
	-mabi=aapcs \
	-fomit-frame-pointer\
	-I$(INC)\
	-T $(LDSCRIPT)

#Linker flags
LIBS=-Wl,--start-group -lgcc -Wl,--end-group 
LINKER_FLAGS=-nostartfiles -Xlinker -osst.elf -Xlinker -M -Xlinker -Map=sst.map
	
#Asm flags
ASFLAGS=-g -gdwarf2 -mcpu=arm7tdmi #-mthumb-interwork -mapcs-frame

ARM_SOURCE= ./main.c \
	    $(SYSTEM)/low_level_init.c \
	    $(SYSTEM)/swi.c \
	    $(SYSTEM)/task.c \
	    $(SYSTEM)/exc.c \
	    ./device/driver_pio.c\
	    ./device/usart.c\
	    ./device/rtt.c\
	    ./device/spi.c\
	    ./device/sd.c\
	    $(APPSRC)/bob_tasks.c\
	    $(FS)/sat_fs.c

ARM_OBJS = $(ARM_SOURCE:.c=.o)
ARM_ASM = $(ARM_SOURCE:.c=.s)

VERSION=11
#This is the 11th build of this system
NAME=sst_$(VERSION)

all: $(NAME).bin
asm: $(ARM_ASM)

$(NAME).bin : sst.elf
	$(OBJCOPY) sst.elf -O binary $(NAME).bin

sst.elf : $(ARM_OBJS) boot.o exceptions.o  Makefile
	$(COMPILE) $(CFLAGS) $(LINKER_FLAGS) $(ARM_OBJS) boot.o exceptions.o 

boot.o : ./system/boot.s
	$(ASSEMBLE) $(ASFLAGS) -o boot.o ./system/boot.s

exceptions.o : ./system/exceptions.s
	$(ASSEMBLE) $(ASFLAGS) -o exceptions.o ./system/exceptions.s

hamming2.o : ./ecc/hamming2.s
	$(ASSEMBLE) $(ASFLAGS) -o hamming2.o ./ecc/hamming2.s 

scrubs.o : ./ecc/scrubs.s
	$(ASSEMBLE) $(ASFLAGS) -o scrubs.o ./ecc/scrubs.s 

$(ARM_OBJS) : %.o : %.c Makefile 
	$(COMPILE) $(CFLAGS) -c $< -o $@ 

$(ARM_ASM) : %.s : %.c Makefile 
	$(COMPILE) $(CFLAGS) -S $< -o $@ 
	cp  ./*/*.s ./asm

clean :
	rm -rf $(ARM_OBJS)
	rm -rf boot.o exceptions.o
	touch Makefile
	rm -rf sst.elf
	
clean-asm:
	rm -rf asm/*.s
