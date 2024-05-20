#include <avr/io.h>
#include <util/delay.h>
#include "twi.h"
#include "twi_lcd.c"


#define MODUL_LCD 0x27
#define	WRITE 0
#define READ 1

unsigned char lcd = 0x00;

// Scriere în modul
void module_write(unsigned char x)
{
	twi_start();
	twi_write_cmd((MODUL_LCD << 1)| WRITE);
	twi_write_dwr(x);
	twi_stop();
}

/* Function to Write 4-bit data to LCD */
void twi_lcd_4bit_send(unsigned char x)
{
	unsigned char temp = 0x00;
	
	lcd &= 0x0F;
	temp = (x & 0xF0);
	lcd |= temp;
	lcd |= (0x04);
	module_write(lcd);
	_delay_us(1);
	lcd &= ~(0x04);
	module_write(lcd);
	_delay_us(5);	
	
	temp = ((x & 0x0F)<<4);
	lcd &= 0x0F;				
	lcd |= temp;
	lcd |= (0x04);
	module_write(lcd);
	_delay_us(1);
	lcd &= ~(0x04);
	module_write(lcd);
	_delay_us(5);
	
}

// Scriere in registrul de comanda al LCD-ului
void twi_lcd_cmd(unsigned char x)
{
	lcd = 0x08;
	lcd &= ~(0x01);
	module_write(lcd);
	twi_lcd_4bit_send(x);
	
}

/* Function to Write to LCD Command Register */
void twi_lcd_dwr(unsigned char x)
{
	lcd |= 0x09;			// Lumina de fundal
	module_write(lcd);
	twi_lcd_4bit_send(x);
}

// Trimitere string
void twi_lcd_msg(char *c)
{
	while (*c != '\0')
	twi_lcd_dwr(*c++);
}

// Clear la display
void twi_lcd_clear()
{
	twi_lcd_cmd(0x01);
}

// Initializare LCD cu 4-Bit Mode, setare cursor și pozitionare pe linie si coloană
void twi_lcd_init()
{	
	lcd = 0x04;
	module_write(lcd);
	_delay_us(25);
	
	twi_lcd_cmd(0x03);
	twi_lcd_cmd(0x03);
	twi_lcd_cmd(0x03);
	twi_lcd_cmd(0x02);
	twi_lcd_cmd(0x28);
	twi_lcd_cmd(0x0F);				// Cursor, efect de blink
	twi_lcd_cmd(0x01);				// Clear display LCD
	twi_lcd_cmd(0x06);				// Auto-incrementare cursor la scriere
	twi_lcd_cmd(0x80);				// Selectare linia 1, coloana 1
}
