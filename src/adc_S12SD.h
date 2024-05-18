#ifndef S12SD_H_
#define S12SD_H_

#include <inttypes.h>

void adc_init_S12SD(void);
uint16_t analog_read_S12SD(uint8_t channel);


#endif // S12SD_H_