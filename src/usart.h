#ifndef USART_H_
#define USART_H_

#include <stdio.h>

/** Macro to compute the UBRR register from CPU frequency &
 * desired baud rate */
#define CALC_USART_UBRR(BAUD) (F_CPU / 16 / (BAUD) - 1)

/*
 * Initialize the USART peripheral.
 */
void USART0_init(unsigned int ubrr);

/** Replaces the default stdout with our USART implementation. */
void USART0_use_stdio(void);

/*
 * Transmit a byte through the USART.
 *
 * @param `data`: the character to send
 */
void USART0_transmit(char data);

/*
 * Receives a byte from USART.
 *
 * @return the data byte received;
 */
char USART0_receive();

/*
 * Trasmits a null-terminated string through the USART.
 *
 * @param str: the string to send
 */
void USART0_print(const char *str);

#endif // USART_H_