
#ifndef neo6m_H
#define neo6m_H
#include "MKL25Z4.h"
#include <stdio.h>

void GPS_init(uint32_t);
void GPS_sendByte(uint8_t);
uint8_t GPS_receiveByte();

#endif