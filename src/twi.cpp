#include <stdio.h>
#include <avr/io.h>

void twi_init() {
	DDRC |= (1 << PC5); // Setez direcția pinului 5 din port C
						// ca pin de ieșire.

	PORTC |= (1 << PC5); // Setez pinul PC5 - semnal înalt
						// SCL setat pentru I2C în starea idle.

	TWCR = 0;

	TWBR = ((F_CPU / 100000) - 16) / 2;  // Rata de transfer

	TWSR &= ~((1 << TWPS0) | (1 << TWPS1));	// Prescaler 1
}

void twi_start(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

void twi_write(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (bit_is_clear(TWCR, TWINT));
}

void twi_stop(void) {
	TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTO);
}

void twi_discover() {
	for (uint8_t address = 0x00; address < 0x7F; address++) {
		printf("Scanning address: %x\r\n", address);
		twi_start();
		twi_write(address << 1 | 1);
		if (((TWSR & 0xF8) == 0x40)) {
			printf("Found device at address: %x\r\n", address);
		}
	}
	twi_stop();
}

void twi_read_ack(uint8_t *data) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while(bit_is_clear(TWCR, TWINT));
    *data = TWDR;
}

void twi_read_nack(uint8_t *data) {
    TWCR = (1 << TWINT) | (1 << TWEN);
	while(bit_is_clear(TWCR, TWINT));
    *data = TWDR;
}