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
 *  @defgroup USBISS_CMD
 *  USBISS setup commands
 *
 *  @see http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm
 *
 *  @{
 */
#define USBISS_CMD			0x5A	/**< Send Setup command */
#define USBISS_I2C_DIRECT	0x57	/**< I2C direct command */
#define USBISS_ISS_VERSION	0x01	/**< Returns 3 bytes, the module ID (7), firmware version (currently 2), and the current operating mode. */
#define USBISS_SET_ISS_MODE	0x02	/**< Sets operating mode, I2C/SPI/Serial etc. */
#define USBISS_GET_SER_NUM	0x03	/**< Returns the modules unique 8 byte USB serial number. */
#define USBISS_CMD_ACK		0xFF	/**< mode setting frames accepted */
#define USBISS_CMD_NCK		0x00	/**< mode setting frame not accepted */
/** @} */   // USBISS_CMD


/**
 *  @defgroup USBISS_MODE
 *  USBISS setup commands
 *
 *  @see http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm
 *
 *  @{
 */
#define USBISS_IO_MODE			0x00 
#define USBISS_IO_CHANGE		0x10
#define USBISS_I2C_S_20KHZ		0x20
#define USBISS_I2C_S_50KHZ		0x30
#define USBISS_I2C_S_100KHZ		0x40 
#define USBISS_I2C_S_400KHZ 	0x50 
#define USBISS_I2C_H_100KHZ		0x60
#define USBISS_I2C_H_400KHZ		0x70
#define USBISS_I2C_H_1000KHZ	0x80
#define USBISS_SPI_MODE 		0x90
#define USBISS_SERIAL 			0x01
/** @} */   // USBISS_MODE


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
	uint8_t				uint8IsOpen;		/**<  connection to usbiss is open */

} t_usbiss;



/* C++ compatibility */
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus



/** 
 *  @brief mode-to-human
 *
 *  converts USBISS mode to human readable string
 *
 *  @param[in,out]  self                common handle #t_usbiss
 *  @return         string              sstring with human readable transfer mode
 *  @since          June 30, 2023
 *  @author         Andreas Kaeberlein
 */
char *usbiss_mode_to_human(uint8_t mode);



/** 
 *  @brief init
 *
 *  initializes common data structure
 *
 *  @param[in,out]  self                common handle #t_usbiss
 *  @return         int                 state
 *  @retval         0                   OK
 *  @since          June 29, 2023
 *  @author         Andreas Kaeberlein
 */
int usbiss_init( t_usbiss *self );



/** 
 *  @brief verbose
 *
 *  set verbose level
 *
 *  @param[in,out]  self                common handle #t_usbiss
 *  @param[in]  	verbose             0: debug messages disabled, 1: debug messages enabled
 *  @since          June 30, 2023
 *  @author         Andreas Kaeberlein
 */
void usbiss_set_verbose( t_usbiss *self, uint8_t verbose );



/** 
 *  @brief open
 *
 *  open connection to USBISS
 *
 *  @param[in,out]  self                common handle #t_usbiss
 *  @param[in]      port                System Path to UART terminal of USBISS
 *  @param[in]      baud              	desired UART transfer rate
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          June 30, 2023
 *  @author         Andreas Kaeberlein
 */
int usbiss_open( t_usbiss *self, char* port, uint32_t baud );



/** 
 *  @brief open
 *
 *  open connection to USBISS
 *
 *  @param[in,out]  self                common handle #t_usbiss
 *  @param[in]      mode                USBISS transfer mode
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          July 3, 2023
 *  @author         Andreas Kaeberlein
 */
int usbiss_set_mode( t_usbiss *self, const char* mode );






#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __USB_ISS_H
