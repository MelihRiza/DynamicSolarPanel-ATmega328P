#include "twi.h"
#include <util/delay.h>

#define SCL_CLK 400000L
#define BITRATE(TWSR)	((F_CPU/SCL_CLK)-16)/(2*pow(4,(TWSR&((1<<TWPS0)|(1<<TWPS1)))))		


// Trimitere comanda spre LCD
void twi_write_cmd(unsigned char address)
{
	TWDR=address;
	TWCR=(1<<TWINT)|(1<<TWEN);	// Dau clear la intrerupere
	while (!(TWCR & (1<<TWINT)));
	while(TW_STATUS != TW_MT_SLA_ACK);
}

// Trimitere date catre slave
void twi_write_dwr(unsigned char data)
{
	TWDR=data;					// Registru de transmitere
	TWCR=(1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));	
	while(TW_STATUS != TW_MT_DATA_ACK);
}