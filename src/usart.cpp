#include <stdio.h>

#include <avr/io.h>
#include "usart.h"


#undef FDEV_SETUP_STREAM
#define FDEV_SETUP_STREAM(p, g, f) \
    {                              \
        .buf = NULL,               \
        .unget = 0,                \
        .flags = f,                \
        .size = 0,                 \
        .len = 0,                  \
        .put = p,                  \
        .get = g,                  \
        .udata = 0                 \
    }

static int _usart0_putchar(char c, FILE *stream);
static FILE USART0_stdout = FDEV_SETUP_STREAM(
    _usart0_putchar, NULL, _FDEV_SETUP_WRITE);

#define CALC_USART_UBRR(BAUD) (F_CPU / 16 / (BAUD) - 1)

void USART0_init(unsigned int ubrr)
{
    // Setare baudrate
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;

    // TX enable, RX enable
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);

    // frame: 8 biti, 2 stop, fara paritate
    UCSR0C = (1<<USBS0) | (3<<UCSZ00);
}


void USART0_use_stdio(void)
{
    stdout = &USART0_stdout;
}


void USART0_transmit(char data)
{
    while (!(UCSR0A & (1<<UDRE0)));

    UDR0 = data;
}

char USART0_receive()
{
    while (!(UCSR0A & (1<<RXC0)));

    return UDR0;
}

void USART0_print(const char *str)
{
    while (*str != '\0') {
        USART0_transmit(*str++);
    }
}

static int _usart0_putchar(char c, FILE *stream)
{
    if (c == '\n')
        _usart0_putchar('\r', stream);
    USART0_transmit(c);
    return 0;
}