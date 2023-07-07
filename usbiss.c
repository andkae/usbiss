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
 *  @defgroup FALL_THROUGH
 *
 *  @brief Fallthrough
 *
 *  Defines fallthrough only for newer compiler 
 *  avoids warning 'error: empty declaration __attribute__((fallthrough))'
 *
 *  @since  2023-01-07
 *  @see https://stackoverflow.com/questions/45349079/how-to-use-attribute-fallthrough-correctly-in-gcc
 */
#if defined(__GNUC__) && __GNUC__ >= 7
	#define FALL_THROUGH __attribute__ ((fallthrough))
#else
	#define FALL_THROUGH ((void)0)
#endif /* __GNUC__ >= 7 */
/** @} */   // FALL_THROUGH


/**
 *  @defgroup MIN_MAX
 *
 *  @brief MIN/MAX
 *
 *  Calculates MIN/MAX of two numbers
 *
 *  @since  2023-07-07
 *  @see https://stackoverflow.com/questions/3437404/min-and-max-in-c
 */
 #define usbiss_max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

 #define usbiss_min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/** @} */   // MIN_MAX



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
 *  mode-to-str
 *    converts USBISS mode to human readable string
 */
static int usbiss_human_to_mode(const char *str, uint8_t *val)
{
	if ( 0 == strcasecmp(str, "IO_MODE") ) {
		*val = (uint8_t) USBISS_IO_MODE;
		return 0;
	} else if ( 0 == strcasecmp(str, "IO_CHANGE") ) {
		*val = (uint8_t) USBISS_IO_CHANGE;
		return 0;
	} else if ( 0 == strcasecmp(str, "I2C_S_20KHZ") ) {
		*val = (uint8_t) USBISS_I2C_S_20KHZ;
		return 0;
	} else if ( 0 == strcasecmp(str, "I2C_S_50KHZ") ) {
		*val = (uint8_t) USBISS_I2C_S_50KHZ;
		return 0;
	} else if ( 0 == strcasecmp(str, "I2C_S_100KHZ") ) {
		*val = (uint8_t) USBISS_I2C_S_100KHZ;
		return 0;
	} else if ( 0 == strcasecmp(str, "I2C_S_400KHZ") ) {
		*val = (uint8_t) USBISS_I2C_S_400KHZ;
		return 0;
	} else if ( 0 == strcasecmp(str, "I2C_H_100KHZ") ) {
		*val = (uint8_t) USBISS_I2C_H_100KHZ;
		return 0;
	} else if ( 0 == strcasecmp(str, "I2C_H_400KHZ") ) {
		*val = (uint8_t) USBISS_I2C_H_400KHZ;
		return 0;
	} else if ( 0 == strcasecmp(str, "I2C_H_1000KHZ") ) {
		*val = (uint8_t) USBISS_I2C_H_1000KHZ;
		return 0;
	} else if ( 0 == strcasecmp(str, "SPI_MODE") ) {
		*val = (uint8_t) USBISS_SPI_MODE;
		return 0;
	} else if ( 0 == strcasecmp(str, "SERIAL") ) {
		*val = (uint8_t) USBISS_SERIAL;
		return 0;
	}
	/* unsupported input */
	*val = (uint8_t) __UINT8_MAX__;
	return -1;
}



/**
 *  usbiss_is_i2c_mode
 *    check if i2c mode is selected
 */
static int usbiss_is_i2c_mode ( uint8_t mode ) 
{
	/* build mode set frame */
	switch ( mode ) {
		/* I2C Modes */
		case USBISS_I2C_S_20KHZ:	FALL_THROUGH;
		case USBISS_I2C_S_50KHZ:	FALL_THROUGH;
		case USBISS_I2C_S_100KHZ:	FALL_THROUGH;
		case USBISS_I2C_S_400KHZ:	FALL_THROUGH;
		case USBISS_I2C_H_100KHZ:	FALL_THROUGH;
		case USBISS_I2C_H_400KHZ:	FALL_THROUGH;
		case USBISS_I2C_H_1000KHZ:	return 0;
	}	
	return -1;
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
 *  mode-to-human
 *    converts USBISS mode to human readable string
 */
char *usbiss_ero_str(uint8_t error)
{
	static char str[64];
  
	switch(error) {
		case USBISS_ERO_ID1:	strncpy(str, USBISS_ERO_ID1_STR, sizeof(str)); break;
		case USBISS_ERO_ID2:	strncpy(str, USBISS_ERO_ID2_STR, sizeof(str)); break;
		case USBISS_ERO_ID3:	strncpy(str, USBISS_ERO_ID3_STR, sizeof(str)); break;
		case USBISS_ERO_ID4:	strncpy(str, USBISS_ERO_ID4_STR, sizeof(str)); break;
		case USBISS_ERO_ID5:	strncpy(str, USBISS_ERO_ID5_STR, sizeof(str)); break;
		case USBISS_ERO_ID6:	strncpy(str, USBISS_ERO_ID6_STR, sizeof(str)); break;
		case USBISS_ERO_ID7:	strncpy(str, USBISS_ERO_ID7_STR, sizeof(str)); break;
		default:				strncpy(str, "UNKNOWN",       	 sizeof(str)); break;
	}
	return str;
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
	self->uint8IsOpen = 0;	// not open
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
int usbiss_open( t_usbiss *self, char* port, uint32_t baud )
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
	/* mark as open */
	self->uint8IsOpen = 1;
	/* graceful end */
    return 0;
}



/**
 *  usbiss_set_mode
 *    set USBISS transfer mode
 */
int usbiss_set_mode( t_usbiss *self, const char* mode )
{
	/** Variables **/
	char		charBuf[512];	// help buffer for string conversion
	uint8_t		uint8Mode;		// new USBISS mode as opcode
	uint8_t		uint8Wr[16];	// write buffer
	uint8_t		uint8Rd[16];	// read buffer
	int			intWrLen;		// write packet length
	int			intRdLen;
	
	
	/* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
	/* USBISS open? */
	if ( !self->uint8IsOpen ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: USBISS connection not open\n", __FUNCTION__);	
		}
		return -1;
	}
	/* convert to int mode */
	if ( 0 != usbiss_human_to_mode(mode, &uint8Mode) ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: USBISS mode '%s' unsupported\n", __FUNCTION__, mode);	
		}
		return -1;
	}
	/* mode change required? */
	if ( self->uint8Mode == uint8Mode ) {
		return 0;	// desired mode selected
	}	
	/* Build I2C mode set frame */
	if ( 0 == usbiss_is_i2c_mode(uint8Mode) ) {
		uint8Wr[0] = USBISS_CMD;
		uint8Wr[1] = USBISS_SET_ISS_MODE;
		uint8Wr[2] = uint8Mode;
		uint8Wr[3] = 0x04;	// IO_TYPE (see I/O mode above)
		intWrLen = 4;
	/* unsupported Format */
	} else {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Methode to change mode not implemented\n", __FUNCTION__);	
		}
		return -1;
	}
	if ( 0 != self->uint8MsgLevel ) {
		usbiss_uint8_to_asciihex( charBuf, sizeof(charBuf), uint8Wr, (uint32_t) intWrLen );	// convert to ascii hex
		printf("  INFO:%s:REQ: %s\n", __FUNCTION__, charBuf);
	}
	/* set USBISS */
	if ( intWrLen != simple_uart_write(self->uart, uint8Wr, intWrLen) ) {	// request
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: unexpected number of byte written\n", __FUNCTION__);
		}
		return -1;
	}
	intRdLen = simple_uart_read(self->uart, uint8Rd, sizeof(uint8Rd));
	if ( 2 != intRdLen ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, intRdLen);	
		}
		return -1;
	}
	if ( USBISS_CMD_ACK != uint8Rd[0] ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Mode change rejected, reason 0x%02x\n", __FUNCTION__, uint8Rd[1]);	
		}
		return -1;
	}
	/* propagate mode change */
	self->uint8Mode = uint8Mode;
	/* graceful end */
    return 0;
}


/**
 *  usbiss_set_mode
 *    set USBISS transfer mode
 */
int usbiss_i2c_wr( t_usbiss *self, uint8_t adr7, void* data, uint16_t len )
{
	/** Variables **/
	uint8_t			uint8Wr[32];		// write buffer: DIRECT + START + WRITE + 16Bytes + STOP
	uint8_t			uint8Rd[4];			// read buffer
	uint16_t		uint16DataIdx;		// data index of write data
	uint8_t			uint8NumPlByte;		// number of payload bytes
	int				intRdLen;			// number of read bytes from terminal
	char			charBuf[256];		// help buffer for debug outputs
	int				intRet;				// internal return code, allows to send stop bit in case of crash
	uint32_t		uint32Cnt;			// loop count
	
	/* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
	/* empty frame provided */
	if ( 0 == len ) {
		return 0;
	}
	/* USBISS open? */
	if ( !self->uint8IsOpen ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: USBISS connection not open\n", __FUNCTION__);	
		}
		return -1;
	}
	/* I2C mode setted? */
	if ( 0 != usbiss_is_i2c_mode(self->uint8Mode) ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: USBISS is configured for non I2C mode\n", __FUNCTION__);	
		}
		return -1;
	}
	/* First packet START + ADR */
	uint8Wr[0] = USBISS_I2C_DIRECT;	// USBISS direct mode
	uint8Wr[1] = USBISS_I2C_START;	// START-Bit
	uint8Wr[2] = (uint8_t) (USBISS_I2C_WRITE);	// only one address byte written
	uint8Wr[3] = (uint8_t) ((adr7 << 1) | USBISS_I2C_WR);	// i2c address
	if ( 4 != simple_uart_write(self->uart, uint8Wr, 4) ) {	// request
		if ( 0 != self->uint8MsgLevel ) {
			usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, 4);	// convert to ascii
			printf("  ERROR:%s: REQ: %s\n", __FUNCTION__, charBuf);
		}
		return -1;
	}
	if ( 0 != self->uint8MsgLevel ) {
		usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) 4);	// convert to ascii
		printf("  INFO:%s:START: REQ=%s\n", __FUNCTION__, charBuf);
	}
	intRdLen = simple_uart_read(self->uart, uint8Rd, sizeof(uint8Rd));
	if ( 2 != intRdLen ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, intRdLen);	
		}
		return -1;
	}
	if ( USBISS_CMD_ACK != uint8Rd[0] ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Start bit rejected, %s, ero=0x%02x\n", __FUNCTION__, usbiss_ero_str(uint8Rd[1]), uint8Rd[1]);
		}
		return (int) (uint8Rd[1]);	// USBISS error code
	}
	/* Intermideate Packets, DATA */
	intRet = 0;
	uint32Cnt = 0;
	uint16DataIdx = 0;	// next data packet start field
	uint8NumPlByte = (uint8_t) usbiss_min(USBISS_I2C_CHUNK, len);	// calculate max number of bytes to send, -1 through i2c adr
	while ( uint16DataIdx < len ) {
		/* calc payload size */
		uint8NumPlByte = (uint8_t) usbiss_min(USBISS_I2C_CHUNK, len-uint16DataIdx);
		/* assemble packet */
		uint8Wr[0] = USBISS_I2C_DIRECT;	// USBISS direct mode
		uint8Wr[1] = (uint8_t) (USBISS_I2C_WRITE + uint8NumPlByte - 1);
		memcpy(uint8Wr+2, data+uint16DataIdx, uint8NumPlByte);
		/* request i2c packet transfer */
		if ( (uint8NumPlByte + 2) != simple_uart_write(self->uart, uint8Wr, uint8NumPlByte + 2) ) {	// request
			if ( 0 != self->uint8MsgLevel ) {
				usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) (uint8NumPlByte + 2));	// convert to ascii
				printf("  ERROR:%s:Packet=%i:OFS=0x%x:REQ: %s\n", __FUNCTION__, uint32Cnt, uint16DataIdx, charBuf);
			}
			intRet = -1;
			break;
		}
		if ( 0 != self->uint8MsgLevel ) {
			usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) (uint8NumPlByte + 2));	// convert to ascii
			printf("  INFO:%s:PKT%i: REQ=%s\n", __FUNCTION__, uint32Cnt, charBuf);
		}
		/* check response */
		intRdLen = simple_uart_read(self->uart, uint8Rd, sizeof(uint8Rd));
		if ( 2 != intRdLen ) {
			if ( 0 != self->uint8MsgLevel ) {
				printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, intRdLen);	
			}
			intRet = -1;
			break;
		}
		if ( USBISS_CMD_ACK != uint8Rd[0] ) {
			if ( 0 != self->uint8MsgLevel ) {
				printf("  ERROR:%s: Stop bit rejected, %s, ero=0x%02x\n", __FUNCTION__, usbiss_ero_str(uint8Rd[1]), uint8Rd[1]);
			}
			intRet = (int) (uint8Rd[1]);	// USBISS error code
			break;
		}
		/* prepare next cycle */
		uint16DataIdx = (uint16_t) (uint16DataIdx + uint8NumPlByte);	// update data pointer
		uint32Cnt++;
	}
	/* Last Packet Stop Bit */
	uint8Wr[0] = USBISS_I2C_DIRECT;	// USBISS direct mode
	uint8Wr[1] = USBISS_I2C_STOP;	// STOP-Bit
	if ( 2 != simple_uart_write(self->uart, uint8Wr, 2) ) {	// request
		if ( 0 != self->uint8MsgLevel ) {
			usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, 2);	// convert to ascii
			printf("  ERROR:%s: REQ: %s\n", __FUNCTION__, charBuf);
		}
		return -1;
	}
	if ( 0 != self->uint8MsgLevel ) {
		usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) 4);	// convert to ascii
		printf("  INFO:%s:STOP: REQ=%s\n", __FUNCTION__, charBuf);
	}
	intRdLen = simple_uart_read(self->uart, uint8Rd, sizeof(uint8Rd));
	if ( 2 != intRdLen ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, intRdLen);	
		}
		return -1;
	}
	if ( USBISS_CMD_ACK != uint8Rd[0] ) {
		if ( 0 != self->uint8MsgLevel ) {
			printf("  ERROR:%s: Stop bit rejected, %s, ero=0x%02x\n", __FUNCTION__, usbiss_ero_str(uint8Rd[1]), uint8Rd[1]);
		}
		return (int) (uint8Rd[1]);	// USBISS error code
	}
	/* graceful end */
    return intRet;
}



