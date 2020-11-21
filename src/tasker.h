
/*
 * file:     tasker.h
 * author:   Ferenc Nemeth,
 *           Refactored and improved by koonix (https://github.com/soystemd)
 * date:     21 Jul 2018
 *
 * brief: This is a really simple, non-preemptive (co-operative) task scheduler.
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

#ifndef TASKER_H
#define TASKER_H

#include <avr/io.h>
#include <stdint.h>

typedef void (*TaskFunctionPtr)(void);
typedef uint16_t TaskTime;

/* Possible states for tasks. */
/* In the RUNNABLE state the task waits for the timer to put it into READY state. */
/* In the READY state the task is ready to be called and executed in the main function. */
/* In the PAUSED state the task is ignored by the timer and executer. */
typedef enum { RUNNABLE = 0, READY, PAUSED } TaskState;

typedef struct {
    TaskFunctionPtr run; /* This is the function (task) that gets called periodically. */
    TaskState state;     /* The current state of the task. */
    TaskTime period;     /* The period we want to call it. */
    TaskTime counter;    /* Counter, if it reaches the period,
                            then the timer puts it into READY state. */
} Task;

void task_time_manager(void);
void task_runner(void);
void task_set_state(TaskFunctionPtr task, TaskState new_state);
void task_set_period(TaskFunctionPtr task, TaskTime new_period);
void task_set_counter(TaskFunctionPtr task, TaskTime new_counter);
TaskState task_get_state(TaskFunctionPtr task);
TaskTime task_get_period(TaskFunctionPtr task);
TaskTime task_get_counter(TaskFunctionPtr task);

#endif /* TASKER_H */
