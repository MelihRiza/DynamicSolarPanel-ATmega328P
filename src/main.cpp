#include <stdio.h>
#include <util/delay.h>
#include <avr/io.h>

#include "usart.h"
#include "twi.h"
#include "tsl2561.h"
#include "twi_lcd.h"
#include "adc_S12SD.h"

#define BAUD_RATE 9600

#define STARTING_POS_OX 2250
#define STRATING_POS_OY 1600

#define MIN_LEFT 1000
#define MAX_RIGHT 4000

#define MIN_UP 1000
#define MAX_DOWN 1600

#define SENSIBILITY_LEFT_RIGHT 80
#define STEP_LEFT_RIGHT 20

#define SENSIBILITY_UP_DOWN 80
#define STEP_UP_DOWN 30

#define GO_DOWN 0
#define GO_LEFT 1
#define GO_UP 2
#define GO_RIGHT 3
#define GOOD_POSITION 4

void init_servo1() {
    DDRB |= (1 << DDB1); // PB 2 iesire

    TCCR1A |= (1 << COM1A1) | (1 << WGM11); // Configurare timer1 cu mod de funcționare
                                            // Fast PWM


    // Un servo motor funcționeaza bine intre 50Hz.
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); // Prescaler de 8

    // (16 MHz / 8 = 2MHz; 1 / 0.02 = 50Hz => ICR1 = 2MHz * 0.02 = 40000  
    // Perioada semnal 20ms
    // Frecventa este de 50 Hz
    ICR1 = 40000;
    OCR1A = STARTING_POS_OX; // Poziție inițială servo.
}

void init_servo2() {
    DDRB |= (1 << DDB2); 

    TCCR1A |= (1 << COM1B1);
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);

    ICR1 = 40000;  
    OCR1B = STRATING_POS_OY; 
}

void tsl2561_init(uint8_t address_tsl2561) {
    twi_start(); // Porneste comunicarea twi.

    twi_write(address_tsl2561 << 1); // Trimite adresa senzorului 
                                    // și seteaza bitul de acțiune pe 0
                                    // pentru scriere.

    twi_write(0x00);    // Registrul de comanda al senzorului TSL2561
                        // 0x00 -> urmeaza o scriere în acesta.

    twi_write(0x03); // Activare senzor.

    twi_stop(); // Oprește comunicarea twi.
}

uint32_t tsl2561_read_luminosity(uint8_t address_tsl2561) {
    uint8_t x, t;
    uint16_t combined = 0, infrared_light = 0;

    twi_start();
    twi_write(address_tsl2561 << 1);
    twi_write(0x80 | 0x20 | 0x0C);  // Comanda pentru citirea canalului 0 
    twi_stop();                     // al sensorului.

    _delay_ms(13.7);

    twi_start();
    twi_write((address_tsl2561 << 1) | 1);  // Citire I2C de la senzor.
    twi_read_ack(&t);                       // 2 citiri succesive
    twi_read_nack(&x);                  // pentru a construi lumina combinata
    twi_stop();                 // intercerptată (lumina vizibila + infraroșie)

    combined = x;
    combined <<= 8;     // Compunere lumina combinata.
    combined |= t;

    twi_start();
    twi_write(address_tsl2561 << 1);
    twi_write(0x80 | 0x20 | 0x0E);   // Comanda pentru citirea canalului 1
    twi_stop();                 

    _delay_ms(13.7);

    twi_start();
    twi_write((address_tsl2561 << 1) | 1);  // Citire I2C de la senzor.
    twi_read_ack(&t);                       // La fel ca mai sus, 2 citiri,
    twi_read_nack(&x);                  // dar de aceasta data valoarea
    twi_stop();                         // din canalul 1 este lumina infraroșie.

    infrared_light = x;
    infrared_light <<= 8;   // Compunere lumina infraroșie
    infrared_light |= t;

    // Funcție calcul intensitate luminoasă (Implementare Datasheet TSL2561)
    /**
     * Parametri: gain 0: 1x; 1: 16x | tInt 0:13.7ms, 1:100ms, 2:402ms
     *             combined, infrared, iType T or CS
     **/
    unsigned int lux = CalculateLux(1, 1, combined, infrared_light, 0);

    return lux;
}

char check_difference_base_sensors(double sensorLEFT_lux, double sensorRIGHT_lux) {
    int diff = sensorLEFT_lux - sensorRIGHT_lux;
    if (diff < 0 && (-diff) > SENSIBILITY_LEFT_RIGHT) {
        return 1;
    } else if (diff > 0 && diff > SENSIBILITY_LEFT_RIGHT) {
        return 1;
    }
    return 0;
}

/**
 * Funcție care returneaza direcția în care trebuie mișcat ansamblul prin servo motoare.
 * **/
char check_position(double sensorRIGHT_lux, double sensorUP_lux, double sensorLEFT_lux) {
    if (((sensorUP_lux > sensorRIGHT_lux + SENSIBILITY_UP_DOWN) || (sensorUP_lux > sensorLEFT_lux + SENSIBILITY_UP_DOWN))
            && (!check_difference_base_sensors(sensorLEFT_lux, sensorRIGHT_lux))) {
        return GO_UP;
    } else if (((sensorRIGHT_lux > sensorUP_lux + SENSIBILITY_UP_DOWN) || (sensorLEFT_lux > sensorUP_lux + SENSIBILITY_UP_DOWN))
            && (!check_difference_base_sensors(sensorLEFT_lux, sensorRIGHT_lux))) {
        return GO_DOWN;
    } else if ((sensorRIGHT_lux > sensorUP_lux + SENSIBILITY_LEFT_RIGHT) || (sensorRIGHT_lux > sensorLEFT_lux + SENSIBILITY_LEFT_RIGHT)) {
        return GO_RIGHT;
    } else if ((sensorLEFT_lux > sensorRIGHT_lux + SENSIBILITY_LEFT_RIGHT) || (sensorLEFT_lux > sensorUP_lux + SENSIBILITY_LEFT_RIGHT)) {
        return GO_LEFT;
    }
    return GOOD_POSITION;
}


int main() {

    USART0_init(CALC_USART_UBRR(BAUD_RATE));
    USART0_use_stdio();    

    init_servo1();
    _delay_ms(50);
    init_servo2();
    _delay_ms(50);

    twi_init();
    tsl2561_init(0x29);
    tsl2561_init(0x39);
    tsl2561_init(0x49);
    adc_init_S12SD();

    twi_lcd_init();

    twi_lcd_cmd(0xC0);

    uint16_t uv_value = analog_read_S12SD(0);
    printf("UV value: %d\n", uv_value);

    short current_pos_ox = STARTING_POS_OX;
	short current_pos_oy = STRATING_POS_OY;


    while(1) {
        double sensorRIGHT_lux = tsl2561_read_luminosity(0x39);
        double sensorUP_lux = tsl2561_read_luminosity(0x49);
        double sensorLEFT_lux = tsl2561_read_luminosity(0x29);

        uv_value = analog_read_S12SD(0);

        printf("Sensor 1: %f\n", sensorRIGHT_lux);
        printf("Sensor 2: %f\n", sensorUP_lux);
        printf("Sensor 3: %f\n", sensorLEFT_lux);
        printf("UV value: %d\n", uv_value);
        printf("\n");

        char str_lux1[7];
        char str_lux2[7];
        char str_lux3[7];
        char str_uv[5];
        sprintf(str_lux1, "%.1f", sensorRIGHT_lux);
        sprintf(str_lux2, "%.1f", sensorUP_lux);
        sprintf(str_lux3, "%.1f", sensorLEFT_lux);
        sprintf(str_uv, "%d", uv_value);


        // Afișare pe LCD a valorilor intensităților luminoase receptate
        // si a valorii UV.
        twi_lcd_clear();
        twi_lcd_cmd(0x80);
        twi_lcd_msg((char*)"R:");
        twi_lcd_msg(str_lux1);
        twi_lcd_msg((char*)" U:");
        twi_lcd_msg(str_lux2);
        twi_lcd_cmd(0xC0);
        twi_lcd_msg((char*)"L:");
        twi_lcd_msg(str_lux3);
        
        twi_lcd_msg((char*)" UV:");
        twi_lcd_msg((char*)str_uv);

        switch (check_position(sensorRIGHT_lux, sensorUP_lux, sensorLEFT_lux)) {
            case GO_DOWN:
                printf("DOWN\n");
                if (current_pos_oy < MAX_DOWN) {
                    current_pos_oy += STEP_UP_DOWN;
                }
                OCR1B = current_pos_oy;
                _delay_ms(20);
                break;
            case GO_LEFT:
                printf("LEFT\n");
                if (current_pos_ox > MIN_LEFT) {
                    current_pos_ox -= STEP_LEFT_RIGHT;
                }
                OCR1A = current_pos_ox;
                _delay_ms(20);
                break;
            case GO_UP:
                printf("UP\n");
                if (current_pos_oy > MIN_UP) {
                    current_pos_oy -= STEP_UP_DOWN;
                }
                OCR1B = current_pos_oy;
                _delay_ms(20);
                break;
            case GO_RIGHT:
                printf("RIGHT\n");
                if (current_pos_ox < MAX_RIGHT) {
                    current_pos_ox += STEP_LEFT_RIGHT;
                }
                OCR1A = current_pos_ox;
                _delay_ms(20);
                break;
            case GOOD_POSITION:
                _delay_ms(20);
                break;
        }
        _delay_ms(100);
    }

    return 0;
}
