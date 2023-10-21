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
 @see           : https://github.com/andkae/usbiss
 @see           : http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm

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
#include <strings.h>    // stringcasecmp
#include <stdarg.h>     // variable parameter list
/** Custom Libs **/
#include "simple_uart.h"    // cross platform UART driver
/** self **/
#include "usbiss.h"     // some defs



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
 *  @brief UART Write
 *
 *  Write to UART port
 *
 *  @param[in,out]  *self               common handle #t_usbiss
 *  @param[in]      data                data array to write
 *  @param[in]      len                 number of bytes to write
 *  @return         uint32_t            number of written bytes
 *  @since          July 18, 2023
 *  @author         Andreas Kaeberlein
 */
static uint32_t usbiss_uart_write( t_usbiss *self, void* data, uint32_t len )
{
    /** Variables **/
    uint32_t    r = 0;

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* UART Write */
    r = (uint32_t) simple_uart_write(self->uart, data, (size_t) len);
    /* flush buffer */
    if ( 0 != simple_uart_flush(self->uart) ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  WARN:%s: flush failed\n", __FUNCTION__);
        }
    }
    /* function finish */
    return r;
}



/**
 *  @brief UART read
 *
 *  Read from UART port
 *
 *  @param[in,out]  *self               common handle #t_usbiss
 *  @param[in]      data                array with read data
 *  @param[in]      len                 requested number of bytes
 *  @return         uint32_t            number of read bytes
 *  @since          July 20, 2023
 *  @author         Andreas Kaeberlein
 */
static uint32_t usbiss_uart_read( t_usbiss *self, void* data, uint32_t len )
{
    /** Variables **/
    uint32_t    r = 0;  // number of recieved bytes
    ssize_t     i = 0;  // help variable for return code of uart

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* check for zero request */
    if ( 0 == len ) {
        return 0;
    }
    /* read until number of required bytes are captured */
    while ( r < len ) {
        i = simple_uart_read(self->uart, data+r, (size_t) (len-r)); // request number of bytes
        if ( i < 0 ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s:READ: UART read failed ero=0x%zx\n", __FUNCTION__, i);
            }
            return r;   // release number of captured bytes until error
        }
        r = r + ((uint32_t) i);
    }
    /* function finish */
    return r;
}



/**
 *  @brief Read data available
 *
 *  Outputs number of availbale read bytes on the UART
 *
 *  @param[in,out]  *self               common handle #t_usbiss
 *  @return         uint32_t            number of available UART read bytes
 *  @since          August 10, 2023
 *  @author         Andreas Kaeberlein
 */
static uint32_t usbiss_uart_read_avail( t_usbiss *self )
{
    /** Variables **/
    int cnt;

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* acquire avail data count */
    cnt = simple_uart_has_data(self->uart);
    if ( cnt < 0 ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Failed to acquire number of available read bytes, ero=0x%x\n", __FUNCTION__, cnt);
        }
        return 0;
    }
    return (uint32_t) cnt;
}



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
        if ( !((strlen(buf) + 3) < buflen) ) {  // not enough memory
            break;
        }
        sprintf(buf+3*i, "%02x ", dat[i]);
    }
    buf[strlen(buf)-1] = '\0';  // delete last ' '
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
 *  @brief Is I2C
 *
 *  Check if USB-ISS has I2C mode
 *
 *  @param[in]      mode                USB-ISS mode
 *  @return         int                 I2C mode active?
 *  @retval         0                   I2C mode
 *  @retval         -1                  Non I2C mode
 *  @since          July 18, 2023
 *  @author         Andreas Kaeberlein
 */
static int usbiss_is_i2c_mode( uint8_t mode )
{
    /* build mode set frame */
    switch ( mode ) {
        /* I2C Modes */
        case USBISS_I2C_S_20KHZ:    FALL_THROUGH;
        case USBISS_I2C_S_50KHZ:    FALL_THROUGH;
        case USBISS_I2C_S_100KHZ:   FALL_THROUGH;
        case USBISS_I2C_S_400KHZ:   FALL_THROUGH;
        case USBISS_I2C_H_100KHZ:   FALL_THROUGH;
        case USBISS_I2C_H_400KHZ:   FALL_THROUGH;
        case USBISS_I2C_H_1000KHZ:  return 0;
    }
    return -1;
}



/**
 *  @brief UART Freeing
 *
 *  Reads until UART recieve queue is empty
 *
 *  @param[in,out]  *self               common handle #t_usbiss
 *  @return         uint32_t            number of read bytes from uart to free the adapter
 *  @since          August 9, 2023
 *  @author         Andreas Kaeberlein
 */
static uint32_t usbiss_uart_free( t_usbiss *self )
{
    /** Variables **/
    uint32_t    cnt;
    uint8_t     dat[4];

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* fetch until no data */
    cnt = 0;
    while ( 0 != usbiss_uart_read_avail(self) ) {
        usbiss_uart_read(self, dat, 1);
        ++cnt;
    }
    /* function finish */
    return cnt;
}



/**
 *  @brief I2C Startbit
 *
 *  sends I2C startbit and slave address with direction
 *
 *  @param[in,out]  *self               common handle #t_usbiss
 *  @param[in]      adr8                7Bit Slave Adress + 1Bit Direction (Read/Write)
 *  @return         int
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          July 10, 2023
 *  @author         Andreas Kaeberlein
 */
static int usbiss_i2c_startbit( t_usbiss *self, uint8_t adr8 )
{
    /** Variables **/
    uint8_t         uint8Wr[4];     // write buffer: DIRECT + START + WRITE + 16Bytes + STOP
    uint8_t         uint8Rd[2];     // read buffer
    uint32_t        uint32RdLen;    // number of read bytes from UART
    char            charBuf[16];    // help buffer for debug outputs

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* Assemble START + ADR packet */
    uint8Wr[0] = USBISS_I2C_DIRECT; // USBISS direct mode
    uint8Wr[1] = USBISS_I2C_START;  // START-Bit
    uint8Wr[2] = (uint8_t) (USBISS_I2C_WRITE);  // only one address byte written
    uint8Wr[3] = adr8;              // i2c address + Direction (Read/Write)
    if ( 4 != usbiss_uart_write(self, uint8Wr, 4) ) {   // request
        if ( 0 != self->uint8MsgLevel ) {
            usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, 4); // convert to ascii
            printf("  ERROR:%s:REQ: %s\n", __FUNCTION__, charBuf);
        }
        return -1;
    }
    if ( 0 != self->uint8MsgLevel ) {
        usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) 4);  // convert to ascii
        printf("  INFO:%s:START:REQ: %s\n", __FUNCTION__, charBuf);
    }
    uint32RdLen = usbiss_uart_read(self, uint8Rd, 2);
    if ( 2 != uint32RdLen ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, uint32RdLen);
        }
        return -1;
    }
    if ( USBISS_CMD_ACK != uint8Rd[0] ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Start bit rejected, %s, ero=0x%02x\n", __FUNCTION__, usbiss_ero_str(uint8Rd[1]), uint8Rd[1]);
        }
        return (int) (uint8Rd[1]);  // USBISS error code, #USBISS_ERROR
    }
    /* function finish */
    return 0;
}



/**
 *  @brief I2C Restartbit
 *
 *  Sends restart bit in established i2c connection, needed for direction change
 *
 *  @param[in,out]  *self               common handle #t_usbiss
 *  @param[in]      adr8                7Bit Slave Adress + 1Bit Direction (Read/Write)
 *  @return         int
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          July 11, 2023
 *  @author         Andreas Kaeberlein
 */
static int usbiss_i2c_restartbit( t_usbiss *self, uint8_t adr8 )
{
    /** Variables **/
    uint8_t         uint8Wr[4];     // write buffer: DIRECT + START + WRITE + 16Bytes + STOP
    uint8_t         uint8Rd[2];     // read buffer
    uint32_t        uint32RdLen;    // number of read bytes from UART
    char            charBuf[16];    // help buffer for debug outputs

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* Assemble START + ADR packet */
    uint8Wr[0] = USBISS_I2C_DIRECT;     // USBISS direct mode
    uint8Wr[1] = USBISS_I2C_RESTART;    // RESTART-BIT
    uint8Wr[2] = (uint8_t) (USBISS_I2C_WRITE);  // only one address byte written
    uint8Wr[3] = adr8;              // i2c address + Direction (Read/Write)
    if ( 4 != usbiss_uart_write(self, uint8Wr, 4) ) {   // request
        if ( 0 != self->uint8MsgLevel ) {
            usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, 4); // convert to ascii
            printf("  ERROR:%s:REQ: %s\n", __FUNCTION__, charBuf);
        }
        return -1;
    }
    if ( 0 != self->uint8MsgLevel ) {
        usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) 4);  // convert to ascii
        printf("  INFO:%s:START:REQ: %s\n", __FUNCTION__, charBuf);
    }
    uint32RdLen = usbiss_uart_read(self, uint8Rd, 2);
    if ( 2 != uint32RdLen ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, uint32RdLen);
        }
        return -1;
    }
    if ( USBISS_CMD_ACK != uint8Rd[0] ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Start bit rejected, %s, ero=0x%02x\n", __FUNCTION__, usbiss_ero_str(uint8Rd[1]), uint8Rd[1]);
        }
        return (int) (uint8Rd[1]);  // USBISS error code, #USBISS_ERROR
    }
    /* function finish */
    return 0;
}



/**
 *  @brief I2C Stopbit
 *
 *  sends I2C stopbit
 *
 *  @param[in,out]  *self               common handle #t_usbiss
 *  @return         int
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          July 10, 2023
 *  @author         Andreas Kaeberlein
 */
static int usbiss_i2c_stopbit( t_usbiss *self )
{
    /** Variables **/
    uint8_t         uint8Wr[2];     // write buffer: DIRECT + START + WRITE + 16Bytes + STOP
    uint8_t         uint8Rd[2];     // read buffer
    uint32_t        uint32RdLen;    // number of read bytes from terminal
    char            charBuf[16];    // help buffer for debug outputs

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    uint8Wr[0] = USBISS_I2C_DIRECT; // USBISS direct mode
    uint8Wr[1] = USBISS_I2C_STOP;   // STOP-Bit
    if ( 2 != usbiss_uart_write(self, uint8Wr, 2) ) {   // request
        if ( 0 != self->uint8MsgLevel ) {
            usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, 2); // convert to ascii
            printf("  ERROR:%s:REQ: %s\n", __FUNCTION__, charBuf);
        }
        return -1;
    }
    if ( 0 != self->uint8MsgLevel ) {
        usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) 4);  // convert to ascii
        printf("  INFO:%s:STOP:REQ: %s\n", __FUNCTION__, charBuf);
    }
    uint32RdLen = usbiss_uart_read(self, uint8Rd, 2);
    if ( 2 != uint32RdLen ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, uint32RdLen);
        }
        return -1;
    }
    if ( USBISS_CMD_ACK != uint8Rd[0] ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Stop bit rejected, %s, ero=0x%02x\n", __FUNCTION__, usbiss_ero_str(uint8Rd[1]), uint8Rd[1]);
        }
        return (int) (uint8Rd[1]);  // USBISS error code, #USBISS_ERROR
    }
    /* function finish */
    return 0;
}



/**
 *  @brief I2C data write
 *
 *  brings only data bytes to I2C lines, Start/Stop bit needs to asserted by dedicated function
 *
 *  @param[in,out]  *self               common handle #t_usbiss
 *  @param[in]      data                data array
 *  @param[in]      len                 number of bytes in data
 *  @return         int
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          July 10, 2023
 *  @author         Andreas Kaeberlein
 */
static int usbiss_i2c_data_wr ( t_usbiss *self, void* data, size_t len )
{
    /** Variables **/
    uint8_t     uint8Wr[32];    // write buffer: DIRECT + START + WRITE + 16Bytes + STOP
    uint8_t     uint8Rd[4];     // read buffer
    size_t      bytesPend;      // number of pending bytes
    size_t      dataOfs;        // byte offset of current data
    uint8_t     uint8Chunk;     // number of data bytes at current chunk
    uint32_t    uint32RdLen;    // number of read bytes from terminal
    char        charBuf[256];   // help buffer for debug outputs
    int         intRet;         // internal return code, allows to send stop bit in case of crash
    size_t      iter;           // loop count

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* empty frame provided */
    if ( 0 == len ) {
        return 0;
    }
    /* Data Packets to line */
    intRet = 0;
    iter = 0;
    dataOfs = 0;    // next data packet start field
    bytesPend = len;    // all bytes pending
    while ( bytesPend > 0 ) {
        /* calc payload size */
        uint8Chunk = (uint8_t) usbiss_min((size_t) USBISS_I2C_CHUNK, bytesPend);    // calculate max number of bytes to send, -1 through i2c adr
        /* assemble packet */
        uint8Wr[0] = USBISS_I2C_DIRECT; // USBISS direct mode
        uint8Wr[1] = (uint8_t) (USBISS_I2C_WRITE + uint8Chunk - 1);
        memcpy(uint8Wr+2, data+dataOfs, uint8Chunk);
        /* request i2c packet transfer */
        if ( ((uint32_t) (uint8Chunk + 2)) != usbiss_uart_write(self, uint8Wr, (uint32_t) (uint8Chunk + 2)) ) { // request
            if ( 0 != self->uint8MsgLevel ) {
                usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) (uint8Chunk + 2));   // convert to ascii
                printf("  ERROR:%s:PKG=%zi:OFS=0x%zx:REQ: %s\n", __FUNCTION__, iter, dataOfs, charBuf);
            }
            intRet = -1;
            break;
        }
        if ( 0 != self->uint8MsgLevel ) {
            usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) (uint8Chunk + 2));   // convert to ascii
            printf("  INFO:%s:PKG=%zi:OFS=0x%zx:REQ: %s\n", __FUNCTION__, iter, dataOfs, charBuf);
        }
        /* check response */
        uint32RdLen = usbiss_uart_read(self, uint8Rd, 2);
        if ( 2 != uint32RdLen ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s:PKG=%zi:RSP: Unexpected number of %i bytes received\n", __FUNCTION__, iter, uint32RdLen);
            }
            intRet = -1;
            break;
        }
        if ( USBISS_CMD_ACK != uint8Rd[0] ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s:PKG=%zi: packet rejected, %s, ero=0x%02x\n", __FUNCTION__, iter, usbiss_ero_str(uint8Rd[1]), uint8Rd[1]);
            }
            intRet = (int) (uint8Rd[1]);    // USBISS error code
            break;
        }
        /* prepare next cycle */
        dataOfs = dataOfs + ((size_t) uint8Chunk);  // update data pointer
        bytesPend = bytesPend - uint8Chunk;         // update count of pending bytes
        iter++;
    }
    /* finish function */
    return intRet;
}



/**
 *  @brief I2C data read
 *
 *  brings only data bytes to I2C lines, Start/Stop bit needs to asserted by dedicated function
 *
 *  @param[in,out]  *self               common handle #t_usbiss
 *  @param[out]     data                data array
 *  @param[in,out]  len                 number of requested bytes as in, and recieved bytes as out
 *  @return         int
 *  @retval         0                   OK
 *  @retval         -1                  FAIL
 *  @since          July 10, 2023
 *  @author         Andreas Kaeberlein
 */
static int usbiss_i2c_data_rd ( t_usbiss *self, void* data, size_t len )
{
    /** variables **/
    uint8_t     uint8Wr[4];     // write buffer: DIRECT + START + WRITE + 16Bytes + STOP
    uint8_t     uint8Rd[32];    // read buffer
    size_t      ackBytesPend;   // number of pending bytes with ACK
    size_t      dataOfs;        // data pointer in array
    uint8_t     uint8Chunk;     // number of data bytes at current chunk
    uint32_t    uint32RdLen;    // number of read bytes from terminal
    size_t      iter;           // loop count
    char        charBuf[256];   // help buffer for debug outputs

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* empty frame provided */
    if ( 0 == len ) {
        return 0;
    }
    /* request ACK Bytes, last byte need NCK */
    dataOfs = 0;
    iter = 0;
    if ( len > 1 ) {
        ackBytesPend = len - 1;
        while ( ackBytesPend > 0 ) {
            /* calc number of requested bytes size */
            uint8Chunk = (uint8_t) usbiss_min((size_t) USBISS_I2C_CHUNK, ackBytesPend); // calculate max number of bytes to send, -1 through i2c adr
            /* assemble request */
            uint8Wr[0] = USBISS_I2C_DIRECT; // USBISS direct mode
            uint8Wr[1] = (uint8_t) (USBISS_I2C_READ + uint8Chunk - 1);
            /* request i2c packet transfer */
            if ( 2 != usbiss_uart_write(self, uint8Wr, 2) ) {   // request
                if ( 0 != self->uint8MsgLevel ) {
                    usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) 2);  // convert to ascii
                    printf("  ERROR:%s:PKG=%zi:OFS=0x%zx:REQ: %s\n", __FUNCTION__, iter, dataOfs, charBuf);
                }
                return -1;
            }
            /* UART data read: get status bytes */
            uint32RdLen = usbiss_uart_read(self, uint8Rd, 2);
            if ( 2 != uint32RdLen ) {
                if ( 0 != self->uint8MsgLevel ) {
                    printf("  ERROR:%s:PKG=%zi:RSP:STATUS: Unexpected number of %i instead %i bytes received\n", __FUNCTION__, iter, uint32RdLen, 2);
                }
                return -1;
            }
            /* check response byte */
            if ( USBISS_CMD_ACK != uint8Rd[0] ) {
                if ( 0 != self->uint8MsgLevel ) {
                    printf("  ERROR:%s:PKG=%zi: packet rejected, %s, ero=0x%02x\n", __FUNCTION__, iter, usbiss_ero_str(uint8Rd[1]), uint8Rd[1]);
                }
                return (int) (uint8Rd[1]);  // USBISS error code
            }
            /* check byte count */
            if ( uint8Chunk != uint8Rd[1] ) {
                if ( 0 != self->uint8MsgLevel ) {
                    printf("  ERROR:%s:PKG=%zi: wrong data count recieved, exp=%i, is=%i\n", __FUNCTION__, iter, uint8Chunk, uint8Rd[1]);
                }
                usbiss_uart_free(self); // clean buffer
                return -1;
            }
            /* UART data read: get payload with data */
            uint32RdLen = usbiss_uart_read(self, uint8Rd+2, (uint32_t) (uint8Chunk));
            if ( ((uint32_t) uint8Chunk) != uint32RdLen ) {
                if ( 0 != self->uint8MsgLevel ) {
                    printf("  ERROR:%s:PKG=%zi:RSP:DATA: Unexpected number of %i instead %i bytes received\n", __FUNCTION__, iter, uint32RdLen, uint8Chunk);
                }
                return -1;
            }
            /* user message */
            if ( 0 != self->uint8MsgLevel ) {
                usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Rd, (uint32_t) (uint8Chunk + 2));   // convert to ascii
                printf("  INFO:%s:PKG=%zi:OFS=0x%zx:RSP: %s\n", __FUNCTION__, iter, dataOfs, charBuf);
            }
            /* fill data in result variable */
            memcpy(data+dataOfs, uint8Rd+2, uint8Chunk);
            /* prepare next cycle */
            dataOfs = dataOfs + ((size_t) uint8Chunk);  // update data pointer
            ackBytesPend = ackBytesPend - uint8Chunk;   // update count of pending bytes
            iter++;
        }
    }
    /* request last byte, NCK */
    uint8Wr[0] = USBISS_I2C_DIRECT; // USBISS direct mode
    uint8Wr[1] = USBISS_I2C_NCK;    // read with NCK
    uint8Wr[2] = USBISS_I2C_READ;   // read one byte
    /* issues transfer */
    if ( 3 != usbiss_uart_write(self, uint8Wr, 3) ) {   // request
        if ( 0 != self->uint8MsgLevel ) {
            usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, (uint32_t) 3);  // convert to ascii
            printf("  ERROR:%s:PKG=%zi:OFS=0x%zx:REQ: %s\n", __FUNCTION__, iter, dataOfs, charBuf);
        }
        return -1;
    }
    /* UART data read: get status bytes */
    uint32RdLen = usbiss_uart_read(self, uint8Rd, 2);
    if ( 2 != uint32RdLen ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s:PKG=%zi:RSP:STATUS Unexpected number of %i instead %i bytes received\n", __FUNCTION__, iter, uint32RdLen, 2);
        }
        return -1;
    }
    /* check response byte */
    if ( USBISS_CMD_ACK != uint8Rd[0] ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s:PKG=%zi: packet rejected, %s, ero=0x%02x\n", __FUNCTION__, iter, usbiss_ero_str(uint8Rd[1]), uint8Rd[1]);
        }
        return (int) (uint8Rd[1]);  // USBISS error code
    }
    /* check byte count */
    if ( 1 != uint8Rd[1] ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s:PKG=%zi: wrong data count recieved, exp=%i, is=%i\n", __FUNCTION__, iter, 1, uint8Rd[1]);
        }
        usbiss_uart_free(self); // clean buffer
        return -1;
    }
    /* UART data read: get payload with data */
    uint32RdLen = usbiss_uart_read(self, uint8Rd+2, 1);
    if ( 1 != uint32RdLen ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s:PKG=%zi:RSP:DATA: Unexpected number of %i instead %i bytes received\n", __FUNCTION__, iter, uint32RdLen, 1);
        }
        return -1;
    }
    /* user message */
    if ( 0 != self->uint8MsgLevel ) {
        usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Rd, 3); // convert to ascii
        printf("  INFO:%s:PKG=%zi:OFS=0x%zx:RSP: %s\n", __FUNCTION__, iter, dataOfs, charBuf);
    }
    /* fill data in result variable */
    memcpy(data+dataOfs, uint8Rd+2, 1);
    /* finish function */
    return 0;
}



/**
 *  mode-to-human
 *    converts USBISS mode to human readable string
 */
char *usbiss_mode_to_human(uint8_t mode)
{
    static char str[16];

    switch(mode) {
        case USBISS_IO_MODE:        strncpy(str, "IO_MODE",       sizeof(str)); break;
        case USBISS_IO_CHANGE:      strncpy(str, "IO_CHANGE",     sizeof(str)); break;
        case USBISS_I2C_S_20KHZ:    strncpy(str, "I2C_S_20KHZ",   sizeof(str)); break;
        case USBISS_I2C_S_50KHZ:    strncpy(str, "I2C_S_50KHZ",   sizeof(str)); break;
        case USBISS_I2C_S_100KHZ:   strncpy(str, "I2C_S_100KHZ",  sizeof(str)); break;
        case USBISS_I2C_S_400KHZ:   strncpy(str, "I2C_S_400KHZ",  sizeof(str)); break;
        case USBISS_I2C_H_100KHZ:   strncpy(str, "I2C_H_100KHZ",  sizeof(str)); break;
        case USBISS_I2C_H_400KHZ:   strncpy(str, "I2C_H_400KHZ",  sizeof(str)); break;
        case USBISS_I2C_H_1000KHZ:  strncpy(str, "I2C_H_1000KHZ", sizeof(str)); break;
        case USBISS_SPI_MODE:       strncpy(str, "SPI_MODE",      sizeof(str)); break;
        case USBISS_SERIAL:         strncpy(str, "SERIAL",        sizeof(str)); break;
        default:                    strncpy(str, "UNKNOWN",       sizeof(str)); break;
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
        case USBISS_ERO_ID1:    strncpy(str, USBISS_ERO_ID1_STR, sizeof(str)); break;
        case USBISS_ERO_ID2:    strncpy(str, USBISS_ERO_ID2_STR, sizeof(str)); break;
        case USBISS_ERO_ID3:    strncpy(str, USBISS_ERO_ID3_STR, sizeof(str)); break;
        case USBISS_ERO_ID4:    strncpy(str, USBISS_ERO_ID4_STR, sizeof(str)); break;
        case USBISS_ERO_ID5:    strncpy(str, USBISS_ERO_ID5_STR, sizeof(str)); break;
        case USBISS_ERO_ID6:    strncpy(str, USBISS_ERO_ID6_STR, sizeof(str)); break;
        case USBISS_ERO_ID7:    strncpy(str, USBISS_ERO_ID7_STR, sizeof(str)); break;
        default:                strncpy(str, "UNKNOWN",          sizeof(str)); break;
    }
    return str;
}



/**
 *  usbiss_list_uart
 *    List suitable port for USB-ISS connection
 */
int usbiss_list_uart( char *str, size_t len, const char sep[] )
{
    /** Variables **/
    char    **names;
    ssize_t numTTY;
    int     numUsbIssTTY = 0;

    /* list UART ports and build */
    numTTY = simple_uart_list(&names);  // get UART ports from system
    str[0] = '\0';
    for (ssize_t i = 0; i < numTTY; i++) {
        #if defined(__linux__) || defined(__APPLE__)
            if ( NULL != strstr(names[i], "ttyACM") ) {
        #endif
            /* make array to simple string */
            snprintf(str+strlen(str), len-strlen(str), "%s%s", names[i], sep);
            ++numUsbIssTTY;
        #if defined(__linux__) || defined(__APPLE__)
            }
        #endif
    }
    if ( strlen(sep) < strlen(str) ) {
        str[strlen(str)-strlen(sep)] = '\0';    // cut separator
    }
    /* finish */
    return numUsbIssTTY;
}



/**
 *  usbiss_init
 *    initializes common data structure
 */
int usbiss_init( t_usbiss *self )
{
    /* init variable */
    self->uint8MsgLevel = 0;    // suppress all outputs
    self->uint32BaudRate = USBISS_UART_BAUD_RATE;   // default baudrate
    self->uint8Fw = 0;      // firmware version
    self->uint8Mode = 0;    // transfer mode
    self->uint8IsOpen = 0;  // not open
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
    uint8_t     uint8Wr[16];        // write buffer
    uint8_t     uint8Rd[16];        // read buffer
    uint32_t    uint32RdLen;        // length of read buffer
    char        charBuf[16];        // string buffer
    char        charUartAuto[256];  // autodedect USB-ISS uart port
    char*       charPtrFirstUart;   // pointer to first UART port
    int         intNumUarts;        // number of UART ports in system

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* no autodetect path provided? */
    if ( '\0' != port[0] ) {
        if ( strlen(port) > (sizeof(self->charPort) - 1) ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: UART port path too long.\n", __FUNCTION__);
            }
            return -1;
        }
        strncpy(self->charPort, port, sizeof(self->charPort));
    } else {    // consider first found UART device in system as USB-ISS uart port
        /* get available USB-ISS suitable ports from system */
        intNumUarts = usbiss_list_uart(charUartAuto, sizeof(charUartAuto), " ");
        /* select Port */
        switch (intNumUarts) {
            /* no UART */
            case 0:
                if ( 0 != self->uint8MsgLevel ) {
                    printf("  ERROR:%s: no USB-ISS suitable port found.\n", __FUNCTION__);
                }
                return -1;
                break;
            /* one UART */
            case 1:
                if ( strlen(charUartAuto) > (sizeof(self->charPort) - 1) ) {
                    if ( 0 != self->uint8MsgLevel ) {
                        printf("  ERROR:%s: UART port path too long.\n", __FUNCTION__);
                    }
                    return -1;
                }
                strncpy(self->charPort, charUartAuto, sizeof(self->charPort));  // copy uart path
                break;
            /* multiple UART ports */
            default:
                charPtrFirstUart = strtok(charUartAuto, " ");   // split list of UART ports and choose first
                if ( strlen(charPtrFirstUart) > (sizeof(self->charPort) - 1) ) {
                    if ( 0 != self->uint8MsgLevel ) {
                        printf("  ERROR:%s: UART port path too long.\n", __FUNCTION__);
                    }
                    return -1;
                }
                strncpy(self->charPort, charPtrFirstUart, sizeof(self->charPort));
                break;
        }
    }
    if ( 0 != self->uint8MsgLevel ) {   // user message
        printf("  INFO:%s: selected UART: %s\n", __FUNCTION__, self->charPort);
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
            case 230400: break;
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
    /* check if there are no pending bytes in UART */
    usbiss_uart_free(self);
    /* check module id */
    uint8Wr[0] = USBISS_CMD;
    uint8Wr[1] = USBISS_ISS_VERSION;
    if ( 2 != usbiss_uart_write(self, uint8Wr, 2) ) {   // request
        if ( 0 != self->uint8MsgLevel ) {
            usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, 2); // convert to ascii
            printf("  ERROR:%s: REQ: %s\n", __FUNCTION__, charBuf);
        }
        return -1;
    }
    uint32RdLen = usbiss_uart_read(self, uint8Rd, 3);
    if ( 3 != uint32RdLen ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, uint32RdLen);
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
    self->uint8Fw = uint8Rd[1];     // USB-ISS firmware revision
    self->uint8Mode = uint8Rd[2];   // current transfer mode
    /* check for proper firmware revision, ISC_DIRECT is needed */
    if ( USBISS_FW_MIN > self->uint8Fw ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Installed USB-ISS FW=0x%02x, Required 0x%02x. Please do an FW update.\n", __FUNCTION__, self->uint8Fw, USBISS_FW_MIN);
        }
        return -1;
    }
    /* Get Serial number */
    uint8Wr[0] = USBISS_CMD;
    uint8Wr[1] = USBISS_GET_SER_NUM;
    if ( 2 != usbiss_uart_write(self, uint8Wr, 2) ) {   // request
        if ( 0 != self->uint8MsgLevel ) {
            usbiss_uint8_to_asciihex(charBuf, sizeof(charBuf), uint8Wr, 2); // convert to ascii
            printf("  ERROR:%s: REQ: %s\n", __FUNCTION__, charBuf);
        }
        return -1;
    }
    uint32RdLen = usbiss_uart_read(self, uint8Rd, 8);
    if ( 8 != uint32RdLen ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, uint32RdLen);
        }
        return -1;
    }
    memset(self->charSerial, 0, sizeof(self->charSerial));  // make empty string
    strncpy(self->charSerial, (char *) uint8Rd, 8); // serial has 8 digits
    if ( 0 != self->uint8MsgLevel ) {
        printf("  INFO:%s: Serial=%s\n", __FUNCTION__, self->charSerial);
    }
    /* mark as open */
    self->uint8IsOpen = 1;
    /* graceful end */
    return 0;
}



/**
 *  usbiss_close
 *    close UART handle
 */
int usbiss_close( t_usbiss *self )
{
    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* close UART handle */
    if ( self->uint8IsOpen ) {
        self->uint8IsOpen = 0;  // close handle
        if ( 0 != simple_uart_close(self->uart) ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: close UART handle\n", __FUNCTION__);
            }
            return -1;
        }
    }
    /* make invalid */
    self->charPort[0] = '\0';
    self->uint32BaudRate = 0;
    self->uint8Mode = __UINT8_MAX__;
    self->uint8Fw = 0;
    self->charSerial[0] = '\0';
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
    char        charBuf[512];   // help buffer for string conversion
    uint8_t     uint8Mode;      // new USBISS mode as opcode
    uint8_t     uint8Wr[16];    // write buffer
    uint8_t     uint8Rd[16];    // read buffer
    uint8_t     uint8WrLen;     // write packet length
    uint32_t    uint32RdLen;

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
        return 0;   // desired mode selected
    }
    /* Build I2C mode set frame */
    if ( 0 == usbiss_is_i2c_mode(uint8Mode) ) {
        uint8Wr[0] = USBISS_CMD;
        uint8Wr[1] = USBISS_SET_ISS_MODE;
        uint8Wr[2] = uint8Mode;
        uint8Wr[3] = 0x04;  // IO_TYPE (see I/O mode above)
        uint8WrLen = 4;
    /* unsupported Format */
    } else {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Methode to change mode not implemented\n", __FUNCTION__);
        }
        return -1;
    }
    if ( 0 != self->uint8MsgLevel ) {
        usbiss_uint8_to_asciihex( charBuf, sizeof(charBuf), uint8Wr, (uint32_t) uint8WrLen );   // convert to ascii hex
        printf("  INFO:%s:REQ: %s\n", __FUNCTION__, charBuf);
    }
    /* set USBISS */
    if ( uint8WrLen != usbiss_uart_write(self, uint8Wr, (uint32_t) uint8WrLen) ) {  // request
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: unexpected number of byte written\n", __FUNCTION__);
        }
        return -1;
    }
    uint32RdLen = usbiss_uart_read(self, uint8Rd, 2);
    if ( 2 != uint32RdLen ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Unexpected number of %i bytes received\n", __FUNCTION__, uint32RdLen);
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
 *  usbiss_i2c_wr
 *    write to I2C device
 */
int usbiss_i2c_wr( t_usbiss *self, uint8_t adr7, void* data, size_t len )
{
    /** Variables **/
    int             intRet;         // internal return code, allows to send stop bit in case of crash

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
    /* Startbit + ADR */
    intRet = usbiss_i2c_startbit(self, (uint8_t) ((adr7 << 1) | USBISS_I2C_WR));
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Startbit failed\n", __FUNCTION__);
        }
        return intRet;
    }
    /* Intermideate Packets, DATA */
    intRet = usbiss_i2c_data_wr(self, data, len);
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s:PKG: Packet Transfer ero=0x%x, go one with STOP BIT to free the bus\n", __FUNCTION__, intRet);
        }
    }
    /* Stop Bit */
    intRet = usbiss_i2c_stopbit(self);
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Stopbit failed, BUS mayby clamped\n", __FUNCTION__);
        }
        return intRet;
    }
    /* graceful end */
    return intRet;
}



/**
 *  usbiss_i2c_rd
 *    read from I2C device
 */
int usbiss_i2c_rd( t_usbiss *self, uint8_t adr7, void* data, size_t len )
{
    /** Variables **/
    int             intRet;         // internal return code, allows to send stop bit in case of crash

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
    /* Startbit + ADR */
    intRet = usbiss_i2c_startbit(self, (uint8_t) ((adr7 << 1) | USBISS_I2C_RD));
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Startbit failed\n", __FUNCTION__);
        }
        return intRet;
    }
    /* Intermideate Packets, read DATA */
    intRet = usbiss_i2c_data_rd(self, data, len);
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s:PKG: Packet Transfer ero=0x%x, go one with STOP BIT to free the bus\n", __FUNCTION__, intRet);
        }
    }
    /* Stop Bit */
    intRet = usbiss_i2c_stopbit(self);
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Stopbit failed, BUS mayby clamped\n", __FUNCTION__);
        }
        return intRet;
    }
    /* graceful end */
    return intRet;
}



/**
 *  usbiss_i2c_wr_rd
 *    writes to i2c devices, sents repeated start for direction change and reads from i2c device
 */
int usbiss_i2c_wr_rd( t_usbiss *self, uint8_t adr7, void* data, size_t wrLen, size_t rdLen )
{
    /** Variables **/
    int     intRet = 0;     // internal return code, allows to send stop bit in case of crash

    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };
    /* empty frame provided */
    if ( (0 == wrLen) || (0 == rdLen) ) {
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
    /* Startbit + ADR */
    intRet |= usbiss_i2c_startbit(self, (uint8_t) ((adr7 << 1) | USBISS_I2C_WR));
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Startbit failed\n", __FUNCTION__);
        }
        return intRet;
    }
    /* Write data */
    intRet |= usbiss_i2c_data_wr(self, data, wrLen);
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s:PKG: Packet Transfer ero=0x%x, go one with STOP BIT to free the bus\n", __FUNCTION__, intRet);
        }
    }
    /* Repeated start + ADR, changes direction to read */
    intRet |= usbiss_i2c_restartbit(self, (uint8_t) ((adr7 << 1) | USBISS_I2C_RD));
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Startbit failed\n", __FUNCTION__);
        }
    }
    /* Read DATA */
    intRet |= usbiss_i2c_data_rd(self, data, rdLen);
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s:PKG: Packet Transfer ero=0x%x, go one with STOP BIT to free the bus\n", __FUNCTION__, intRet);
        }
    }
    /* Stop Bit */
    intRet |= usbiss_i2c_stopbit(self);
    if ( 0 != intRet ) {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Stopbit failed, BUS mayby clamped\n", __FUNCTION__);
        }
    }
    /* graceful end */
    return intRet;
}
