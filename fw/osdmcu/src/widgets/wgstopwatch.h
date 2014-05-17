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
 * @file    wgStopwatch.h
 * @brief   brief description here
 *
 */


#ifndef WGSTOPWATCH_H_
#define WGSTOPWATCH_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "osdwidget.h"
#include "wgtext.h"

typedef struct {
    OsdWidget widget;
    WgText wgtext;
    int seconds;
    bool running;
    int secondsHigh;
    bool warn;
    uint32_t milis;

} WgStopwatch;

void wgStopwatchInit(WgStopwatch *wg);
void wgStopwatchStart(WgStopwatch *wg, bool reset);
void wgStopwatchStop(WgStopwatch *wg);
void wgStopwatchSetLimit(WgStopwatch *wg, int secondsHigh);


#ifdef __cplusplus
}
#endif
#endif /* WGSTOPWATCH_H_ */

/** @} */
