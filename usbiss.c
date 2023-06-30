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
	self->uint32BaudRate = USBISS_UART_BAUD_RATE;	// default baudrate
	self->uint8Fw = 0;		// firmware version
	self->uint8Mode = 0;	// transfer mode
	/* init default path to UART based on platform */
    #if defined(__linux__) || defined(__APPLE__)
		strncpy(self->charPort, USBISS_UART_PATH_LINUX, sizeof(self->charPort));
	#else
		strncpy(self->charPort, USBISS_UART_PATH_WIN, sizeof(self->charPort));
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
	/** variable **/
	uint8_t		uint8Wr[16];	// write buffer
	uint8_t		uint8Rd[16];	// read buffer
	int			intRdLen;		// length of read buffer
	
	/* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
	/* non default path provided? */
	if ( '\0' != port[0] ) {
		if ( strlen(port) > (sizeof(self->charPort) - 1) ) {
			if ( 0 != self->uint8MsgLevel ) {
				printf("  ERROR:%s: UART port path too long.\n", __FUNCTION__);
			}
			return -1;
		}
		strncpy(self->charPort, port, sizeof(self->charPort));
	}
	/* non default baudrate */
	if ( 0 != baud ) {
		switch (baud) {
			case 9600: break;
			case 14400: break;
			case 19200: break;
			case 38400: break;
			case 57600: break;
			case 115200: break;
			default:
				if ( 0 != self->uint8MsgLevel ) {
					printf("  ERROR:%s: Unsupported baudrate %i.\n", __FUNCTION__, baud);
				}
				return -1;
		}
		self->uint32BaudRate = baud;
	}
	/* open uart port */
	self->uart = simple_uart_open(self->charPort, (int) self->uint32BaudRate, "8N1");
	if (!(self->uart)) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: error to open uart port %s with %i baud\n", __FUNCTION__, self->charPort, self->uint32BaudRate);
		}
		return -1;
	}
	/* check module id */
	uint8Wr[0] = USBISS_CMD;
	uint8Wr[1] = USBISS_ISS_VERSION;
	if ( 2 != simple_uart_write(self->uart, uint8Wr, 2) ) {	// request
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Request \n", __FUNCTION__);
		}
		return -1;
	}
	intRdLen = simple_uart_read(self->uart, uint8Rd, sizeof(uint8Rd));
	if ( 3 != intRdLen ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, intRdLen);	
		}
		return -1;
	}
	if ( 0 != self->uint8MsgLevel ) {
		printf("  INFO:%s: ID=0x%02x, FW=0x%02x, MODE=0x%02x\n", __FUNCTION__, uint8Rd[0], uint8Rd[1], uint8Rd[2]);
	}
	if ( USBISS_ID != uint8Rd[0] ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Unexpected module id 0x%02x\n", __FUNCTION__, uint8Rd[0]);	
		}		
		return -1;
	}
	self->uint8Fw = uint8Rd[1];
	self->uint8Mode = uint8Rd[2];
	/* Get Serial number */
	uint8Wr[0] = USBISS_CMD;
	uint8Wr[1] = USBISS_GET_SER_NUM;
	if ( 2 != simple_uart_write(self->uart, uint8Wr, 2) ) {	// request
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Request \n", __FUNCTION__);
		}
		return -1;
	}
	intRdLen = simple_uart_read(self->uart, uint8Rd, sizeof(uint8Rd));
	if ( 8 != intRdLen ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, intRdLen);	
		}
		return -1;
	}
	strncpy(self->charSerial, (char *) uint8Rd, sizeof(self->charSerial)-1);
	self->charSerial[sizeof(self->charSerial)-1] = '\0';
	if ( 0 != self->uint8MsgLevel ) {
		printf("  INFO:%s: Serial=%s\n", __FUNCTION__, self->charSerial);
	}
    /* gracefull end */
    return 0;
}



