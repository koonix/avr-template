/*
 * This is basically a protocol that I made that allows you to send
 * strings and numbers to an Arduino (via SPI), and then the arduino
 * shows them on your computer.
 *
 * Just flash the included arduino sketch, add this file to your
 * tasks (look at the CONFIG.H-EXAMPLE file), and enjoy.
 */

#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <util/delay.h>
#include <stdint.h>
#include "config.h"
#include "tasker.h"
#include "spi-arduino.h"

static void number_mode(void);
static void string_mode(void);
static uint8_t is_spi_bussy(void);

static uint8_t mode_char;
static uint8_t is_in_number_mode        = 1;
static uint8_t spi_byte[2]              = {0};
static char spi_string[SPI_STRING_SIZE] = {0};

/* this is the user function for all spi communication. */
void spi_send(SPIDataType type, SPIData data)
{
    if (is_spi_bussy())
        return;

    if (type == NUMBER) {
        /* convert integer number into two bytes for sending */
        spi_byte[1] = (data.number >> 8) & 0xFF;
        spi_byte[0] = data.number & 0xFF;
        task_set_state(spi_send_number, RUNNABLE);
    }

    else if (type == STRING) {
        /* pour the passed string into spi_string */
        strcpy(spi_string, data.string);
        strcat(spi_string, "\n\r");
        task_set_state(spi_send_string, RUNNABLE);
    }
}

/* the scheduled function for sending data via spi */
void spi_send_number(void)
{
    static uint8_t cycle = 0;

    /* set spi to number mode if it isn't */
    if (is_in_number_mode != 1) {
        number_mode();
        return;
    }

    SPI_START;
    SPDR = spi_byte[cycle++];

    if (cycle >= 2) {
        cycle = 0;
        memset(spi_byte, 0, sizeof(spi_byte));
        task_set_state(spi_send_number, PAUSED);
        task_set_state(spi_finish_transmission, RUNNABLE);
    }
}

/* the scheduled function for sending data via spi */
void spi_send_string(void)
{
    static uint8_t cycle = 0;

    /* set spi to number mode if it isn't */
    if (is_in_number_mode != 0) {
        string_mode();
        return;
    }

    SPI_START;
    SPDR = spi_string[cycle++];

    if (cycle >= SPI_STRING_SIZE) {
        cycle = 0;
        memset(spi_string, 0, sizeof(spi_string));
        task_set_state(spi_send_string, PAUSED);
        task_set_state(spi_finish_transmission, RUNNABLE);
    }
}

/* this runs once after data sending is done. */
void spi_finish_transmission(void)
{
    SPI_END;
    task_set_state(spi_finish_transmission, PAUSED);
}

/* reset the spi mode to align with the receiver */
void spi_align(void)
{
    if (is_in_number_mode == 1)
        number_mode();
    else if (is_in_number_mode == 0)
        string_mode();
}

/* begin setting spi to number sending mode */
static void number_mode(void)
{
    if (is_in_number_mode > 1)
        return;
    mode_char         = 0x21;
    is_in_number_mode = 3;
    task_set_state(spi_mode, RUNNABLE);
}

/* begin setting spi to string sending mode */
static void string_mode(void)
{
    if (is_in_number_mode > 1)
        return;
    mode_char         = 0x24;
    is_in_number_mode = 2;
    task_set_state(spi_mode, RUNNABLE);
}

/* set spi mode */
void spi_mode(void)
{
    static uint8_t cycle           = 0;
    static char spi_mode_string[3] = {0x07, 0x07, 0x07};

    if (cycle == 0)
        spi_mode_string[2] = mode_char;

    SPI_START;
    SPDR = spi_mode_string[cycle++];

    if (cycle >= 5) {
        cycle = 0;
        task_set_state(spi_mode, PAUSED);
        is_in_number_mode = is_in_number_mode - 2;
    }
}

/* detect if spi is busy transmitting */
static uint8_t is_spi_bussy(void)
{
    /* get task states */
    TaskState numb = task_get_state(spi_send_number);
    TaskState strg = task_get_state(spi_send_string);
    TaskState mode = task_get_state(spi_mode);

    /* exit this function right here if any spi transmission is in progress */
    if (numb == RUNNABLE || numb == READY || strg == RUNNABLE || strg == READY ||
            mode == RUNNABLE || mode == READY)
        return 1;
    return 0;
}
