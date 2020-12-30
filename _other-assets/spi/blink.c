#include <stdint.h>
#include "blink.h"
#include "tasker.h"
#include "iomacros.h"
#include "config.h"

#define RELAY_IS_OFF 0

static uint8_t cycles_remaining = 0;
static uint8_t blink_cycle      = RELAY_IS_OFF;
uint8_t blink_count             = 1;

/* turns a 0 into a 1 and vice-versa. */
static inline void flip(uint8_t* bool);

/* this task runs once at startup. */
void blink_start(void)
{
    static uint8_t count=0;
    FLIP(RELAY);
    count++;

    if (count >= 6)
    {
        task_set_state(blink_start, PAUSED);
        task_set_state(chkadc, RUNNABLE);
    }
}

void blink(void)
{
    if (cycles_remaining == 0)
        cycles_remaining = blink_count * 2;

    FLIP(RELAY);
    flip(&blink_cycle);
    cycles_remaining--;

    if (blink_cycle == RELAY_IS_OFF) {
        task_set_period(blink, MSEC(350));
    } else {
        task_set_period(blink, MSEC(50));
    }

    if (cycles_remaining == 0) {
        task_set_state(blink, PAUSED);
    }
}

static inline void flip(uint8_t* bool) {
    *bool = 1 - *bool;
}
