/*
 * config.h
 * most of the project's configuration is done here.
 * you just have to write the tasks, and put
 * interrupt service routines in main.c.
 */

#ifndef CONFIG_H
#define CONFIG_H

/* this macro allows you to enter task period in milliseconds, regardless of
 * how often task_time_manager runs. */
/* #define MSEC(t) (((TaskTime)t/TASK_TIME_INTERVAL_MSEC)+1) */
#define MSEC(t) ((TaskTime)t * 4)

/* system headers */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "iomacros.h"
#include "tasker.h"


// =======================
// = Task Headers
// =======================
#include "chkadc.h"
#include "blink.h"
#include "spi.h"
#include "testing.h"

// =====================
// = Input/Output
// =====================
/* Put meaningful names on I/O pins */
#define RELAY   D,5
#define MOSI    B,3
#define SCK     B,5
#define SS      B,2

#define ADC_AMBIENT 3
#define ADC_PROXIMA 2

#ifdef MAIN_C
static void io_init(void)
{
    OUTPUT (RELAY);
    OFF    (RELAY);

    OUTPUT (MOSI);
    OUTPUT (SCK);
    OUTPUT (SS);
    OFF    (MOSI);
    OFF    (SCK);
    OFF    (SS);
}
#endif

// =====================
// = Other Macros
// =====================
#define SPI_START  OFF(SS)
#define SPI_END    ON(SS)

#define ADC_START_CONVERSION        SETBIT(ADCSRA,ADSC)
#define ADC_CONVERSION_IN_PROGRESS  GETBIT(ADCSRA, ADSC)


// =====================
// = Other Config
// =====================
#define TASK_TIME_INTERVAL_MSEC 4 /* how often the tsk_task_time_manager runs (in milliseconds) */


#ifdef TASKER_C

// =====================
// = Tasks
// =====================
Task task_array[] = {

/*
 * Define tasks, their initial state and their run period.
 * Period of one means task runs once (won't repeat).
 * You should always set task's counter to 1.
 */

/*    task function             state     period        counter (must be 1)  */
    { spi_send_task,            PAUSED,   1,            1 },
    { spi_finish_transmission,  PAUSED,   1,            1 },
    { chkadc,                   PAUSED,   MSEC(400),    1 },
    { blink,                    PAUSED,   MSEC(400),    1 },
    { blink_start,              PAUSED,   MSEC(200),    1 },
    { adc_sample_runner,        RUNNABLE, MSEC(100),    1 },
    { adc_sample,               PAUSED,   MSEC(1),      1 },
    { sanitize,                 PAUSED,   MSEC(200),    1 },
};


#endif
#ifdef MAIN_C

// =====================
// = Registers
// =====================

/* Define I/O pins as inputs or outputs, and other I/O settings */

#define TCNT0_VALUE 6 /* This is the timer-counter's initial value.
                         The timer-counter will be initialiez to this value
                         on startup (in here) and in timer's overflow interrupt routine. */

static void timer0_init(void)
{
    TCCR0 =
    (0 << CS02) |
    (1 << CS01) |
    (0 << CS00);

    TCNT0 = TCNT0_VALUE;
}

static void timer_interrupts_init(void)
{
    TIMSK =
    (0 << OCIE2)  |
    (0 << TOIE2)  |
    (0 << TICIE1) |
    (0 << OCIE1A) |
    (0 << OCIE1B) |
    (0 << TOIE1)  |
    (1 << TOIE0);
}

/* ADCLAR = adc left adjust; if 8-bit precision (0-256) is enough,
 * set ADCLAR to 1 (to turn on left adjust) and read the ADCH register.
 * otherwise (if you need full 0-1024 precision),
 * set it to 0 and read the ADC register. */
static void adc_init (void)
{
    ADMUX =
    (0 << REFS1) |
    (1 << REFS0) |
    (0 << ADLAR) |
    (0 << MUX3)  |
    (0 << MUX2)  |
    (1 << MUX1)  |
    (0 << MUX0);

    ADCSRA =
    (1 << ADEN)  |
    (0 << ADSC)  |
    (0 << ADFR)  |
    (0 << ADIF)  |
    (0 << ADIE)  |
    (1 << ADPS2) |
    (1 << ADPS1) |
    (0 << ADPS0);
}

static void spi_init(void)
{
    SPCR=
    (0 << SPIE) |
    (1 << SPE)  |
    (0 << DORD) |
    (1 << MSTR) |
    (0 << CPOL) |
    (0 << CPHA) |
    (0 << SPR1) |
    (0 << SPR0);

    SPSR = (0 << SPI2X);
}

static void registers_init(void)
{
    io_init();
    timer0_init();
    adc_init();
    spi_init();
    timer_interrupts_init();
}

ISR(TIMER0_OVF_vect)
{
    TCNT0 = TCNT0_VALUE;
    task_time_manager();
}

#endif /* MAIN_C */
#endif /* CONFIG_H */
