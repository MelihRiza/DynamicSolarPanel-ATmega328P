#include <stdio.h>
#include <avr/io.h>
#include "adc_S12SD.h"

void adc_init_S12SD(void) {
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Setare prescaler 128

    ADMUX |= (1 << REFS0);

    ADCSRA |= (1 << ADEN);
}

uint16_t analog_read_S12SD(uint8_t channel) {
    channel &= 0b00000111;
    ADMUX &= ~(0b00011111);
    ADMUX |= channel;
    
    ADCSRA |= (1 << ADSC); // Porneste o conversie
    while (bit_is_clear(ADCSRA, ADIF));

    ADCSRA |= (1 << ADIF); // Clear la intrerupere
    
    return ADC;
}