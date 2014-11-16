/*
    DRACO - Copyright (C) 2013-2014 Daniel Strnad

    This file is part of DRACO project.

    DRACO is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    DRACO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * @file    swtimer_config.h
 * @brief   software timers configuration file
 *
 */

#ifndef SWTIMER_CONFIG_H_
#define SWTIMER_CONFIG_H_

#include "utils.h"

/** define maximum count of simultaneously started timers, max. 128 */
#define SWTIMER_SLOTS_COUNT     12

/** external function to query elapsed miliseconds since system start */
#define swtGetElapsedMs() elapsedMs()

#endif /* SWTIMER_CONFIG_H_ */
