#include <math.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "libarduino2.h"


#define BIT_SET(_out_, _bit_, _val_) ((_out_) = ((_out_) & ~(1 << (_bit_))) | ((_val_) << (_bit_)))
#define BIT_GET(_out_, _bit_) (((_out_) >> (_bit_)) & 1)
#define BIT_HI(_out_, _bit_) ((_out_) |= (1 << (_bit_)))
#define BIT_LO(_out_, _bit_) ((_out_) &= ~(1 << (_bit_)))

/*
 * Digital IO
 */
void digital_init(pin_t pin, pinmode_t mode) {
    if (pin < 8) {
        BIT_SET(DDRD, pin, mode);
    } else if (pin < 16) {
        BIT_SET(DDRB, pin - 8, mode);
    } else {
        ERROR("digital_init", "invalid pin index");
    }
}

bool digital_get(pin_t pin) {
    if (pin < 8) {
        return BIT_GET(PIND, pin);
    } else if (pin < 16) {
        return BIT_GET(PINB, pin - 8);
    } else {
        ERROR("digital_get", "invalid pin index");
        return 0;
    }
}

void digital_set(pin_t pin, bool value) {
    if (pin < 8) {
        BIT_SET(PIND, pin, value);
    } else if (pin < 16) {
        BIT_SET(PINB, pin, value);
    } else {
        ERROR("digital_set", "invalid pin index");
    }
}

/*
 * Analog Input (ADCs)
 */
#define ADC_NUM           6
#define ADC_MAXCLOCK      200000L
#define ADC_PRESCALE_BITS ((uint8_t)ceil(log((float)F_CPU / ADC_MAXCLOCK) / log(2)))
#define ADC_PRESCALE      ((uint8_t)pow(2, ADC_PRESCALE_BITS))
#define ADC_CUR           (ADMUX & 7)
#define ADC_NEXT          ((ADC_CUR + 1) & 7)
#define ADC_PREV          ((ADC_CUR) ? ADC_CUR - 1 : ADC_NUM - 1)
#define ADC_SET(_pin_)    (ADMUX = (ADMUX & ~7) | ((_pin_) & 7))

static uint16_t volatile g_adc_val[6];
static bool     volatile g_adc_done = false;

void analog_init(void) {
    int i;

    /* Set all pins connected to ADCs as analog inputs. */
    for (i = 0; i < ADC_NUM; ++i) {
        BIT_SET(DDRC, i, PIN_INPUT);
    }

    BIT_LO(PRR,    PRADC); /* disable power reduction */
    BIT_HI(ADCSRA, ADATE); /* auto-trigger on interrupt */
    BIT_HI(ADCSRA, ADIE);  /* interrupt on completion */
    BIT_HI(ADCSRA, ADEN);  /* enable */
    BIT_HI(ADMUX,  REFS0); /* use AVCC as the reference voltage */
    ADCSRA |= ADC_PRESCALE_BITS; /* clock divisor, three bits */

    /* Start the conversion on ADC0; triggers an interrupt when complete. */
    ADC_SET(0);
    BIT_HI(ADCSRA, ADSC); /* begin conversion */

    _delay_loop_2(ADC_PRESCALE);
    ADC_SET(ADC_NEXT);
}

uint16_t analog_get(pin_t pin) {
    uint16_t value;
    if (pin < ADC_NUM) {
        /* Reading 16-bit value from g_adc_cur must be atomic. */
        sei();
        value = g_adc_val[pin];
        cli();
        return value;
    } else {
        ERROR("analog_get", "invalid pin index");
        return 0;
    }
}

bool analog_available(void) {
    bool temp  = g_adc_done;
    g_adc_done = false;
    return temp;
}

ISR(ADC_vect) {
    g_adc_val[ADC_PREV] = ADC; /* ADC_CUR is in progress */
    g_adc_done = g_adc_done || !ADC_CUR;
    ADC_SET(ADC_NEXT);
}

/*
 * Analog Output (PWM)
 */
void pwm_init(pin_t pin) {
    if (pin == 1 || pin == 2) {
        BIT_HI(TCCR1A, WGM10); /* 8-bit */
        BIT_HI(TCCR1B, CS12);  /* divide by 256 */
        TCNT1 = 0;             /* initial value */
    } else if (pin == 3 || pin == 4) {
        BIT_HI(TCCR2A, WGM20);
        BIT_HI(TCCR2B, CS21);
        BIT_HI(TCCR2B, CS22);
        TCNT2 = 0;
    }

    switch (pin) {
    case 1:
        OCR1A = 0;              /* Initial value. */
        BIT_HI(DDRB, 1);        /* Set pin as output. */
        BIT_HI(TCCR1A, COM1A1); /* Enable */
        break;

    case 2:
        OCR1B = 0;
        BIT_HI(DDRB, 2);
        BIT_HI(TCCR1A, COM1B1);
        break;

    case 3:
        OCR2A = 0;
        BIT_HI(DDRB, 3);
        BIT_HI(TCCR2A, COM2A1);
        break;

    case 4:
        OCR2B = 0;
        BIT_HI(DDRD, 3);
        BIT_HI(TCCR2A, COM2B1);
        break;

    default:
        ERROR("pwm_init", "invalid pin index");
    }
}

void pwm_set(pin_t pin, uint8_t value) {
    switch (pin) {
    case 1:
        OCR1A = value;
        break;

    case 2:
        OCR1B = value;
        break;

    case 3:
        OCR2A = value;
        break;

    case 4:
        OCR2B = value;
        break;

    default:
        ERROR("pwm_set", "invalid pin index");
    }
}

/*
 * Serial Communication (UART)
 */
#define UART_BAUD (F_CPU / (SERIAL_BAUD * 16L) - 1)
#define UART_SIZE 16

static uint8_t volatile g_uart_buf[UART_SIZE];
static uint8_t volatile g_uart_read  = 0;
static uint8_t volatile g_uart_write = 0;
static FILE g_uart;

FILE *serial_init(void) {
    /* Set the default baud rate. */
    UBRR0H = UART_BAUD >> 8;
    UBRR0L = UART_BAUD;

    BIT_HI(UCSR0B, TXEN0);  /* enable send */
    BIT_HI(UCSR0B, RXEN0);  /* enable receive */
    BIT_HI(UCSR0B, RXCIE0); /* enable receive interrupt */
    sei();                  /* enable global interrupts */

    /* Use fdev_stream_setup() instead of fdevopen() to avoid malloc(). Unlike
     * fdevopen(), we must manually set stdin and stdout.
     */
    fdev_setup_stream(&g_uart, serial_putchar, serial_getchar, _FDEV_SETUP_RW);
    stdout = &g_uart;
    stderr = &g_uart;
    stdin  = &g_uart;
    return &g_uart;
}

int serial_getc(void) {
    char ch;

    /* Only read from the buffer if new data is available. */
    if (g_uart_read == g_uart_write) {
        ch = g_uart_buf[g_uart_read];
        g_uart_read = (g_uart_read + 1) % UART_SIZE;
        return ch;
    } else {
        return EOF;
    }
}

int serial_getchar(FILE *fp) {
    char ch;

    /* Block until data is available. */
    while (g_uart_read == g_uart_write);

    /* Read UART data from the ring buffer. */
    ch = g_uart_buf[g_uart_read];
    g_uart_read = (g_uart_read + 1) % UART_SIZE;
    return ch;
}

int serial_putchar(char ch, FILE *fp) {
    /* Block until space is available. */
    while (!BIT_GET(UCSR0A, UDRE0));
    UDR0 = ch; 
    return ch;
}

ISR(USART_RX_vect) {
    g_uart_buf[g_uart_write] = UDR0;
    g_uart_write = (g_uart_write + 1) % UART_SIZE;
}
