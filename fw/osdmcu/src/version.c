/*
 * version.c
 *
 *  Created on: 3.2.2012
 *      Author: strnad
 */


#include "version.h"

// version mustn't exceed 32 bytes!
__attribute__ ((section(".app_version"), used))
const char __APP_VERSION[] = VERSIONSTRING;
