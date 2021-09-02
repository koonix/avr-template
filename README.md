# avr-template

## Intro

this is the firmware for an AVR atmega8a, which controlls a mini drill press.

it uses a cooperative (aka non-preemptive) task scheduler. that, combined with
the way I set everything up, makes this project extremely simple to be used
as the basis for your own project.

## Initialization

Please run the command below in project's root:

```sh
make init
```

or:

```sh
git config core.hooksPath .githooks
```

In order to add the pre-commit hook to git.

## Customization

Take a look at the Makefile, and configure all the variables.

If you use **vscode**, configure the json file in .vscode.

me? I use vim btw.

If you use **ccls** for C/C++ completion, configure the .ccls file.

If you want to automatically format and beautify your code, use ***clang-format***
and configure the .clang-format file.

## Coding

**tasker.c, tasker.h :**

this is the task scheduler. check out and customize the macro settings in tasker.h.

**registers.c, registers.h :**

all the register settings (input/output, adc, twi, timers/counters, etc.)
are done here.

**iomacros.h :**

some cool macros for reading or setting the IO pins of AVR.

**taskmacros.h :**

each task that you create is given an index, starting from 1. you then
use these indexes to control the tasks later (via tasker functions, ofcourse.)
so in this file, you can set meaningful macros for these indexes.

**tasks directory :**

all the tasks and their headers go in here.
