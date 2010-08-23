#ifndef LIBARDUINO2_H_
#define LIBARDUINO2_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#if defined(DEBUG)
#define ERROR(_func_, _msg_) printf("err: %s: %s\n", (_func_), (_msg_))
#else
#define ERROR(_func_, _msg_)
#endif

#if (!defined(__AVR_ATmega168__) && !defined(__AVR_ATmega328P__))
#error "Processor not supported by libarduino2."
#endif

#if (F_CPU != 16000000 && F_CPU != 8000000)
#error "Processor clock speed not supported by libarduino2."
#endif

#define SERIAL_BAUD 57600

typedef enum {
    PIN_INPUT  = 0,
    PIN_OUTPUT = 1
} pinmode_t;
typedef uint8_t pin_t;

void digital_init(pin_t, pinmode_t);
bool digital_get(pin_t);
void digital_set(pin_t, bool);

void analog_init(void);
uint16_t analog_get(pin_t);
bool analog_available(void);

void pwm_init(pin_t);
void pwm_set(pin_t, uint8_t);

FILE *serial_init(void);
int serial_getc(void);
int serial_getchar(FILE *);
int serial_putchar(char, FILE *);

int readline(char *, char *);

#endif
