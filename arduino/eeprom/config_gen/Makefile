CC=gcc
CFLAGS=-c -Wall -I../../lib/eeprom_config
EXECUTABLE=eeconfgen

src = $(wildcard *.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)  # one dependency file for each source

all: clean cleandep $(EXECUTABLE)

$(EXECUTABLE): $(obj) libeeconfig
	$(CC) -o $@ $^ 

-include $(dep)   # include all dep files in the makefile

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

libeeconfig: 
	$(CC) -c -x c ../../lib/eeprom_config/eeprom_config.cpp -o libeeconfig

.PHONY: clean
clean:
	rm -f $(obj) $(EXECUTABLE) libeeconfig

.PHONY: cleandep
cleandep:
	rm -f $(dep)

