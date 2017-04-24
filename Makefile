DEVICE = msp430g2553
INCLUDE_DIR = $$MSP_HOME/msp430_gcc/include/

IFLAGS = -I $(INCLUDE_DIR)
LFLAGS = -L $(INCLUDE_DIR) -T $(DEVICE).ld
CFLAGS = $(IFLAGS) -Wall -g -mmcu=$(DEVICE)

CC = msp430-elf-gcc
LD = msp430-elf-ld

TARGET = msp430-ili9488

OBJECTS = $(TARGET).o

all: $(OBJECTS) 
	$(CC) $(CFLAGS) $(LFLAGS) -o $(TARGET).elf $(TARGET).c

clean:
	rm -fr $(TARGET).elf $(OBJECTS)