# **********************************************************************
#  @copyright	: Siemens AG
#  @license		: GPLv3
#  @author		: Andreas Kaeberlein
#  @address		: Clemens-Winkler-Strasse 3, 09116 Chemnitz
#
#  @maintainer	: Andreas Kaeberlein
#  @telephone	: +49 371 4810-2108
#  @email		: andreas.kaeberlein@siemens.com
#
#  @file		: Makefile
#  @date		: 2023-06-28
#
#  @brief		: Build
#				  builds sources with all dependencies
# **********************************************************************



# select compiler
CC = gcc

# set linker
LINKER = gcc

# set compiler flags
ifeq ($(origin CFLAGS), undefined)
  CFLAGS = -c -O -Wall -Wextra -Wimplicit -Wconversion -I . -I ./inc/simple_uart
endif

# linking flags here
ifeq ($(origin LFLAGS), undefined)
  LFLAGS = -Wall -Wextra -Wimplicit -I. -lm
endif


all: usbiss


usbiss: usbiss_main.o simple_uart.o usbiss.o
	$(LINKER) ./obj/usbiss_main.o ./obj/simple_uart.o ./obj/usbiss.o $(LFLAGS) -o ./bin/usbiss

usbiss_main.o: ./usbiss_main.c
	$(CC) $(CFLAGS) ./usbiss_main.c -o ./obj/usbiss_main.o

usbiss.o: ./usbiss.c
	$(CC) $(CFLAGS) ./usbiss.c -o ./obj/usbiss.o

simple_uart.o: ./inc/simple_uart/simple_uart.c
	$(CC) $(CFLAGS) ./inc/simple_uart/simple_uart.c -o ./obj/simple_uart.o

ci: ./usbiss.c
	$(CC) $(CFLAGS) -Werror ./usbiss.c -o ./obj/usbiss.o

clean:
	rm -f ./obj/*.o ./bin/usbiss
