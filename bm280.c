#include "MKL25Z4.h"
#include "i2c.h"

// Dirección I2C del sensor BMP280
#define BMP280_I2C_ADDRESS 0x76

// Direcciones de los registros del BMP280
#define BMP280_REG_DIG_T1    0x88
#define BMP280_REG_DIG_T2    0x8A
#define BMP280_REG_DIG_T3    0x8C
#define BMP280_REG_DIG_P1    0x8E
#define BMP280_REG_DIG_P2    0x90
#define BMP280_REG_DIG_P3    0x92
#define BMP280_REG_DIG_P4    0x94
#define BMP280_REG_DIG_P5    0x96
#define BMP280_REG_DIG_P6    0x98
#define BMP280_REG_DIG_P7    0x9A
#define BMP280_REG_DIG_P8    0x9C
#define BMP280_REG_DIG_P9    0x9E
#define BMP280_REG_CHIPID    0xD0
#define BMP280_REG_SOFTRESET 0xE0
#define BMP280_REG_CTRL_MEAS 0xF4
#define BMP280_REG_CONFIG    0xF5
#define BMP280_REG_PRESS_MSB 0xF7
#define BMP280_REG_PRESS_LSB 0xF8
#define BMP280_REG_PRESS_XLSB 0xF9
#define BMP280_REG_TEMP_MSB  0xFA
#define BMP280_REG_TEMP_LSB  0xFB
#define BMP280_REG_TEMP_XLSB 0xFC

// Coeficientes de calibración del BMP280
uint16_t dig_T1;
int16_t  dig_T2;
int16_t  dig_T3;
uint16_t dig_P1;
int16_t  dig_P2;
int16_t  dig_P3;
int16_t  dig_P4;
int16_t  dig_P5;
int16_t  dig_P6;
int16_t  dig_P7;
int16_t  dig_P8;
int16_t  dig_P9;

// Datos del sensor
int32_t pressure;
int32_t temperature;

// Inicializar el BMP280
void bmp280_init() {
    // Habilitar el reloj para el puerto E (I2C)
    SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

    // Configurar los pines SDA y SCL del I2C0
    PORTE_PCR24 = PORT_PCR_MUX(5);  // PTE24 como SDA
    PORTE_PCR25 = PORT_PCR_MUX(5);  // PTE25 como SCL

    // Configurar el módulo I2C0
    I2C0_F  = 0x14; // Configurar la velocidad del bus I2C a 100 kHz
    I2C0_C1 = I2C_C1_IICEN_MASK; // Habilitar el módulo I2C
}

// Leer un registro del BMP280
int bmp280_read_reg(uint8_t reg) {
    // Escribir la dirección del registro en el bus I2C
    I2C0_C1 |= I2C_C1_MST_MASK; // Iniciar la transmisión
    I2C0_C1 |= I2C_C1_TX_MASK; // Cambiar a modo de transmisión
    I2C0_D = BMP280_I2C_ADDRESS << 1; // Escribir la dirección del dispositivo
    while (!(I2C0_S & I2C_S_IICIF_MASK)); // Esperar a que se complete la transmisión
    I2C0_S |= I2C_S_IICIF_MASK; // Limpiar la bandera de interrupción
    I2C0_D = reg; // Escribir la dirección del registro
    while (!(I2C0_S & I2C_S_IICIF_MASK)); // Esperar a que se complete la transmisión
    I2C0_S |= I2C_S_IICIF_MASK; // Limpiar la bandera de interrupción

    // Leer el valor del registro desde el bus I2C
    I2C0_C1 &= ~I2C_C1_TX_MASK; // Cambiar a modo de recepción
    I2C0_C1 |= I2C_C1_TXAK_MASK; // Deshabilitar el ACK
    I2C0_D; // Realizar una lectura para recibir el siguiente byte
    while (!(I2C0_S & I2C_S_IICIF_MASK)); // Esperar a que se complete la transmisión
    I2C0_S |= I2C_S_IICIF_MASK; // Limpiar la bandera de interrupción
    I2C0_C1 &= ~I2C_C1_MST_MASK; // Detener la transmisión

    return I2C0_D; // Devolver el valor del registro
}

// Leer los coeficientes de calibración del BMP280
void bmp280_read_calib() {
    dig_T1 = bmp280_read_reg(BMP280_REG_DIG_T1);
    dig_T2 = bmp280_read_reg(BMP280_REG_DIG_T2);
    dig_T3 = bmp280_read_reg(BMP280_REG_DIG_T3);
    dig_P1 = bmp280_read_reg(BMP280_REG_DIG_P1);
    dig_P2 = bmp280_read_reg(BMP280_REG_DIG_P2);
    dig_P3 = bmp280_read_reg(BMP280_REG_DIG_P3);
    dig_P4 = bmp280_read_reg(BMP280_REG_DIG_P4);
    dig_P5 = bmp280_read_reg(BMP280_REG_DIG_P5);
    dig_P6 = bmp280_read_reg(BMP280_REG_DIG_P6);
    dig_P7 = bmp280_read_reg(BMP280_REG_DIG_P7);
    dig_P8 = bmp280_read_reg(BMP280_REG_DIG_P8);
    dig_P9 = bmp280_read_reg(BMP280_REG_DIG_P9);
}

// Leer los datos de presión y temperatura del BMP280
void bmp280_read_data() {
    uint32_t press_raw, temp_raw;

    // Leer los registros de presión y temperatura
    press_raw = bmp280_read_reg(BMP280_REG_PRESS_MSB);
    press_raw <<= 8;
    press_raw |= bmp280_read_reg(BMP280_REG_PRESS_LSB);
    press_raw <<= 8;
    press_raw |= bmp280_read_reg(BMP280_REG_PRESS_XLSB);
    press_raw >>= 4;

    temp_raw = bmp280_read_reg(BMP280_REG_TEMP_MSB);
    temp_raw <<= 8;
    temp_raw |= bmp280_read_reg(BMP280_REG_TEMP_LSB);
    temp_raw <<= 8;
    temp_raw |= bmp280_read_reg(BMP280_REG_TEMP_XLSB);
    temp_raw >>= 4;

    // Almacenar los datos en las variables globales pressure y temperature
    pressure = press_raw;
    temperature = temp_raw;
}

// Calcular la presión y temperatura a partir de los datos sin procesar
void bmp280_calculate() {
    int var1, var2, var3, t_fine;

    // Calcular la temperatura
    var1 = (((temperature / 8) - (dig_T1 / 2))) * ((dig_T2)) / 2048;
    var2 = ((((temperature / 16) - ((dig_T1)) * ((temperature / 16) - (dig_T1))) / 4096) * (dig_T3)) / 16384;
    t_fine = var1 + var2;
    temperature = ((t_fine * 5 + 128) / 256);

    // Calcular la presión
    var1 = ((t_fine) >> 1) - 64000;
    var2 = (((var1 / 4) * (var1 / 4)) / 2048) * (dig_P6);
    var2 = var2 + ((var1 * (dig_P5)) / 2);
    var2 = (var2 / 4) + ((dig_P4) / 65536);
    var1 = (((dig_P3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8) + (((dig_P2) * var1) / 2)) / 262144;
    var1 = (((32768 + var1)) * (dig_P1)) / 32768;
    if (var1 == 0) {
        return; // Evitar división por cero
    }
    var3 = 1048576 - pressure;
    var3 = (((var3 / 32768) - (var2 / 128)) + (dig_P0)) / var1;
    var1 = (((var3 / 65536) * (var3 / 65536)) * (dig_P9)) >> 8;
    var2 = ((var3 * (dig_P8)) / 65536);
    pressure = var3 + ((var1 + var2 + (dig_P7)) >> 4);
}
