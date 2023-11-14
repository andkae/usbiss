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
 @see           : https://github.com/andkae/usbiss
 @see           : http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm

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
#define USBISS_UART_BAUD_RATE   230400      /**< default baudrate */
#define USBISS_ID               0x07        /**< module ID */
#define USBISS_FW_MIN           (8)         /**< minimal USB-ISS firmware version for proper operation */
#define USBISS_VCP_VID          "04d8"      /**< USB-ISS Virtual COM Port Vendor ID */
#define USBISS_VCP_PID          "ffee"      /**< USB-ISS Virtual COM Port Device/Product ID */
/** @} */   // USBISS_MISC



/**
 *  @defgroup USBISS_CMD
 *  USBISS setup commands
 *
 *  @see http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm
 *
 *  @{
 */
#define USBISS_CMD          0x5A    /**< Send Setup command */
#define USBISS_I2C_DIRECT   0x57    /**< I2C direct command */
#define USBISS_ISS_VERSION  0x01    /**< Returns 3 bytes, the module ID (7), firmware version (currently 2), and the current operating mode. */
#define USBISS_SET_ISS_MODE 0x02    /**< Sets operating mode, I2C/SPI/Serial etc. */
#define USBISS_GET_SER_NUM  0x03    /**< Returns the modules unique 8 byte USB serial number. */
#define USBISS_CMD_ACK      0xFF    /**< mode setting frames accepted */
#define USBISS_CMD_NCK      0x00    /**< mode setting frame not accepted */
/** @} */   // USBISS_CMD



/**
 *  @defgroup USBISS_MODE
 *  USBISS setup commands
 *
 *  @see http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm
 *
 *  @{
 */
#define USBISS_IO_MODE          0x00
#define USBISS_IO_CHANGE        0x10
#define USBISS_I2C_S_20KHZ      0x20
#define USBISS_I2C_S_50KHZ      0x30
#define USBISS_I2C_S_100KHZ     0x40
#define USBISS_I2C_S_400KHZ     0x50
#define USBISS_I2C_H_100KHZ     0x60
#define USBISS_I2C_H_400KHZ     0x70
#define USBISS_I2C_H_1000KHZ    0x80
#define USBISS_SPI_MODE         0x90
#define USBISS_SERIAL           0x01
/** @} */   // USBISS_MODE



/**
 *  @defgroup USBISS_I2C_DIRECT
 *  USBISS setup commands
 *
 *  @see http://www.robot-electronics.co.uk/htm/usb_iss_i2c_tech.htm
 *
 *  @{
 */
#define USBISS_I2C_CHUNK    (16)    /**< Maximum chunk size for packet */
#define USBISS_I2C_START    (0x01)  /**< send start sequence */
#define USBISS_I2C_RESTART  (0x02)  /**< send restart sequence */
#define USBISS_I2C_STOP     (0x03)  /**< send stop sequence */
#define USBISS_I2C_NCK      (0x04)  /**< send NACK after next read */
#define USBISS_I2C_READ     (0x20)  /**< 0x2n reads n+1 bytes, f.e 0x20 reads one byte */
#define USBISS_I2C_WRITE    (0x30)  /**< 0x3n writes n+1 bytes */
/** @} */   // USBISS_I2C_DIRECT



/**
 * @defgroup USBISS_I2C
 *
 * Direction (Read or Write) of I2C communication
 *
 * @{
 */
#define USBISS_I2C_WR   (0x00)      /**<  I2C Write Bit */
#define USBISS_I2C_RD   (0x01)      /**<  I2C Read Bit */
/** @} */



/**
 *  @defgroup USBISS_ERROR
 *  USBISS Error Codes
 *
 *  @see http://www.robot-electronics.co.uk/htm/usb_iss_i2c_tech.htm
 *
 *  @{
 */
#define USBISS_ERO_ID1      (0x01)                                                      /**< Ero 1 */
#define USBISS_ERO_ID1_STR  "No ACK from device"                                        /**< Ero 1 String */
#define USBISS_ERO_ID2      (0x02)                                                      /**< Ero 2 */
#define USBISS_ERO_ID2_STR  "Buffer Overflow, You must limit the frame to < 60 bytes"   /**< Ero 2 String */
#define USBISS_ERO_ID3      (0x03)                                                      /**< Ero 3 */
#define USBISS_ERO_ID3_STR  "Buffer Underflow, More write data was expected than sent"  /**< Ero 3 String */
#define USBISS_ERO_ID4      (0x04)                                                      /**< Ero 4 */
#define USBISS_ERO_ID4_STR  "Unknown command"                                           /**< Ero 4 String */
#define USBISS_ERO_ID5      (0x05)                                                      /**< Ero 5 */
#define USBISS_ERO_ID5_STR  "Unknown command"                                           /**< Ero 5 String */
#define USBISS_ERO_ID6      (0x06)                                                      /**< Ero 6 */
#define USBISS_ERO_ID6_STR  "Internal Error 1"                                          /**< Ero 6 String */
#define USBISS_ERO_ID7      (0x07)                                                      /**< Ero 7 */
#define USBISS_ERO_ID7_STR  "Internal Error 2"                                          /**< Ero 7 String */
/** @} */   // USBISS_ERROR



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
    uint8_t             uint8MsgLevel;      /**<  mesage level */
    char                charPort[128];      /**<  path to UART part */
    uint32_t            uint32BaudRate;     /**<  UART baud rate */
    struct simple_uart  *uart;              /**<  handle to simple uart */
    uint8_t             uint8Fw;            /**<  firmware version */
    uint8_t             uint8Mode;          /**<  current mode */
    char                charSerial[10];     /**<  serial number */
    uint8_t             uint8IsOpen;        /**<  connection to usbiss is open */

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
 *  @brief ero-to-str
 *
 *  converts USBISS error code into human readable string
 *
 *  @param[in,out]  self                common handle #t_usbiss
 *  @return         string              sstring with human readable error string
 *  @since          July 7, 2023
 *  @author         Andreas Kaeberlein
 */
char *usbiss_ero_str(uint8_t error);



/**
 *  @brief list UART ports
 *
 *  Lists system UART ports
 *
 *  @param[in,out]  uart                USB-ISS matching UART ports
 *  @param[in]      len                 maximum string length
 *  @param[in]      sep                 separator between suitable ports
 *  @return         int                 number of USB-ISS matching ports
 *  @since          July 21, 2023
 *  @author         Andreas Kaeberlein
 */
int usbiss_list_uart( char *uart, size_t len, const char sep[] );



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
 *  @param[in]      verbose             0: debug messages disabled, 1: debug messages enabled
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
 *  @param[in]      baud                desired UART transfer rate
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          June 30, 2023
 *  @author         Andreas Kaeberlein
 */
int usbiss_open( t_usbiss *self, char* port, uint32_t baud );



/**
 *  @brief close
 *
 *  close connection to USBISS
 *
 *  @param[in,out]  self                common handle #t_usbiss
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          July 13, 2023
 *  @author         Andreas Kaeberlein
 */
int usbiss_close( t_usbiss *self );



/**
 *  @brief Transfer Mode
 *
 *  set up USB-ISS transfer mode
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



/**
 *  @brief i2c-write
 *
 *  write arbitrary sized data packet to i2c device
 *
 *  @param[in,out]  self                common handle #t_usbiss
 *  @param[in]      adr7                Seven Bit I2C address
 *  @param[in,out]  data                write data
 *  @param[in]      len                 number of bytes in data
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          July 7, 2023
 *  @author         Andreas Kaeberlein
 */
int usbiss_i2c_wr( t_usbiss *self, uint8_t adr7, void* data, size_t len );



/**
 *  @brief i2c-read
 *
 *  read arbitrary sized data packet from i2c device
 *
 *  @param[in,out]  self                common handle #t_usbiss
 *  @param[in]      adr7                Seven Bit I2C address
 *  @param[in,out]  data                read data
 *  @param[in]      len                 number of requested read bytes
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          July 11, 2023
 *  @author         Andreas Kaeberlein
 */
int usbiss_i2c_rd( t_usbiss *self, uint8_t adr7, void* data, size_t len );



/**
 *  @brief i2c-write-read
 *
 *  writes to i2c devices, sents repeated start for direction change and reads from i2c device
 *  write and read data takes places in the same buffer, that meamns the write buffer
 *  will be overwritten by read data.
 *
 *  @param[in,out]  self                common handle #t_usbiss
 *  @param[in]      adr7                Seven Bit I2C address
 *  @param[in,out]  data                read data
 *  @param[in]      wrLen               number of bytes to write to slave
 *  @param[in]      len                 number of requested bytes from slave
 *  @return         int                 state
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          July 11, 2023
 *  @author         Andreas Kaeberlein
 */
int usbiss_i2c_wr_rd( t_usbiss *self, uint8_t adr7, void* data, size_t wrLen, size_t rdLen );


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __USB_ISS_H
