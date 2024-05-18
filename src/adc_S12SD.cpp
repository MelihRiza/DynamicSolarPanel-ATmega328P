#include <stdio.h>
#include <avr/io.h>
#include "adc_S12SD.h"

void adc_init_S12SD(void) {
    // Set ADC prescaler to 128 (don't need higher precision for this project)
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Set ADC reference voltage to AVCC
    ADMUX |= (1 << REFS0);

    // Enable ADC
    ADCSRA |= (1 << ADEN);
}

uint16_t analog_read_S12SD(uint8_t channel) {
    // Force input channel to be between 0 and 7 (as ADC pins are PA0-7)
    channel &= 0b00000111;
    // Clear the old channel value (if any, last 5 bits in ADMUX)
    ADMUX &= ~(0b00011111);
    // Select the new channel in ADMUX
    ADMUX |= channel;
    // Start single conversion
    ADCSRA |= (1 << ADSC);
    // Busy wait for conversion to complete
    while (bit_is_clear(ADCSRA, ADIF));
    // Clear the ADC interrupt flag
    ADCSRA |= (1 << ADIF);
    // Return ADC value
    return (ADC);
}