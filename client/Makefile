CC=gcc
OBJCOPY=avr-objcopy

client: client.c ../common/xsvf.c ../common/xsvf.h
	$(CC) -o $@ $^


.phony: clean

clean:
	rm -f client

