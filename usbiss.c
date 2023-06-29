/***********************************************************************
 @copyright     : Siemens AG
 @license       : GPLv3
 @author        : Andreas Kaeberlein
 @address       : Clemens-Winkler-Strasse 3, 09116 Chemnitz

 @maintainer    : Andreas Kaeberlein
 @telephone     : +49 371 4810-2108
 @email         : andreas.kaeberlein@siemens.com

 @file          : usbiss.c
 @date          : 2023-06-29
 @see			: http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm

 @brief         : USB-ISS driver
                  Driver for USB-ISS:
				    * I2C access functions
***********************************************************************/



/** Includes **/
/* Standard libs */
#include <stdio.h>      // f.e. printf
#include <stdlib.h>     // defines four variables, several macros,
                        // and various functions for performing
                        // general functions
#include <stdint.h>     // defines fixed data types, like int8_t...
#include <string.h>     // string handling functions
#include <stdarg.h>     // variable parameter list
/** Custom Libs **/
#include "./inc/simple_uart/simple_uart.h"	// cross platform UART driver
/** self **/
#include "usbiss.h"		// some defs



/**
 *  usbiss_init
 *    initializes common data structure
 */
int usbiss_init( t_usbiss *self )
{
	/* init variable */
	self->uint8MsgLevel = 0;	// suppress all outputs
	self->uint32BaudRate = HELP_UART_BAUD_RATE;	// default baudrate
	/* init default path to UART based on platform */
    #if defined(__linux__) || defined(__APPLE__)
		strncpy(self->charPort, HELP_UART_PATH_LINUX, sizeof(self->charPort));
	#else
		strncpy(self->charPort, HELP_UART_PATH_WIN, sizeof(self->charPort));
	#endif
    /* gracefull end */
    return 0;
}



/**
 *  usbiss_init
 *    initializes common data structure
 */
int usbiss_open( t_usbiss *self, char *port, uint32_t baud )
{	
	/* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
	/* non default path provided? */
	if ( '\0' != port[0] ) {
		if ( strlen(port) > (sizeof(self->charPort) - 1) ) {
			if ( 0 != self->uint8MsgLevel ) {
				printf("  ERROR:%s: UART port path too long.\n", __FUNCTION__);
				return -1;
			}
		}
		strncpy(self->charPort, port, sizeof(self->charPort));
	}






}



