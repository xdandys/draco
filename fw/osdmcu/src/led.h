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
 * @file    led.h
 * @brief   LEDs controlling code
 *
 */

#ifndef LED_H_
#define LED_H_

#define LED_RED                 0
#define LED_GREEN               1
#define LED_BLUE                2
#define LED_ORANGE              3

#define LED_MODE_OFF            0
#define LED_MODE_ON             1
#define LED_MODE_BLINK_SLOW     2
#define LED_MODE_BLINK_FAST     3

void ledInit(void);
void ledControl(uint8_t led, uint8_t mode);

#endif /* LED_H_ */

/** @} */
