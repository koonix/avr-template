/*
 * file:     tasker.c
 * author:   Ferenc Nemeth, Refactored by koonix (https://github.com/soystemd)
 * date:     21 Jul 2018
 * brief:
 *
 * This is a really simple, non-preemptive task scheduler.
 * You can register tasks with their runnable function and the periodic
 * time you want to call them. With a help of a timer the tasks get into READY
 * state after every time period (except if they are PAUSED) and they get
 * called and executed in the main()'s inifinte loop. After they are finished
 * everything starts over. This Scheduler helps you to keep your tasks and
 * timing organized.
 *
 * Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth/
 *
 */

#include <stddef.h>
#include <avr/interrupt.h>
#include "tasker.h"
#define TASKER_C
#include "config.h"

// Static function prototypes:
static uint8_t get_task_index(TaskFunctionPtr task);
static const uint8_t numberof_tasks = sizeof(task_array) / sizeof(task_array[1]);

/* This function keeps track of the tasks' time and puts them into
READY state. This function shall be called in a timer interrupt. */
void task_time_manager(void)
{
    for (uint8_t task_index = 0; task_index < numberof_tasks; task_index++) {

        if (task_array[task_index].state == PAUSED) {
            continue;

        } else if (task_array[task_index].period == 0) {
            task_array[task_index].counter = 1;
            task_array[task_index].state   = READY;

        } else if (task_array[task_index].counter >= task_array[task_index].period) {
            task_array[task_index].counter = 1;
            task_array[task_index].state   = READY;
        }

        else
            task_array[task_index].counter++;
    }
}

/* This function calls the READY tasks and then puts them back into
RUNNABLE state. This function SHALL be called in the infinite loop. */
void task_runner(void)
{
    for (uint8_t task_index = 0; task_index < numberof_tasks; task_index++) {
        /* If it is ready, call it.*/
        if (task_array[task_index].state == READY) {
            if (task_array[task_index].period == 0) {
                cli();
                task_array[task_index].state = PAUSED;
                sei();
            } else {
                cli();
                task_array[task_index].state = RUNNABLE;
                sei();
            }
            task_array[task_index].run();
        }
    }
}

/* Returns the state of the task. */
TaskState task_get_state(TaskFunctionPtr task)
{
    return task_array[get_task_index(task)].state;
}

/* Returns the burst time of the task. */
TaskTime task_get_period(TaskFunctionPtr task)
{
    return task_array[get_task_index(task)].period;
}

/* Returns the current counter value of the task. */
TaskTime task_get_counter(TaskFunctionPtr task)
{
    return task_array[get_task_index(task)].counter;
}

/* Manually changes the task's state. */
void task_set_state(TaskFunctionPtr task, TaskState new_state)
{
    uint8_t task_index           = get_task_index(task);
    task_array[task_index].state = new_state;
    if (new_state == PAUSED)
        task_array[task_index].counter = 1;
}

/* Manually changes the task's burst time. */
void task_set_period(TaskFunctionPtr task, TaskTime new_period)
{
    task_array[get_task_index(task)].period = new_period;
}

/* Manually changes the task's counter. */
void task_set_counter(TaskFunctionPtr task, TaskTime new_counter)
{
    task_array[get_task_index(task)].counter = new_counter;
}

/*
 * Finds a task's function and find it's
 * index in the tasks_array.
 */
static uint8_t get_task_index(TaskFunctionPtr task)
{
    uint8_t task_index = 0;
    for (; task_index < numberof_tasks; task_index++)
        if (task_array[task_index].run == task)
            break;
    return task_index;
}
