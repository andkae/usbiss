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

 @brief         : usbiss CLI
                  provides CLI interface to usbiss
***********************************************************************/



/** Standard libs **/
#include <errno.h>          // number of last error
#include <fcntl.h>          // open
#include <getopt.h>         // CLI parser
#include <stdint.h>         // int8_t, ...
#include <stdio.h>          // standard input/output
#include <stdlib.h>
#include <string.h>         // string manipulation
#include <unistd.h>         // fwrite/fread
#include <stddef.h>         // offsetof - offset of a structure member
#include <math.h>           // round
/** Custom Libs **/
#include "simple_uart.h"	// cross platform UART driver
#include "usbiss.h"			// USBISS driver



/**
 *  @defgroup HELP_CONSTANTS
 *  help constants for programm
 *  @{
 */
#define USBISS_TERM_VERSION     "0.0.1"    	/**< Programm version */
#define MSG_LEVEL_BRIEF 		0           /**< Brief Message level */
#define MSG_LEVEL_NORM  		1           /**< Normal Message level */
#define MSG_LEVEL_VERB  		2           /**< Verbose Message Level */
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
 *
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
    snprintf(charBuf, sizeof(charBuf), "%lx", size-1);	// convert to ascii for address digits calc, -1: addresses start at zero
    uint8NumAdrDigits = (uint8_t) strlen(charBuf);
    /* print to console */
    for ( size_t i = 0; i < size; i = (i + 16) ) {
        /* print address line */
        printf("%s%0*lx:  ", leadBlank, uint8NumAdrDigits, i);
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



// **************************************************************************
// Function: to print help command
// **************************************************************************
void usbiss_term_help(const char path[])
{
    /** Variables **/
    char*       charPtrTyps;
    uint32_t    uint32TypsLen;


    /* clear console */
    if (system("clear")) {};

    /* print help */
    printf("\n");
    printf("USBISS V%s - a CLI tool to interact with the USB-ISS\n", USBISS_TERM_VERSION);
    printf("  http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm\n");
    printf("\n");
    printf("Usage:\n");
    printf("  %s --options... \n", path);
    printf("\n");
    printf("Options:\n");
    printf("  -p, --port=[<UART>]         USB-ISS belonging <UART> port\n");
	printf("            =[COM1]             Windows default\n");
	printf("            =[/dev/ttyACM0]     Linux default\n");
    printf("  -b, --baud=[115200]         UART baud rate\n");
    printf("  -m, --mode=[I2C_S_100KHZ]   I2C transfer mode\n");
    printf("                                Standard [I2C_S_20KHZ  | I2C_S_50KHZ  | I2C_S_100KHZ | I2C_S_400KHZ]\n");
	printf("                                Fast     [I2C_H_100KHZ | I2C_H_400KHZ | I2C_H_1000KHZ]\n");
    printf("  -c, --command=\"{<pkg>}\"     Data packet to transfer\n");
    printf("                                <adr7> w <b0> <bn>    : I2C write access with arbitrary number of write bytes <bn>\n");
	printf("                                <adr7> r <cnt>        : I2C read access with <cnt> bytes read\n");
	printf("                                <adr7> w <bn> r <cnt> : I2C write access followed by repeated start with read access\n");
    printf("  -h, --help                  Help\n");
	printf("  -v, --version               Version\n");
    printf("      --verbose               Advanced output\n");
    printf("      --brief                 Only mandatory output\n");
    printf("\n");
    printf("Return Value:\n");
    printf("   0   OK\n");
    printf("   1   Error, check log for details\n");
    printf("\n");
    printf("UART Ports:\n");
	printf("\n");
	printf("Authors:\n");
    printf("  Andreas Kaeberlein         andreas.kaeberlein@siemens.com\n");
    printf("\n");
    printf("Contribute:\n");
    printf("  https://github.com/andkae/usbiss\n");
    printf("\n");
}



// *********************************************************************
// Function: main function routine / Provides CLI
// *********************************************************************
int main (int argc, char *argv[])
{
	/** Variables **/
	uint8_t					uint8MsgLevel = MSG_LEVEL_NORM;		// message level
	uint32_t				uint32BaudRate;						// CLI: baud rate
	char					charPort[128];						// CLI: port
	char					charMode[128];						// CLI: change mode
	t_usbiss				usbiss;								// usbiss handle

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
		{"version",     no_argument,  		0,  'v'},
        {"help",        no_argument,        0,  'h'},
        /* Protection */
        {0,             0,                  0,  0 }     // NULL
    };
    static const char shortopt[] = "p:b:m:c:hv";



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
	uint32BaudRate = 0;		// use usbiss defaults
	charPort[0] = '\0';		// use defaults
	charMode[0] = '\0';		// no change

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
				uint32BaudRate = (uint32_t) atoi(optarg);	// convert to integer
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
			
            /* Print command line options */
            case 'h':
                usbiss_term_help(argv[0]);
                uint8MsgLevel = MSG_LEVEL_BRIEF;	// avoid normal end message
				goto GD_END_L0;
                break;

            /* Print version to console */
            case 'v':
                printf("V%s\n", USBISS_TERM_VERSION);
                uint8MsgLevel = MSG_LEVEL_BRIEF;	// avoid normal end message
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
		usbiss_set_verbose(&usbiss, 1);	// enable advanced output
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



	/* todo */
		// write test data
	uint8_t data[256];
	for (uint8_t i=0; i < 64; i++ ) {
		data[i] = i;
		//data[i] = 0xff;
	}
	data[0] =0x00; 
	data[1] =0x00; 
	if ( 0 != usbiss_i2c_wr(&usbiss, 0x50, data, 64) ) {
		goto ERO_END_L1;
	}
	usleep(5100);	// wait for complete
		// set address counter to zero
	data[0] =0x00; 
	data[1] =0x00; 
	if ( 0 != usbiss_i2c_wr(&usbiss, 0x50, data, 2) ) {
		goto ERO_END_L1;
	}
	usleep(5100);	// wait for complete
		// read data
	if ( 0 != usbiss_i2c_rd(&usbiss, 0x50, data, 256) ) {
		goto ERO_END_L1;
	}
	print_hexdump("  ", data, 256);
	usleep(5100);	// wait for complete
	data[0] =0x00; 
	data[1] =0x00;
	if ( 0 != usbiss_i2c_wr_rd(&usbiss, 0x50, data, 2, 256) ) {
		goto ERO_END_L1;
	}
	print_hexdump("  ", data, 256);
	usleep(5100);	// wait for complete	
	
	
	


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
		while (0) {};	// todo, close UART connection
	/* Error L0 End */
    goto ERO_END_L0;
	ERO_END_L0:
        if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
			printf("[ FAIL ]   ended abnormally :-(\n");
		}
        exit(EXIT_FAILURE);

}

