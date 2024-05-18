/*

		TWI MAIN FILE
		twi.c
*/

#include "twi.h"
#include <util/delay.h>

#define SCL_CLK 400000L							//--- SCL for TWI
#define BITRATE(TWSR)	((F_CPU/SCL_CLK)-16)/(2*pow(4,(TWSR&((1<<TWPS0)|(1<<TWPS1)))))		


/* Function to Send Slave Address for Write operation */

void twi_write_cmd(unsigned char address)
{
	TWDR=address;								//--- SLA Address and write instruction
	TWCR=(1<<TWINT)|(1<<TWEN);					//--- Clear TWI interrupt flag,Enable TWI
	while (!(TWCR & (1<<TWINT)));				//--- Wait till complete TWDR byte transmitted to Slave
	while(TW_STATUS != TW_MT_SLA_ACK);			//--- Check for the acknowledgment
}

/* Function to Send Data to Slave Device  */

void twi_write_dwr(unsigned char data)
{
	TWDR=data;									//--- Put data in TWDR
	TWCR=(1<<TWINT)|(1<<TWEN);					//--- Clear TWI interrupt flag,Enable TWI
	while (!(TWCR & (1<<TWINT)));				//--- Wait till complete TWDR byte transmitted to Slave
	while(TW_STATUS != TW_MT_DATA_ACK);			//--- Check for the acknowledgment

}

/* Function to Send Repeated Start Condition */

void twi_repeated_start()
{
	TWCR= (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);		//--- Repeated Start Condition as per Datasheet
	while(!(TWCR & (1<<TWINT)));				//--- Wait till restart condition is transmitted to Slave
	while(TW_STATUS != TW_REP_START);			//--- Check for the acknowledgment
}