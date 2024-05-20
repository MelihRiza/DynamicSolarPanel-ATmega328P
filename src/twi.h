#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/twi.h>
#include <stdio.h>
#include "usart.h"


#define F_SCL 100000UL
#define PRESCALER 1
#define TWBR_VAL ((((F_CPU / F_SCL) / PRESCALER) - 16 ) / (2 * PRESCALER))

// Inițializare comunicație I2C
void twi_init(void);

// Start condition
void twi_start(void);

// Scriere byte
void twi_write(uint8_t data);

// Citire byte cu ACK
void twi_read_ack(uint8_t *data);

// Citire byte fara ACK
void twi_read_nack(uint8_t *data);

// Stop condition
void twi_stop(void);

// Cautare în adresele 0 - 127 dispozitive I2C conectate.
void twi_discover(void);
