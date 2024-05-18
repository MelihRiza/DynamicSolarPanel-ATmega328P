// #include <Arduino.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/io.h>

#include "usart.h"
#include "twi.h"
#include "tsl2561.h"
#include "twi_lcd.h"
#include "adc_S12SD.h"

#define BAUD_RATE 9600

#define STARTING_POS_OX 2250  // Middle position for servo (1.5ms pulse width)
#define STRATING_POS_OY 1600

#define MIN_LEFT 1000         // Minimum pulse width (1ms)
#define MAX_RIGHT 4000       // Maximum pulse width (2ms)

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
    DDRB |= (1 << DDB1); // PB1 (pin 9 on Arduino Uno)

    // Configure Timer1 for PWM, mode 14 (Fast PWM with ICR1 as top)
    TCCR1A |= (1 << COM1A1) | (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); // Prescaler = 8

    ICR1 = 20000;  // Set top value for 20ms period (50Hz)
    OCR1A = STARTING_POS_OX; // Initial position
}

void init_servo2() {
    DDRB |= (1 << DDB2); // PB2 (pin 10 on Arduino Uno)

    // Configure Timer1 for PWM, mode 14 (Fast PWM with ICR1 as top)
    TCCR1A |= (1 << COM1B1);
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); // Prescaler = 8

    ICR1 = 20000;  // Set top value for 20ms period (50Hz)
    OCR1B = STRATING_POS_OY; // Initial position
}

void tsl2561_init(uint8_t address_tsl2561) {
    twi_start();
    twi_write(address_tsl2561 << 1);
    twi_write(0x00);
    twi_write(0x03);
    twi_stop();
}

uint32_t tsl2561_read_luminosity(uint8_t address_tsl2561) {
    uint8_t x, t;
    uint16_t combined = 0, infrared_light = 0;

    twi_start();
    twi_write(address_tsl2561 << 1);
    twi_write(0x80 | 0x20 | 0x0C);
    twi_stop();

    _delay_ms(13.7);

    twi_start();
    twi_write((address_tsl2561 << 1) | 1);
    twi_read_ack(&t);
    twi_read_nack(&x);
    twi_stop();

    combined = x;
    combined <<= 8;
    combined |= t;

    twi_start();
    twi_write(address_tsl2561 << 1);
    twi_write(0x80 | 0x20 | 0x0E);
    twi_stop();

    _delay_ms(13.7);

    twi_start();
    twi_write((address_tsl2561 << 1) | 1);
    twi_read_ack(&t);
    twi_read_nack(&x);
    twi_stop();

    infrared_light = x;
    infrared_light <<= 8;
    infrared_light |= t;

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
    twi_lcd_msg((char*)"Manevra e mare");

    twi_lcd_cmd(0xC0);
    twi_lcd_msg((char*)"Manevra e TARE");

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
