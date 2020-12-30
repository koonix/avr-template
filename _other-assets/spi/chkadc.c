#include <iomacros.h>

#include <stdint.h>
#include "config.h"
#include "blink.h"
#include "chkadc.h"
#include "iomacros.h"
#include "tasker.h"

void chkadc(void)
{
    static uint8_t back_from_blink = 1;

    /* get task states */
    TaskState blink_start_state = task_get_state(blink_start);
    TaskState blink_state       = task_get_state(blink);
    /* exit this function right here if any blink task is running */
    if (blink_start_state == RUNNABLE ||
        blink_start_state == READY    ||
        blink_state       == RUNNABLE ||
        blink_state       == READY)
        return;

    /* exit if an adc conversion is in progress already */
    if (ADC_CONVERSION_IN_PROGRESS)
        return;

    /* start an adc conversion. it's results become relevant
       * the next time this task runs. */
    if (back_from_blink) {
        ADC_START_CONVERSION;
        task_set_period(chkadc, MSEC(2000));
        back_from_blink = 0;
        return;
    }

    /* set amount of blinks based on adc value,
     * and set the blink task to run in the next period. */
    blink_count = (ADC / 100) + 1;
    task_set_state(blink, READY);
    back_from_blink = 1;
    task_set_period(chkadc, MSEC(100));
}
