CC          = avr-gcc
LD          = avr-gcc
OBJCOPY     = avr-objcopy
AD          = avrdude

F_CPU       = 16000000
MCU         = atmega328p
AD_BAUDRT   = 57600
AD_CONFIG   = arduino
AD_DEVICE   = /dev/tty.usbserial-A6007AnN
AD_PARTNO   = m328p

ALL_CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DDEBUG -pedantic -Wall -Werror -Os $(CFLAGS)
ALL_LDFLAGS = -mmcu=$(MCU) $(LDFLAGS)
ALL_ADFLAGS = -Cavrdude.conf -cstk500v1 -p$(AD_PARTNO) -b$(AD_BAUDRT) -P$(AD_DEVICE) -D $(ADFLAGS)

TARGET      = libarduino2
SOURCE      = libarduino2.c main.c
OBJECTS     = $(SOURCE:=.o)

.PHONY: all clean rebuild install
.SUFFIXES: .elf .rom

all : $(TARGET).rom

clean :
	$(RM) $(TARGET).elf $(TARGET).rom $(OBJECTS)

rebuild : clean all

install : $(TARGET).rom
	$(AD) $(ALL_ADFLAGS) -Uflash:w:$^

$(TARGET).elf : $(OBJECTS)
	$(LD) $(ALL_LDFLAGS) -o $@ $^

%.rom : %.elf
	$(OBJCOPY) -O srec $< $@

%.c.o : %.c
	$(CC) $(ALL_CFLAGS) -c -o $@ $<
