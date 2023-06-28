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
#include "./inc/simple_uart/simple_uart.h"	// cross platform UART driver
/** self **/
#include "usbiss_main.h"	// some defs







// *********************************************************************
// Function: main function routine / Provides CLI
// *********************************************************************
int main (int argc, char *argv[])
{
	/** Variables **/
	uint8_t					uint8MsgLevel = MSG_LEVEL_NORM;		// message level
	uint32_t				uint32BaudRate = 9600;				// default baudrate
	char					charPort[128];						// default port
	struct simple_uart 		*uart;								// uart handle

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
        {"help",        no_argument,        0,  'h'},
        /* Protection */
        {0,             0,                  0,  0 }     // NULL
    };
    static const char shortopt[] = "p:b:m:c:h";



    /* Entry Message */
    if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
        printf("[ INFO ]   usbiss started\n");
        printf("             %s\n", VERSION);
    }

    /* no param, no operation */
    if (argc < 2) {
        if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
            printf("[ FAIL ]   command line options are missing\n");
            printf("             Try '%s --help' for more information.\n", argv[0]);
        }
        goto ERO_END_L0;
    }

	/* init default path to UART based on platform */
    #if defined(__linux__) || defined(__APPLE__)
		strncpy(charPort, "/dev/ttyACM0", sizeof(charPort));
	#else
		strncpy(charPort, "COM1", sizeof(charPort));
	#endif

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
				switch (uint32BaudRate) {
					case 9600: break;
					case 14400: break;
					case 19200: break;
					case 38400: break;
					case 57600: break;
					case 115200: break;
					default:
						if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
							printf("[ FAIL ]   Unsupported USBISS baudrate of %i\n", uint32BaudRate);
						}
						goto ERO_END_L0;
				}
				break;
			
            /* Print command line options */
            case 'h':
                //usbiss_help(argv[0]);	// TODO
                goto GD_END_L0;
                break;

            /* Something went wrong */
            default:
                printf("[ FAIL ]   unrecognized option '-%c' use '--help' for proper args.\n", opt);
                goto ERO_END_L0;
        }
    }

	/* open UART Port */
	uart = simple_uart_open(charPort, uint32BaudRate, "8N1");	// usbiss is fixed to 8bit and 1 stopbit
	if (!uart) {
		if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
			printf("[ FAIL ]   unable to open UART\n");
			printf("             Port: %s\n", charPort);
			printf("             Baud: %i\n", uint32BaudRate);
			printf("             Mode: 8N1\n", opt);
		}
		goto ERO_END_L0;
	}
	printf("[ OKAY ]   UART %s open\n", charPort);








    /* gracefull end */
    goto GD_END_L0; // avoid compile warning
    GD_END_L0:
		if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
			printf("[ OKAY ]   ended normally\n");
		}
		exit(EXIT_SUCCESS);

    /* Error L0 End */
    goto ERO_END_L0;
	ERO_END_L0:
        if ( MSG_LEVEL_NORM <= uint8MsgLevel ) {
			printf("[ FAIL ]   ended abnormally :-(\n");
		}
        exit(EXIT_FAILURE);

}

