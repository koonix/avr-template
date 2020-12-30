#include <string.h>
#include <stdlib.h>
#include "testing.h"
#include "config.h"
#include "spi.h"

/* Config */
#define PROXIMITY_SENSORVALUE_DIFF_THRESHOLD   (uint16_t)35
#define HAND_HOLD_ACTIVATION_TIME_100MSEC      (uint8_t )4
#define AVERAGING_SAMPLE_PERIOD_100MSEC        (uint8_t )30
#define AVG_ROLLING_SIZE                       (uint8_t )36
#define FIRSTTIME_QUICKSAMPLE_DURATION_100MSEC (uint8_t )3
#define MINIMUM_RELIABLE_SENSOR_VALUE          (uint16_t)650
#define AVG_DISCARD_DIFF_THRESHOLD             (uint16_t)150

static void check_sanitize(uint16_t adc);
static uint16_t mvavg(uint16_t new_data);
static void adc_select(uint8_t channel);

static uint16_t average = 0;
static uint8_t admux_nomux = 0 | (0 << REFS1) | (1 << REFS0) | (0 << ADLAR);


void adc_sample_runner(void) {
    task_set_state(adc_sample, RUNNABLE);
}
void adc_sample(void)
{
    static uint8_t cycle                = 0;
    static uint8_t avg_cycle_msec       = 0;
    static uint8_t first_time           = 1;
    static char string[SPI_BUFFSIZE]    = {0};
    char str_utoa[6]                    = {0};
    char str_utoa2[6]                   = {0};

    if (ADC_CONVERSION_IN_PROGRESS)
        return;

    cycle++;
    if (cycle == 1)
        ADC_START_CONVERSION;

    else {
        cycle = 0;

        avg_cycle_msec++;

        if (first_time) {
            average = mvavg(ADC);
            if (avg_cycle_msec >= FIRSTTIME_QUICKSAMPLE_DURATION_100MSEC)
                first_time = 0;
        }
        else if (avg_cycle_msec >= AVERAGING_SAMPLE_PERIOD_100MSEC) {
            avg_cycle_msec = 0;
            average = mvavg(ADC);
        }

        mstpcpy(mstpcpy(mstpcpy(mstpcpy(mstpcpy(string,
            "Pxm: "  ), utoa(ADC, str_utoa, 10)),
            ", Avg: "), utoa(average, str_utoa2, 10)), "\n\r");
        spi_send(string);

        if (!first_time)
            check_sanitize(ADC);

        task_set_state(adc_sample, PAUSED);
    }
}


static void check_sanitize(uint16_t adc)
{
    static uint8_t hand_detected_cycles = 0;

    /* if hand is far */
    if(adc > average - PROXIMITY_SENSORVALUE_DIFF_THRESHOLD ||
       adc < (uint16_t)MINIMUM_RELIABLE_SENSOR_VALUE) {
        hand_detected_cycles = 0;
        return;
    }
    /* if hand has been close for just long enough */
    else if (hand_detected_cycles == (uint8_t)HAND_HOLD_ACTIVATION_TIME_100MSEC) {
        hand_detected_cycles++;
        sanitize();
        return;
    }
    else if (hand_detected_cycles > (uint8_t)HAND_HOLD_ACTIVATION_TIME_100MSEC)
        return;

    hand_detected_cycles++;
}


static uint16_t diff(uint16_t v1, uint16_t v2) {
    return v1>v2 ? v1-v2 : v2-v1;
}
static uint16_t mvavg(uint16_t new_data)
{
    static uint16_t data[AVG_ROLLING_SIZE] = {0};
    static uint16_t sum = 0;
    static uint8_t total = 0;
    static uint8_t cycle = 0;

    if(total >= AVG_ROLLING_SIZE && diff(average, new_data) >= AVG_DISCARD_DIFF_THRESHOLD)
        return average;

    data[cycle] = new_data;

    uint8_t old_cycle = cycle + 1;
    if (old_cycle >= AVG_ROLLING_SIZE)
        old_cycle = 0;
    uint16_t old_data = data[old_cycle];

    cycle++;
    if (cycle >= AVG_ROLLING_SIZE)
        cycle = 0;

    if(total < AVG_ROLLING_SIZE)
    {
        total++;
        old_data = 0;
    }

    sum = sum - old_data + new_data;
    uint16_t avg = sum / total;
    return avg;
}

static void adc_select(uint8_t channel)
{
    ADMUX = channel;
    ADMUX |= admux_nomux;
}
