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
#include "usbiss.h"			// USBISS driver
/** self **/
#include "usbiss_main.h"	// some defs







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
        {"help",        no_argument,        0,  'h'},
        /* Protection */
        {0,             0,                  0,  0 }     // NULL
    };
    static const char shortopt[] = "p:b:m:c:h";



    /* check for root rights */
    #if defined(__linux__) || defined(__APPLE__)
		if (getuid()) {
			if ( MSG_LEVEL_NORM <= uint8MsgLevel ) { printf("[ FAIL ]   Root rights required! Try 'sudo %s'\n", argv[0]); }
			goto ERO_END_L0;
		}
	#endif;


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
                //usbiss_help(argv[0]);	// TODO
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
        printf("             %s\n", VERSION);
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

