# Simple AVR Makefile
#
# written by michael cousins (http://github.com/mcous)
# edited by koonix (http://github.com/soystemd)
# released to the public domain

# =#=#=#=#=#=#=#=#=#=
# = Config
# =#=#=#=#=#=#=#=#=#=

# project name (don't put whitespace in there!)
PRJ = mini-drill-press

# where to look for source files and external headers or libraries
# (consisting of .c and .h files)
# e.g. SRCS = ../../EyeToSee ../../YouSART
SRCS = src src/tasks

# where to look for header files or libraries
INCLUDE = /usr/avr/include src src/tasks

# where to put the hex and elf files
REL = release

# directories for clang-format (clangf target)
CLANGF = src src/tasks src/config


# ==================
# = MCU Config
# ==================

# mcu clock	frequency
CLK = 8000000UL


# mcu model
# see -mmcu section of avr-gcc's man-page for all possible MCU options.
MCU = atmega8a


# mcu model for avr-size
# set this to something compatible if avr-size reports unknown device.
SIZEMCU = atmega8


# mcu model for avrdude
# get a list of supported mcus with this command:
#
# $ avrdude -p '?'
#
DUDEMCU = m8


# mcu model macro definition
# see /usr/avr/include/avr/io.h for available options.
MCU_DEF = __AVR_ATmega8A__


# avr programmer (and port if necessary)
# e.g. PRG = usbtiny -or- PRG = arduino -P /dev/tty.usbmodem411
# get a list of supported programmers with this command:
#
# $ avrdude -c '?'
#
PRG = usbasp-clone


# fuse values for avr: low, high, and extended
# see http://www.engbedded.com/fusecalc/ for other MCUs and options
# BE INSANELY CAREFUL WITH THE FUSES! MISTAKE = BRICKING YOUR CHIP.
#
# low
LFU = 0x04
# high
HFU = 0xD9
# extended
EFU =


# ==================
# = Compile Flags
# ==================

# compiler warning/error flags
# handpicked these flags myself. ain't no bugs gonna slip through these amirie!
#
WFLAGS = -std=c99 -pedantic -Wall \
    -Wno-missing-braces -Wextra -Wno-missing-field-initializers -Wformat=2 \
    -Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
    -Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
    -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code \
    -Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls \
    -Wold-style-definition -Wdouble-promotion -Werror \

# other compile options
#
EXTRAFLAGS = -funsigned-char -funsigned-bitfields -fshort-enums \
	-fpack-struct -fno-jump-tables -ftrapv

# flag related to preprocessor
PREPROCFLAGS = -save-temps=cwd


# =#=#=#=#=#=#=#=#=#=
# = End of Config
# =#=#=#=#=#=#=#=#=#=


# ==================
# = Build commands
# ==================

# executables
AVRDUDE = avrdude -c $(PRG) -p $(DUDEMCU)
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE    = avr-size --format=avr --target=elf32-avr --mcu=$(SIZEMCU)
CC      = avr-gcc

# generate list of objects
SRCX     := $(foreach dir, $(SRCS), $(wildcard $(dir)/*.c))
INCX     := $(foreach inc, $(INCLUDE), -I$(inc))
OBJ       = $(SRCX:.c=.o)
HEX = $(REL)/$(PRJ).hex
ELF = $(REL)/$(PRJ).elf

# c flags
CFLAGS = -std=c99 $(WFLAGS) $(EXTRAFLAGS) $(PREPROCFLAGS) -Werror -Os -flto $(DEBUG) \
		 -mmcu=$(MCU) $(INCX) -D$(MCU_DEF)


# ==================
# = Targets
# ==================

.PHONY: test fuse clear clean size init help

# clean and build for release
all: clean build clear

# clean and build for debug
debug: clean set-debug build clear

# set debug flags for analysis of elf file
set-debug:
	$(eval DEBUG := -g -fdebug-prefix-map=/=)

# compile all files
build: $(HEX)

# hex file
$(HEX): $(ELF)
	rm -f $(HEX)
	$(OBJCOPY) -j .text -j .data -O ihex $(ELF) $(HEX)
	$(SIZE) $(ELF)

# elf file
$(ELF): $(OBJ)
	$(CC) $(CFLAGS) -o $(ELF) $(OBJ)

# objects from c files
.c.o:
	@$(CC) $(CFLAGS) -c $< -o $@



# ==================
# = Other Targets
# ==================

clangf:
	@find $(CLANGF) -mindepth 1 -maxdepth 1   \
		-type f -name '*.c' -or -name '*.h' | \
		xargs clang-format -i --verbose

# test programmer connectivity
test:
	$(AVRDUDE) -v

# erase the chip
erase:
	$(AVRDUDE) -e

# flash program to mcu
flash: all
	$(AVRDUDE) -U flash:w:$(HEX):i

# write fuses to mcu
fuse:
	$(AVRDUDE) -U lfuse:w:$(LFU):m -U hfuse:w:$(HFU):m
	#-U efuse:w:$(EFU):m #FOR EXTENDED FUSE#

# generate disassembly files for debugging
disasm: $(ELF)
	$(OBJDUMP) -d $(ELF)

# show size of functions in a sorted manner
fnsize:
	@echo
	@avr-nm -Crtd --size-sort $(ELF)
	@echo

binsize:
	@echo
	$(SIZE) $(ELF)
	@echo

# remove compiled files
clean:
	@rm -f $(HEX) $(ELF) *.o
	@$(foreach dir, $(SRCS), rm -f $(dir)/*.o;)
	@mkdir -p preproc

# remove after-build unnecessary files
clear:
	@rm -f *.o
	@$(foreach dir, $(SRCS), rm -f $(dir)/*.o;)
	@mv *.s *.i *.res preproc/

init:
	@git config core.hooksPath .githooks

prjname:
	@echo $(PRJ)

# help lmao
h: help
help:
	@echo
	@echo "Makefile for AVR toolchain"
	@echo
	@echo "Commands:"
	@echo
	@echo "  make         compile and build all for release"
	@echo "  debug        compile and build all for debug"
	@echo "  test         test programmer connectivity"
	@echo "  flash        build and flash to mcu"
	@echo "  fuse         write fuses to mcu"
	@echo "  disasm       get disassembly for debug"
	@echo "  size         show size of functions in a sorted manner"
