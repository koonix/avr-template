#ifndef SPI_H
#define SPI_H

#include <stdint.h>

/* size of spi buffer string */
#define SPI_BUFFSIZE 30

/* the value that spi_send function returns if spi is busy */
#define SPI_IS_BUSY 1
#define SPI_IS_FREE 0

uint8_t spi_send(const char* string);
void spi_send_task(void);
void spi_finish_transmission(void);
void spi_align(void);
char* mstpcpy(char* dest, const char* src);

#endif /* SPI_H */
