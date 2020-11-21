/*
 * This file has tasks for using and testing the spi-arduino tasks.
 */

#include <string.h>
#include <util/delay.h>
#include "testing-spi-arduino.h"
#include "iomacros.h"
#include "config.h"
#include "spi-arduino.h"

void test_spi_align(void)
{
    task_set_state(test_spi_align, PAUSED);
    spi_align();
}

void test_spi_string(void)
{
    SPIData data;
    strcpy(data.string, "Ayy Lmao");
    spi_send(STRING, data);
}

void test_spi_number(void)
{
    task_set_period (test_spi_number, MSEC(1000));
    SPIData data;
    data.number = 6969;
    spi_send(NUMBER, data);
}
