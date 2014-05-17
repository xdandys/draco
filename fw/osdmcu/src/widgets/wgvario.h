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
 * @file    wgvario.h
 * @brief   brief description here
 *
 */


#ifndef WGVARIO_H_
#define WGVARIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "osdwidget.h"

typedef struct {
    OsdWidget widget;
    float vspeed;
    float range;
    char units[6];
} WgVario;


void wgVarioInit(WgVario *wg, float range, const char *units);
void wgVarioSetSpeed(WgVario *wg, float vspeed);
void wgVarioSetUnits(WgVario *wg, const char *units);
void wgVarioSetRange(WgVario *wg, float range);

#ifdef __cplusplus
}
#endif
#endif /* WGVARIO_H_ */

/** @} */
