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
 * @file    wgverticalgauge.h
 * @brief   brief description here
 *
 */


#ifndef WGVERTICALGAUGE_H_
#define WGVERTICALGAUGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "osdwidget.h"


typedef struct {
    OsdWidget widget;
    char units[6];
    float scale;
    float value;
    bool leftSide;
    bool noNegative;
} WgVerticalGauge;


void wgVerticalGaugeInit(WgVerticalGauge *wg, float scale, bool leftSide, bool noNegative, const char *units);
void wgVerticalGaugeSetValue(WgVerticalGauge *wg, float value);
void wgVerticalGaugeSetUnits(WgVerticalGauge *wg, const char *units);

#ifdef __cplusplus
}
#endif

#endif /* WGVERTICALGAUGE_H_ */

/** @} */
