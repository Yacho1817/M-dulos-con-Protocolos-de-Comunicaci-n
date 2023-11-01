#ifndef bm280_H
#define bm280_H
#include "MKL25Z4.h"
#include <stdio.h>

void bmp280_init();
int bmp280_read_reg(uint8_t);
void bmp280_read_calib();
void bmp280_read_data();
void bmp280_calculate();

#endif