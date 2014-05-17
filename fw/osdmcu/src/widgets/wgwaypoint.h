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
 * @file    wgwaypoint.h
 * @brief   brief description here
 *
 */


#ifndef WGWAYPOINT_H_
#define WGWAYPOINT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "osdwidget.h"

typedef struct {
    OsdWidget widget;
    float heading;
    float distance;
    char name[6];
    char units[4];
} WgWaypoint;


void wgWaypointInit(WgWaypoint *wg, const char *name, const char *units);
void wgWaypointSet(WgWaypoint *wg, float heading, float distance);
void wgWaypointSetName(WgWaypoint *wg, const char *name);
void wgWaypointSetUnits(WgWaypoint *wg, const char *units);

#ifdef __cplusplus
}
#endif
#endif /* WGWAYPOINT_H_ */

/** @} */
