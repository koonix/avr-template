#ifndef SPI_ARDUINO_H
#define SPI_ARDUINO_H

#include <stdint.h>

#define SPI_STRING_SIZE 14

typedef union {
    uint16_t number;
    char string[SPI_STRING_SIZE - 2];
} SPIData;

typedef enum { NUMBER = 0, STRING } SPIDataType;

void spi_send(SPIDataType type, SPIData data);
void spi_send_number(void);
void spi_send_string(void);
void spi_finish_transmission(void);
void spi_mode(void);
void spi_align(void);

#endif /* SPI_ARDUINO_H */
