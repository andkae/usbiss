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
#include <strings.h>	// stringcasecmp
#include <stdarg.h>     // variable parameter list
/** Custom Libs **/
#include "./inc/simple_uart/simple_uart.h"	// cross platform UART driver
/** self **/
#include "usbiss.h"		// some defs



/**
 *  usbiss_uint8_to_str
 *    convert to ascii hex
 */
static void usbiss_uint8_to_asciihex( char *buf, uint32_t buflen, uint8_t *dat, uint32_t datlen )
{
	
	/* spave available? */
	if ( 0 == buflen ) {
		return;
	}
	/* make empty */
	memset(buf, 0, buflen);
	/* convert */
	for ( uint32_t i = 0; i < datlen; i++ ) {
		if ( !((strlen(buf) + 3) < buflen) ) {	// not enough memory
			break;
		}
		sprintf(buf+3*i, "%02x ", dat[i]);
	}
	buf[strlen(buf)-1] = '\0';	// delete last ' '
}


/**
 *  mode-to-human
 *    converts USBISS mode to human readable string
 */
char *usbiss_mode_to_human(uint8_t mode)
{
	static char str[16];
  
	switch(mode) {
		case USBISS_IO_MODE: 		strncpy(str, "IO_MODE",       sizeof(str)); break;
		case USBISS_IO_CHANGE: 		strncpy(str, "IO_CHANGE",     sizeof(str)); break;
		case USBISS_I2C_S_20KHZ:	strncpy(str, "I2C_S_20KHZ",   sizeof(str)); break;
		case USBISS_I2C_S_50KHZ: 	strncpy(str, "I2C_S_50KHZ",   sizeof(str)); break;
		case USBISS_I2C_S_100KHZ: 	strncpy(str, "I2C_S_100KHZ",  sizeof(str)); break;
		case USBISS_I2C_S_400KHZ: 	strncpy(str, "I2C_S_400KHZ",  sizeof(str)); break;
		case USBISS_I2C_H_100KHZ: 	strncpy(str, "I2C_H_100KHZ",  sizeof(str)); break;
		case USBISS_I2C_H_400KHZ: 	strncpy(str, "I2C_H_400KHZ",  sizeof(str)); break;
		case USBISS_I2C_H_1000KHZ: 	strncpy(str, "I2C_H_1000KHZ", sizeof(str)); break;
		case USBISS_SPI_MODE: 		strncpy(str, "SPI_MODE",      sizeof(str)); break;
		case USBISS_SERIAL: 		strncpy(str, "SERIAL",        sizeof(str)); break;
		default:					strncpy(str, "UNKNOWN",       sizeof(str)); break;
	}
	return str;
}


/**
 *  mode-to-str
 *    converts USBISS mode to human readable string
 */
uint8_t usbiss_human_to_mode(const char *str)
{
	if ( 0 == strcasecmp(str, "IO_MODE") ) {
		return (uint8_t) USBISS_IO_MODE;
	} else if ( 0 == strcasecmp(str, "IO_CHANGE") ) {
		return (uint8_t) USBISS_IO_CHANGE;
	} else if ( 0 == strcasecmp(str, "I2C_S_20KHZ") ) {
		return (uint8_t) USBISS_I2C_S_20KHZ;
	} else if ( 0 == strcasecmp(str, "I2C_S_50KHZ") ) {
		return (uint8_t) USBISS_I2C_S_50KHZ;
	} else if ( 0 == strcasecmp(str, "I2C_S_100KHZ") ) {
		return (uint8_t) USBISS_I2C_S_100KHZ;
	} else if ( 0 == strcasecmp(str, "I2C_S_400KHZ") ) {
		return (uint8_t) USBISS_I2C_S_400KHZ;
	} else if ( 0 == strcasecmp(str, "I2C_H_100KHZ") ) {
		return (uint8_t) USBISS_I2C_H_100KHZ;
	} else if ( 0 == strcasecmp(str, "I2C_H_400KHZ") ) {
		return (uint8_t) USBISS_I2C_H_400KHZ;
	} else if ( 0 == strcasecmp(str, "I2C_H_1000KHZ") ) {
		return (uint8_t) USBISS_I2C_H_1000KHZ;
	} else if ( 0 == strcasecmp(str, "SPI_MODE") ) {
		return (uint8_t) USBISS_SPI_MODE;
	} else if ( 0 == strcasecmp(str, "SERIAL") ) {
		return (uint8_t) USBISS_SERIAL;
	} else {
		return (uint8_t) __UINT8_MAX__;
	}
}


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
    /* graceful end */
    return 0;
}


/**
 *  usbiss_set_verbose
 *    set verbose level
 */
void usbiss_set_verbose( t_usbiss *self, uint8_t verbose )
{
	self->uint8MsgLevel = verbose;
}


/**
 *  usbiss_open
 *    open handle to USBISS and checks ID + serial read
 */
int usbiss_open( t_usbiss *self, char *port, uint32_t baud, uint8_t mode )
{	
	/** variable **/
	uint8_t		uint8Wr[16];	// write buffer
	uint8_t		uint8Rd[16];	// read buffer
	int			intRdLen;		// length of read buffer
	char		charBuf[16];	// string buffer
	
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
			usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, 2);	// convert to ascii
			printf("  ERROR:%s: REQ: %s\n", __FUNCTION__, charBuf);
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
			usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, 2);	// convert to ascii
			printf("  ERROR:%s: REQ: %s\n", __FUNCTION__, charBuf);
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
	memset(self->charSerial, 0, sizeof(self->charSerial));	// make empty string
	strncpy(self->charSerial, (char *) uint8Rd, 8);	// serial has 8 digits
	if ( 0 != self->uint8MsgLevel ) {
		printf("  INFO:%s: Serial=%s\n", __FUNCTION__, self->charSerial);
	}
    /* set mode */
	if ( __UINT8_MAX__ != mode ) {
		uint8Wr[0] = USBISS_CMD;
		uint8Wr[1] = USBISS_SET_ISS_MODE;
		
	}
	
	
	
	
	/* graceful end */
    return 0;
}











