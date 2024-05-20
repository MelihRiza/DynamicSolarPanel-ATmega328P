#ifndef USART_H_
#define USART_H_

#include <stdio.h>


#define CALC_USART_UBRR(BAUD) (F_CPU / 16 / (BAUD) - 1)

// Inițializare uart
void USART0_init(unsigned int ubrr);

void USART0_use_stdio(void);

void USART0_transmit(char data);

char USART0_receive();

void USART0_print(const char *str);

#endif // USART_H_