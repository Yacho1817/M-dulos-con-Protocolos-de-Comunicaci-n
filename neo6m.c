#include "MKL25Z4.h"
#include "UART.h"

// Definición de pines UART
#define UART0_RX_PIN 21 // PTB1
#define UART0_TX_PIN 22 // PTB2

// Definir los registros de control del módulo UART0 del KL25Z
#define UART0_BASE 0x4006A000
#define UART0_BDH (*(uint8_t *)(UART0_BASE + 0x00))
#define UART0_BDL (*(uint8_t *)(UART0_BASE + 0x01))
#define UART0_C1 (*(uint8_t *)(UART0_BASE + 0x02))
#define UART0_C2 (*(uint8_t *)(UART0_BASE + 0x03))
#define UART0_S1 (*(uint8_t *)(UART0_BASE + 0x04))
#define UART0_D (*(uint8_t *)(UART0_BASE + 0x07))

// Inicializar el módulo GPS
void GPS_init(uint32_t baud_rate) {
    // Habilitar el reloj para el puerto B y el UART0
    SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
    SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;

    // Configurar los pines RX y TX del UART0
    PORTB_PCR1 = PORT_PCR_MUX(2); // PTB1 como RX
    PORTB_PCR2 = PORT_PCR_MUX(2); // PTB2 como TX

    // Configurar la velocidad de baudios con fórmula estandar del UART
    uint16_t baud_divisor = (SystemCoreClock / (16 * baud_rate)) - 1;
    UART0_BDH = (baud_divisor >> 8) & 0xFF;
    UART0_BDL = baud_divisor & 0xFF;

    // Configurar los registros de control del UART0
    UART0_C1 = 0x00;
    UART0_C2 = UART_C2_TE_MASK | UART_C2_RE_MASK;
}

// Enviar un byte de datos al GPS
void GPS_sendByte(uint8_t data) {
    // Esperar a que el buffer de transmisión esté vacío
    while(!(UART0_S1 & UART_S1_TDRE_MASK));

    // Enviar el byte de datos
    UART0_D = data;
}

// Recibir un byte de datos desde el GPS
uint8_t GPS_receiveByte() {
    // Esperar a que haya datos en el buffer de recepción
    while(!(UART0_S1 & UART_S1_RDRF_MASK));

    // Leer el byte de datos
    return UART0_D;
}