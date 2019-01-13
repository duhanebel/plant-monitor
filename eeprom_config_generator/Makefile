CC=gcc
CFLAGS=-I../lib/eeprom_config
DEPS = ../lib/eeprom_config/eeprom_config.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

build: main.o eeprom_config.o
	$(CC) -o eeconfgen main.o eeprom_config.o
	