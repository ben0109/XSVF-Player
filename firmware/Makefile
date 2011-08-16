BIT_DIR=/home/ben/prog/papilio/ArduinoSoftCore/scripts/XilinxISE
BIT_FILE=$(BIT_DIR)/Papilio_AVR8.bit
BMM_FILE=$(BIT_DIR)/Papilio_AVR8_bd.bmm

# Papilio dev environment
BUTTERFLYPROG=butterflyprog
SREC_CAT=srec_cat
GAWK=gawk
DATA2MEM=/home/ben/prog/Xilinx/13.1/ISE_DS/ISE/bin/lin/data2mem

# AVR dev environment
MCU=atmega103
CC=avr-gcc
OBJCOPY=avr-objcopy
# optimize for size:
CFLAGS=-g -mmcu=$(MCU) -Wall -Wstrict-prototypes -Os -mcall-prologues

# needed objects
OBJECTS=main.o ports.o states.o uart.o

# output level
CFLAGS:=$(CFLAGS) -D LOG_LEVEL=3 -D DEBUG_MODE

load: avr.bit
	sudo $(BUTTERFLYPROG) -v -f avr.bit

avr.bit: avr_progmem.mem
	$(DATA2MEM) -bm $(BMM_FILE) -bd avr_progmem.mem -bt $(BIT_FILE) -o b avr.bit

avr_progmem.mem: avr_progmem.hex
	$(SREC_CAT)  $< -Intel -Byte_Swap 2  -Data_Only -o tmp.mem -vmem 8
	$(GAWK) ' BEGIN{FS=" ";} { $$1= ""; print}' tmp.mem > $@
	rm tmp.mem


avr_progmem.hex : avr_progmem.out 
	$(OBJCOPY) -R .eeprom -O ihex avr_progmem.out avr_progmem.hex 
avr_progmem.out : $(OBJECTS)
	$(CC) $(CFLAGS) -o avr_progmem.out -Wl,-Map,avr_progmem.map $^
%.o : %.c 
	$(CC) $(CFLAGS) -Os -c $<
%.o : %.S
	$(CC) $(CFLAGS) -Os -c $<

.phony: clean

clean:
	rm -f avr.bit avr_progmem.mem avr_progmem.hex avr_progmem.out avr_progmem.map *.o

