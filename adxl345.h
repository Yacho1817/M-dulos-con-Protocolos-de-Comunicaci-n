#ifndef adxl345_H
#define adxl345_H
#include "MKL25Z4.h"
#include <stdio.h>

void adxl345_init();
void adxl345_write_reg(uint8_t, uint8_t);
int adxl345_read_reg(uint8_t);
void adxl345_read_accel(uint8_t, uint8_t, uint8_t);

#endif