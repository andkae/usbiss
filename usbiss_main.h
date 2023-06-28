/***********************************************************************
 @copyright     : Siemens AG
 @license       : GPLv3
 @author        : Andreas Kaeberlein
 @address       : Clemens-Winkler-Strasse 3, 09116 Chemnitz

 @maintainer    : Andreas Kaeberlein
 @telephone     : +49 371 4810-2108
 @email         : andreas.kaeberlein@siemens.com

 @file          : usbiss_main.h
 @date          : 2023-06-28

 @brief         : usbiss CLI
                  provides CLI interface to usbiss
***********************************************************************/



// Define Guard
#ifndef __USBISS_MAIN_H
#define __USBISS_MAIN_H



/**
 *  @defgroup HELP_CONSTANTS
 *  help constants for programm
 *  @{
 */
#define VERSION                 "v0.0.1"    /**< Programm version */
#define MSG_LEVEL_BRIEF 0                   /**< Brief Message level */
#define MSG_LEVEL_NORM  1                   /**< Normal Message level */
#define MSG_LEVEL_VERB  2                   /**< Verbose Message Level */
#define POLL_SLEEP_US           10          /**< wait time for sleep in busy wait */
/** @} */   // HELP_CONSTANTS




#endif // __USBISS_MAIN_H
