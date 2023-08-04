/***********************************************************************
 @copyright     : Siemens AG
 @license       : GPLv3
 @author        : Andreas Kaeberlein
 @address       : Clemens-Winkler-Strasse 3, 09116 Chemnitz

 @maintainer    : Andreas Kaeberlein
 @telephone     : +49 371 4810-2108
 @email         : andreas.kaeberlein@siemens.com

 @file          : usbiss_main.c
 @date          : 2023-06-28
 @see           : https://github.com/andkae/usbiss
 @see           : http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm

 @brief         : usbiss CLI
                  provides CLI interface to usbiss
***********************************************************************/



/** Standard libs **/
#include <errno.h>          // number of last error
#include <getopt.h>         // CLI parser
#include <stdint.h>         // int8_t, ...
#include <stdio.h>          // standard input/output
#include <stdlib.h>
#include <string.h>         // string manipulation
#include <unistd.h>         // fwrite/fread/getuid
/** Custom Libs **/
#include "simple_uart.h"    // cross platform UART driver
#include "usbiss.h"         // USBISS driver



/**
 *  @defgroup HELP_CONSTANTS
 *  help constants for programm
 *  @{
 */
#define USBISS_TERM_VERSION     "0.0.1"     /**< Programm version */
#define MSG_LEVEL_BRIEF         0           /**< Brief Message level */
#define MSG_LEVEL_NORM          1           /**< Normal Message level */
#define MSG_LEVEL_VERB          2           /**< Verbose Message Level */
/** @} */   // HELP_CONSTANTS



/**
 *  @brief print hexdump
 *
 *  dumps memory segment as ascii hex
 *
 *  @param[in]      leadBlank       number of leading blanks in hex line
 *  @param[in,out]  *mem            pointer to memory segment
 *  @param[in,out]  size            number of bytes in *mem to print
 *  @return         void
 *  @since          July 7, 2023
 */
static void print_hexdump (char leadBlank[], void *mem, size_t size)
{
    /** Variables **/
    char        charBuf[32];        // character buffer
    uint8_t     uint8NumAdrDigits;  // number of address digits


    /* check for size */
    if ( 0 == size ) {
        return;
    }
    /* acquire number of digits for address */
    snprintf(charBuf, sizeof(charBuf), "%zx", size-1);  // convert to ascii for address digits calc, -1: addresses start at zero
    uint8NumAdrDigits = (uint8_t) strlen(charBuf);
    /* print to console */
    for ( size_t i = 0; i < size; i = (i + 16) ) {
        /* print address line */
        printf("%s%0*zx:  ", leadBlank, uint8NumAdrDigits, i);
        /* print hex values */
        for ( uint8_t j = 0; j < 16; j++ ) {
            /* out of memory */
            if ( !((i+j) < size) ) {
                break;
            }
            /* print */
            printf("%02x ", ((uint8_t*) mem)[i+j]);
            /* divide high / low byte */
            if ( 7 == j ) {
                printf(" ");
            }
        }
        printf("\n");
    }
}



/**
 *  @brief sprint_hex
 *
 *  write data to hex string
 *
 *  @param[in]      leadBlank       number of leading blanks in hex line
 *  @param[in,out]  *mem            pointer to memory segment
 *  @param[in,out]  size            number of bytes in *mem to print
 *  @return         void
 *  @since          July 7, 2023
 */
static void sprint_hex (char *str, void *mem, size_t size)
{
    /** Variables **/
    size_t  i;
    char    hex[4];

    /* make empty */
    str[0] = '\0';
    /* check for non zero input */
    if ( 0 == size ) {
        return;
    }
    /* convert to hex */
    for ( i = 0; i < size; i++ ) {
        snprintf(hex, sizeof(hex), "%02x ", ((uint8_t*) mem)[i]);
        strncpy(str+i*3, hex, 3);
    }
    /* last blank gets terminator */
    *(str+(i*3-1)) = '\0';
}



/**
 *  @brief count char
 *
 *  count char occurance in string
 *
 *  @param[in]      str             string with char to llok for
 *  @param[in]      c               character to look for
 *  @return         int             count with occurence of 'c'
 *  @since          July 13, 2023
 */
static int cnt_chr (char *str, char c)
{
    /** variables **/
    int cnt = 0;

    /* count */
    for ( size_t i = 0; i < strlen(str); i++ ) {
        if ( c == str[i] ) {
            ++cnt;
        }
    }
    return cnt;
}



/**
 *  @brief to_int
 *
 *  converts string to int
 *
 *  @param[in]      str             string with char to llok for
 *  @return         int             converted number
 *  @since          July 13, 2023
 */
static int to_int (char *str)
{
    int ret = 0;

    /* check for length */
    if ( 2 < strlen(str) ) {    // possible '0x' or '0b'
        if ( ('0' == str[0]) && (('x' == str[1]) || ('X' == str[1])) ) {
            ret = (int) strtol(str+2, NULL, 16);
        } else {
            ret = (int) atoi(str);  // integer number convert
        }
    } else {
        ret = (int) atoi(str);  // integer number convert
    }
    return ret;
}


/**
 *  @brief process command
 *
 *  count char occurance in string
 *
 *  @param[in]      str             string with char to llok for
 *  @param[in]      c               character to look for
 *  @return         int             count with occurence of 'c'
 *  @since          July 13, 2023
 */
static int process_cmd (char *str, uint8_t *adr, uint8_t **data, uint32_t *wrLen, uint32_t *rdLen)
{
    /** Variables **/
    char        access;
    uint32_t    i;
    char        *split;
    char        *ptr;

    /* set defaults */
    *adr = __UINT8_MAX__;
    *wrLen = 0;
    *rdLen = 0;
    /* check for proper string */
    if ( (1 < cnt_chr(str, 'w')) || (1 < cnt_chr(str, 'r')) ) {
        return 1;   // wrong command sequence
    }
    /* copy string to avoid mess up original one */
    split = malloc(strlen(str)+1);
    if ( NULL == split ) {
        return 2;   // mem alloc failed
    }
    strncpy(split, str, strlen(str)+1);
    /* count bytes for read /write */
    i = 0;
    access = ' ';
    ptr = strtok(split, " ");   // split blank
    while( NULL != ptr ) {
        /* process address */
        if ( 0 == i ) {
            *adr = (uint8_t) to_int(ptr);
        }
        /* process non address */
        if ( 0 != i ) {
            if ( 0 < strlen(ptr) ) {
                if ( ('r' == ptr[0]) || ('w' == ptr[0]) ) {
                    access = ptr[0];
                } else {
                    if ( 'w' == access ) {
                        ++(*wrLen);
                    } else if ( 'r' == access ) {
                        *rdLen = (uint32_t) to_int(ptr);
                    } else {
                        return 4;
                    }
                }
            }
        }
        /* prepare nest */
        ++i;
        ptr = strtok(NULL, " ");
    }
    /* allocate memory according array dimensions */
    *data = malloc(usbiss_max(*wrLen, *rdLen));
    if ( *wrLen > 0 ) {
        /* reset write counter */
        *wrLen = 0;
        /* copy string */
        strncpy(split, str, strlen(str)+1);
        /* count bytes for read /write */
        i = 0;
        access = ' ';
        ptr = strtok(split, " ");   // split blank
        while( NULL != ptr ) {
            /* process non address */
            if ( 0 != i ) {
                if ( 0 < strlen(ptr) ) {
                    if ( ('r' == ptr[0]) || ('w' == ptr[0]) ) {
                        access = ptr[0];
                    } else {
                        if ( 'w' == access ) {
                            (*data)[*wrLen] = (uint8_t) to_int(ptr);
                            ++(*wrLen);
                        }
                    }
                }
            }
            /* prepare nest */
            ++i;
            ptr = strtok(NULL, " ");
        }
    }
    /* all fine */
    free(split);
    return 0;
}



// **************************************************************************
// Function: to print help command
// **************************************************************************
void usbiss_term_help(const char path[])
{
    /** Variables **/
    char    uart[1024];

    /* acquire available UART ports */
    if ( 0 == usbiss_list_uart(uart, sizeof(uart)) ) {
        strcpy(uart, "*** no USB-ISS matching ports found ***");
    }
    /* print help */
    printf(
        "\n"
        "USBISS V%s - a CLI tool to interact with the USB-ISS\n"
        "  http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm\n"
        "\n"
        "Usage:\n"
        "  %s --options... \n"
        "\n"
        "Options:\n"
        "  -p, --port=[<UART>]         USB-ISS belonging <UART> port\n"
        "            =[COM1]             Windows default\n"
        "            =[/dev/ttyACM0]     Linux default\n"
        "  -b, --baud=[115200]         UART baud rate\n"
        "  -m, --mode=[I2C_S_100KHZ]   I2C transfer mode\n"
        "                                Standard [I2C_S_20KHZ  | I2C_S_50KHZ  | I2C_S_100KHZ | I2C_S_400KHZ]\n"
        "                                Fast     [I2C_H_100KHZ | I2C_H_400KHZ | I2C_H_1000KHZ]\n"
        "  -c, --command=\"{<pkg>}\"     Data packet to transfer\n"
        "                                <adr7> w <b0> <bn>    : I2C write access with arbitrary number of write bytes <bn>\n"
        "                                <adr7> r <cnt>        : I2C read access with <cnt> bytes read\n"
        "                                <adr7> w <bn> r <cnt> : I2C write access followed by repeated start with read access\n"
        "  -h, --help                  Help\n"
        "  -v, --version               Version\n"
        "      --verbose               Advanced output\n"
        "      --brief                 Only mandatory output\n"
        "\n"
        "Return Value:\n"
        "  0   OK\n"
        "  1   Error, check log for details\n"
        "\n"
        "Ports:\n"
        "  %s\n"
        "\n"
        "Authors:\n"
        "  Andreas Kaeberlein   andreas.kaeberlein@siemens.com\n"
        "\n"
        "Contribute:\n"
        "  https://github.com/andkae/usbiss\n"
        "\n",
        USBISS_TERM_VERSION,
        path,
        uart
    );
}



// *********************************************************************
// Function: main function routine / Provides CLI
// *********************************************************************
int main (int argc, char *argv[])
{
    /** Variables **/
    t_usbiss                usbiss;                         // usbiss handle
    uint8_t                 uint8MsgLevel = MSG_LEVEL_NORM; // message level
    uint32_t                uint32BaudRate;                 // CLI: baud rate
    char                    charPort[128];                  // CLI: port
    char                    charMode[128];                  // CLI: change mode
    char*                   charPtrCmd;                     // CLI: operation command
    char*                   charPtrBuf;                     // buffer help variable
    uint8_t                 uint8I2cAdr = __UINT8_MAX__;    // i2c address
    uint8_t*                uint8PtrWrRd = NULL;            // array with write/read data
    uint32_t                uint32WrLen;                    // number of write elements in uint8PtrWrRd
    uint32_t                uint32RdLen;                    // number of read elements in uint8PtrWr
    uint8_t*                uint8PtrHelp = NULL;            // help variable for wr-rd i2c function


    /* command line parser */
    int opt;                            // switch for parameter
    int arg_index = 0;                  // argument index
    const struct option longopt[] = {   // CLI options
        /* flags */
        { "brief",      no_argument,    (int*) &uint8MsgLevel, MSG_LEVEL_BRIEF },
        { "verbose",    no_argument,    (int*) &uint8MsgLevel, MSG_LEVEL_VERB },
        /* We distinguish them by their indices */
        {"port",        required_argument,  0,  'p'},
        {"baud",        required_argument,  0,  'b'},
        {"mode",        required_argument,  0,  'm'},
        {"command",     required_argument,  0,  'c'},
        {"version",     no_argument,        0,  'v'},
        {"help",        no_argument,        0,  'h'},
        /* Protection */
        {0,             0,                  0,  0 }     // NULL
    };
    static const char shortopt[] = "p:b:m:c:vh";



    /* check for root rights */
    #if defined(__linux__) || defined(__APPLE__)
        if (getuid()) {
            if ( MSG_LEVEL_NORM <= uint8MsgLevel ) { printf("[ FAIL ]   Root rights required! Try 'sudo %s'\n", argv[0]); }
            goto ERO_END_L0;
        }
    #endif

    /* no param, no operation */
    if (argc < 2) {
        if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
            printf("[ FAIL ]   command line options are missing\n");
            printf("             Try '%s --help' for more information.\n", argv[0]);
        }
        goto ERO_END_L0;
    }

    /* flag defaults */
    uint32BaudRate = 0; // use usbiss defaults
    charPort[0] = '\0'; // use defaults
    charPtrCmd = NULL;  // no command
    strncpy(charMode, "I2C_S_100KHZ", sizeof(charMode));

    /* Parse CLI */
    while (-1 != (opt = getopt_long(argc, argv, shortopt, longopt, &arg_index))) {
        // process parameters
        switch (opt)
        {
            /* Process Flags */
            case 0:
                break;

            /* process '--port<COMX | /dev/tty*>' argument */
            case 'p':
                /* check for enough memory */
                if ( (strlen(optarg) + 1) > sizeof(charPort) ) {
                    if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
                        printf("[ FAIL ]   not enough static memory\n");
                    }
                    goto ERO_END_L0;
                }
                /* copy user path */
                strncpy(charPort, optarg, sizeof(charPort));
                break;

            /* process '--baud<baud>' argument */
            case 'b':
                uint32BaudRate = (uint32_t) atoi(optarg);   // convert to integer
                break;

            /* process '--mode<mode>' argument */
            case 'm':
                /* check for enough memory */
                if ( (strlen(optarg) + 1) > sizeof(charMode) ) {
                    if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
                        printf("[ FAIL ]   not enough static memory\n");
                    }
                    goto ERO_END_L0;
                }
                /* copy user path */
                strncpy(charMode, optarg, sizeof(charMode));
                break;

            /* process '--command<cmd>' argument */
            case 'c':
                charPtrCmd = malloc(strlen(optarg) + 2);
                strncpy(charPtrCmd, optarg, strlen(optarg) + 2);
                break;

            /* Print command line options */
            case 'h':
                usbiss_term_help(argv[0]);
                uint8MsgLevel = MSG_LEVEL_BRIEF;    // avoid normal end message
                goto GD_END_L0;
                break;

            /* Print version to console */
            case 'v':
                printf("V%s\n", USBISS_TERM_VERSION);
                uint8MsgLevel = MSG_LEVEL_BRIEF;    // avoid normal end message
                goto GD_END_L0;
                break;

            /* Something went wrong */
            default:
                if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
                    printf("[ FAIL ]   unrecognized option '-%c' use '--help' for proper args.\n", opt);
                }
                goto ERO_END_L0;
        }
    }

    /* Entry Message */
    if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
        printf("[ INFO ]   USBISS started\n");
    }

    /* init USBISS handle */
    if ( 0 != usbiss_init(&usbiss) ) {
        printf("[ FAIL ]   USBISS handle\n");
        goto ERO_END_L0;
    }

    /* propagate message level */
    if ( MSG_LEVEL_VERB == uint8MsgLevel ) {
        usbiss_set_verbose(&usbiss, 1); // enable advanced output
    }

    /* check for proper command */
    if ( NULL == charPtrCmd ) {
        printf("[ FAIL ]   no transfer requested, use -c for proper args\n");
        goto ERO_END_L0;
    }
    if ( 0 != process_cmd(charPtrCmd, &uint8I2cAdr, &uint8PtrWrRd, &uint32WrLen, &uint32RdLen) ) {
        printf("[ FAIL ]   option '-c %s' unsupported, use --help for proper read/write command\n", charPtrCmd);
        goto ERO_END_L0;
    }

    /* open UART Port */
    if ( 0 != usbiss_open(&usbiss, charPort, uint32BaudRate) ) {
        if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
            printf("[ FAIL ]   unable to open USBISS\n");
            printf("             Port: %s\n", usbiss.charPort);
            printf("             Baud: %i\n", usbiss.uint32BaudRate);
        }
        goto ERO_END_L0;
    }
    if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
        printf("[ OKAY ]   USBISS connected\n");
        printf("             Baudrate : %i\n", usbiss.uint32BaudRate);
        printf("             Firmware : 0x%02x\n", usbiss.uint8Fw);
        printf("             Serial   : %s\n", usbiss.charSerial);
    }

    /* set mode */
    if ( 0 < strlen(charMode) ) {
        if ( 0 != usbiss_set_mode(&usbiss, charMode) ) {
            printf("[ FAIL ]   USBISS mode setup\n");
            goto ERO_END_L1;
        }
    }
    if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
        printf("             Mode     : %s\n", usbiss_mode_to_human(usbiss.uint8Mode));
    }

    /* perform access */
    if ( (0 != uint32WrLen) && (0 == uint32RdLen) ) {
        /* write access only */
        if ( 0 == usbiss_i2c_wr(&usbiss, uint8I2cAdr, (void*) uint8PtrWrRd, (size_t) uint32WrLen) ) {
            if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
                printf("[ OKAY ]   Write %i bytes to device 0x%02x\n", uint32WrLen, uint8I2cAdr);
                print_hexdump("             ", uint8PtrWrRd, uint32WrLen);
            }
        } else {
            if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
                printf("[ FAIL ]   Write %i bytes to device 0x%02x\n", uint32WrLen, uint8I2cAdr);
            }
            goto ERO_END_L1;
        }
    } else if ( (0 == uint32WrLen) && (0 != uint32RdLen) ) {
        /* read access only */
        if ( 0 == usbiss_i2c_rd(&usbiss, uint8I2cAdr, (void*) uint8PtrWrRd, (size_t) uint32RdLen) ) {
            if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
                printf("[ OKAY ]   Read %i bytes from device 0x%02x\n", uint32RdLen, uint8I2cAdr);
                print_hexdump("             ", uint8PtrWrRd, uint32RdLen);
            } else {
                charPtrBuf = malloc(3*uint32RdLen+1);
                if ( NULL != charPtrBuf ) {
                    sprint_hex(charPtrBuf, uint8PtrWrRd, uint32RdLen);
                    printf("%s\n", charPtrBuf);
                    free(charPtrBuf);
                }
            }
        } else {
            if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
                printf("[ FAIL ]   Read %i bytes from device 0x%02x\n", uint32RdLen, uint8I2cAdr);
            }
            goto ERO_END_L1;
        }
    } else if ( (0 != uint32WrLen) && (0 != uint32RdLen) ) {
        /* write-read access only */
            // save write data
        uint8PtrHelp = malloc(uint32WrLen);
        if ( NULL != uint8PtrHelp ) {
            memcpy(uint8PtrHelp, uint8PtrWrRd, uint32WrLen);
        }
            // perform access
        if ( 0 == usbiss_i2c_wr_rd(&usbiss, uint8I2cAdr, (void*) uint8PtrWrRd, (size_t) uint32WrLen, (size_t) uint32RdLen) ) {
            if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
                printf("[ OKAY ]   Write/Read interaction with device 0x%02x\n", uint8I2cAdr);
                printf("           Write %i Bytes\n", uint32WrLen);
                if ( NULL != uint8PtrHelp ) {
                    print_hexdump("             ", uint8PtrHelp, uint32WrLen);
                    free(uint8PtrHelp);
                }
                printf("           Read %i Bytes\n", uint32RdLen);
                print_hexdump("             ", uint8PtrWrRd, uint32RdLen);
            } else {
                charPtrBuf = malloc(3*uint32RdLen+1);
                if ( NULL != charPtrBuf ) {
                    sprint_hex(charPtrBuf, uint8PtrWrRd, uint32RdLen);
                    printf("%s\n", charPtrBuf);
                    free(charPtrBuf);
                }
            }
        } else {
            if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
                printf("[ FAIL ]   Write %i and Read %i bytes from device 0x%02x\n", uint32WrLen, uint32RdLen, uint8I2cAdr);
            }
            goto ERO_END_L1;
        }
    }

    /* mostly done, clean-up */
    free(uint8PtrWrRd);
    free(charPtrCmd);
    if ( 0 != usbiss_close(&usbiss) ) {
        printf("[ FAIL ]   close USBISS connection\n");
        goto ERO_END_L0;
    }

    /* gracefull end */
    goto GD_END_L0; // avoid compile warning
    GD_END_L0:
        if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
            printf("[ OKAY ]   ended normally\n");
        }
        exit(EXIT_SUCCESS);

    /* Error L1 End */
    goto ERO_END_L1;
    ERO_END_L1:
        usbiss_close(&usbiss);  // ero end, try to close connection
    /* Error L0 End */
    goto ERO_END_L0;
    ERO_END_L0:
        if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
            printf("[ FAIL ]   ended abnormally :-(\n");
        }
        exit(EXIT_FAILURE);

}
