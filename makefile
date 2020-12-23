
PROJECT = sid

CC =avr-g++
CFLAGS = -mmcu=atmega328p    \
				 -Wall -Os           \
				 --std=c++11         \
				 -DF_CPU=16000000UL  \
				 -fno-use-cxa-atexit \

INCLUDE = -Isrc\

SRC = $(wildcard src/*.cc) 

hex:
	$(CC) $(CFLAGS) $(INCLUDE) $(SRC) -o bin/$(PROJECT).elf
	avr-objcopy -j .text -j .data -O ihex bin/$(PROJECT).elf bin/$(PROJECT).hex
	avr-size --mcu=atmega328 --format=avr bin/$(PROJECT).elf

flash: hex
	@avrdude -p m328p -c usbtiny -b 57600 -e -U flash:w:bin/$(PROJECT).hex

flashad: hex
	@avrdude -p m328p -P /dev/ttyUSB0 -b 57600 -c arduino -e -U flash:w:bin/$(PROJECT).hex

clean:
	@rm bin/*.elf
	@rm bin/*.hex
	@rm bin/test*

default: hex
