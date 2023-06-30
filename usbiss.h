/***********************************************************************
 @copyright     : Siemens AG
 @license       : GPLv3
 @author        : Andreas Kaeberlein
 @address       : Clemens-Winkler-Strasse 3, 09116 Chemnitz

 @maintainer    : Andreas Kaeberlein
 @telephone     : +49 371 4810-2108
 @email         : andreas.kaeberlein@siemens.com

 @file          : usbiss.h
 @date          : 2023-06-29
 @see			: http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm

 @brief         : USB-ISS driver
                  Driver for USB-ISS:
				    * I2C access functions
***********************************************************************/



// Define Guard
#ifndef __USB_ISS_H
#define __USB_ISS_H


/**
 *  @defgroup USBISS_MISC
 *  help constants for programm
 *  @{
 */
#define USBISS_UART_PATH_LINUX	"/dev/ttyACM0"	/**< default path on Linux platform */
#define USBISS_UART_PATH_WIN	"COM1"			/**< default path on Windows platform */
#define USBISS_UART_BAUD_RATE	115200			/**< default baudrate */
#define USBISS_ID				0x07			/**< module ID */
/** @} */   // USBISS_MISC


/**
 *  @defgroup USBISS_SETUP
 *  USBISS setup commands
 *
 *  @see http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm
 *
 *  @{
 */
#define USBISS_CMD			0x5A	/**< Send Setup command */
#define USBISS_ISS_VERSION	0x01	/**< Returns 3 bytes, the module ID (7), firmware version (currently 2), and the current operating mode. */
#define USBISS_GET_SER_NUM	0x03	/**< Returns the modules unique 8 byte USB serial number. */


/** @} */   // USBISS_SETUP



/**
 *  @typedef t_usbiss
 *
 *  @brief  common handles
 *
 *  handles all related information for usbiss operation
 *
 *  @since  June 29, 2023
 *  @author Andreas Kaeberlein
 */
typedef struct t_usbiss {
    uint8_t             uint8MsgLevel;		/**<  mesage level */
    char				charPort[128];		/**<  path to UART part */
	uint32_t			uint32BaudRate;		/**<  UART baud rate */
	struct simple_uart	*uart;				/**<  handle to simple uart */
	uint8_t				uint8Fw;			/**<  firmware version */
	uint8_t				uint8Mode;			/**<  current mode */
	char				charSerial[10];		/**<  serial number */

} t_usbiss;



/* C++ compatibility */
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus






#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __USB_ISS_H
