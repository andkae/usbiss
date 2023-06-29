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
 *  @defgroup HELP_CONSTANTS
 *  help constants for programm
 *  @{
 */
#define HELP_UART_PATH_LINUX	"/dev/ttyACM0"	/**< default path on Linux platform */
#define HELP_UART_PATH_WIN		"COM1"			/**< default path on Windows platform */
#define HELP_UART_BAUD_RATE		9600			/**< default baudrate */
/** @} */   // HELP_CONSTANTS



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
