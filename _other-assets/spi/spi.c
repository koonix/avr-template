#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include "spi.h"
#include "config.h"

/* local function prototypes */
static uint8_t is_spi_busy(void);

/* main spi buffer for sending */
char spi_send_buffer[SPI_BUFFSIZE] = {0};

/* this is the user function for all spi communication. */
uint8_t spi_send(const char* string)
{
    if (is_spi_busy() == SPI_IS_BUSY)
        return SPI_IS_BUSY;
    strcpy(spi_send_buffer, string);
    task_set_state(spi_send_task, RUNNABLE);
    return SPI_IS_FREE;
}

/* the scheduled task that sends the string over spi one-byte at a time. */
void spi_send_task(void)
{
    static uint8_t cycle = 0;

    SPI_START;
    SPDR = spi_send_buffer[cycle];

    /* if (cycle >= SPI_BUFFSIZE) { */
    if (spi_send_buffer[cycle] == '\0' || ++cycle >= SPI_BUFFSIZE) {
        cycle = 0;
        task_set_state(spi_send_task, PAUSED);
        task_set_state(spi_finish_transmission, RUNNABLE);
    }
}

/* this runs once after data sending is done. */
void spi_finish_transmission(void)
{
    SPI_END;
    task_set_state(spi_finish_transmission, PAUSED);
}

/* detect if spi is busy transmitting */
static uint8_t is_spi_busy(void)
{
    /* get task states */
    TaskState send = task_get_state(spi_send_task);
    TaskState fnsh = task_get_state(spi_finish_transmission);

    /* exit this function right here if any spi transmission is in progress */
    if (send == RUNNABLE || send == READY || fnsh == RUNNABLE || fnsh == READY)
        return SPI_IS_BUSY;
    return SPI_IS_FREE;
}

/*
 * this function does the exact same job as POSIX's stpcpy.
 *
 * you can use this instead of multiple strcats for faster concatenation
 * of multiple strings and building string out of variables and words,
 * without the use of printf/sprintf/snprintf (which are huge, heavy, slow functions).
 *
 * example usage can be found in testing.c.
 */
char* mstpcpy(char* dest, const char* src)
{
    while ((*dest = *src++))
        ++dest;
    return dest;
}
