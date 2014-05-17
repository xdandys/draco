/*
 * swtimer_config.h
 *
 *  Created on: 4.2.2012
 *      Author: dan
 *      SW timer configuration file
 */

#ifndef SWTIMER_CONFIG_H_
#define SWTIMER_CONFIG_H_

#include "delay.h"


// define maximum count of simultaneously started timers, max. 128
#define SWTIMER_SLOTS_COUNT     12

// external function to query elapsed miliseconds since system start
#define swtGetElapsedMs() getElapsedMs()

#endif /* SWTIMER_CONFIG_H_ */
