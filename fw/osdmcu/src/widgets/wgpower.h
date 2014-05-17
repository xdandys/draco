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
 * @file    wgpower.h
 * @brief   brief description here
 *
 */


#ifndef WGPOWER_H_
#define WGPOWER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "osdwidget.h"
#include "wgtext.h"

typedef struct {
    OsdWidget widget;
    WgText wgtext;
    float voltage;
    float current;
    uint16_t mAhs;
    float voltageLow;
    float mAhsHigh;
    bool showVoltage;
    bool showCurrent;
    bool showMahs;
    bool warning;
} WgPower;

void wgPowerInit(WgPower *wg);
void wgPowerSetVoltage(WgPower *wg, float voltage);
void wgPowerSetCurrent(WgPower *wg, float current);
void wgPowerSetMahs(WgPower *wg, uint16_t mahs);
void wgPowerSetLimits(WgPower *wg, float voltageLow, uint16_t mAhsHigh);

#ifdef __cplusplus
}
#endif
#endif /* WGPOWER_H_ */

/** @} */
