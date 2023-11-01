#include "MKL25Z4.h"

// Direcciones de los registros del ADXL345
#define ADXL345_REG_DEVID          0x00
#define ADXL345_REG_THRESH_TAP     0x1D
#define ADXL345_REG_OFSX           0x1E
#define ADXL345_REG_OFSY           0x1F
#define ADXL345_REG_OFSZ           0x20
#define ADXL345_REG_DUR            0x21
#define ADXL345_REG_LATENT         0x22
#define ADXL345_REG_WINDOW         0x23
#define ADXL345_REG_THRESH_ACT     0x24
#define ADXL345_REG_THRESH_INACT   0x25
#define ADXL345_REG_TIME_INACT     0x26
#define ADXL345_REG_ACT_INACT_CTL  0x27
#define ADXL345_REG_THRESH_FF      0x28
#define ADXL345_REG_TIME_FF        0x29
#define ADXL345_REG_TAP_AXES       0x2A
#define ADXL345_REG_ACT_TAP_STATUS 0x2B
#define ADXL345_REG_BW_RATE        0x2C
#define ADXL345_REG_POWER_CTL      0x2D
#define ADXL345_REG_INT_ENABLE     0x2E
#define ADXL345_REG_INT_MAP        0x2F
#define ADXL345_REG_INT_SOURCE     0x30
#define ADXL345_REG_DATA_FORMAT    0x31
#define ADXL345_REG_DATAX0         0x32
#define ADXL345_REG_DATAX1         0x33
#define ADXL345_REG_DATAY0         0x34
#define ADXL345_REG_DATAY1         0x35
#define ADXL345_REG_DATAZ0         0x36
#define ADXL345_REG_DATAZ1         0x37
#define ADXL345_REG_FIFO_CTL       0x38
#define ADXL345_REG_FIFO_STATUS    0x39

// Configuración del ADXL345
#define ADXL345_POWER_ON           0x08
#define ADXL345_AUTO_SLEEP         0x30
#define ADXL345_MEASURE_MODE       0x08

// Inicializar el ADXL345
void adxl345_init() {
    // Habilitar el reloj para el puerto C (SPI)
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;

    // Configurar los pines MOSI, MISO, SCK y CS del SPI0
    PORTC_PCR4 = PORT_PCR_MUX(1);  // PTC4 como MOSI
    PORTC_PCR6 = PORT_PCR_MUX(1);  // PTC6 como MISO
    PORTC_PCR5 = PORT_PCR_MUX(2);  // PTC5 como SCK
    PORTC_PCR0 = PORT_PCR_MUX(1);  // PTC0 como CS

    // Configurar el módulo SPI0
    SPI0_C1 = SPI_C1_MSTR_MASK | SPI_C1_CPHA_MASK | SPI_C1_CPOL_MASK; // Configurar como maestro, modo 3
    SPI0_C2 = 0x00;
    SPI0_BR = 0x01; // Configurar la velocidad del bus SPI

    // Configurar el ADXL345
    adxl345_write_reg(ADXL345_REG_POWER_CTL, 0x00); // Apagar el dispositivo
    adxl345_write_reg(ADXL345_REG_BW_RATE, 0x0A); // Configurar la frecuencia de datos a 100 Hz
    adxl345_write_reg(ADXL345_REG_DATA_FORMAT, 0x00); // Configurar el rango a +-2g
    adxl345_write_reg(ADXL345_REG_POWER_CTL, ADXL345_POWER_ON | ADXL345_MEASURE_MODE); // Encender el dispositivo y habilitar el modo de medición
}

// Escribir un registro del ADXL345
void adxl345_write_reg(uint8_t reg, uint8_t data) {
    // Activar el chip select (CS)
    PTC->PCOR = (1 << 0);

    // Escribir la dirección del registro y el dato
    SPI0_D = reg;
    while (!(SPI0_S & SPI_S_SPTEF_MASK));
    SPI0_D = data;
    while (!(SPI0_S & SPI_S_SPRF_MASK));

    // Desactivar el chip select (CS)
    PTC->PSOR = (1 << 0);
}

// Leer un registro del ADXL345
uint8_t adxl345_read_reg(uint8_t reg) {
    uint8_t data;

    // Activar el chip select (CS)
    PTC->PCOR = (1 << 0);

    // Escribir la dirección del registro y leer el dato
    SPI0_D = reg | 0x80;
    while (!(SPI0_S & SPI_S_SPTEF_MASK));
    data = SPI0_D;
    while (!(SPI0_S & SPI_S_SPRF_MASK));
    data = SPI0_D;

    // Desactivar el chip select (CS)
    PTC->PSOR = (1 << 0);

    return data;
}

// Leer los datos de aceleración del ADXL345
void adxl345_read_accel(int *x, int *y, int *z) {
    uint8_t data[6];

    // Activar el chip select (CS)
    PTC->PCOR = (1 << 0);

    // Escribir la dirección del registro y leer los datos
    SPI0_D = ADXL345_REG_DATAX0 | 0x80 | 0x40; // Modo de lectura múltiple
    while (!(SPI0_S & SPI_S_SPTEF_MASK));
    data[0] = SPI0_D;
    while (!(SPI0_S & SPI_S_SPRF_MASK));
    data[0] = SPI0_D;
    for (int i = 1; i < 6; i++) {
        data[i] = SPI0_D;
        while (!(SPI0_S & SPI_S_SPRF_MASK));
    }

    // Desactivar el chip select (CS)
    PTC->PSOR = (1 << 0);

    // Convertir los datos a formato de 16 bits
    *x = (int)((data[1] / 256) | data[0]);
    *y = (int)((data[3] / 256) | data[2]);
    *z = (int)((data[5] / 256) | data[4]);
}