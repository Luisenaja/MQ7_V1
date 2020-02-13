#ifndef MQ7_H_
#define MQ7_H_

#include "stm32f10x.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

void MQ7_init(ADC_TypeDef *_adc);
void preheat(void);
void calibration_ro(void);
void read_CO(uint8_t* _PPM);

#endif