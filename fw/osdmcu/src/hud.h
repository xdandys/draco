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
 * @file    hud.h
 * @brief   manages all HUD widgets
 *
 */

#ifndef HUD_H_
#define HUD_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include "osddevice.h"

typedef enum {
    HUD_UNITS_METRIC,
    HUD_UNITS_IMPERIAL,
} HudUnits;

typedef enum {
    HUD_WAYPOINT_LOS,
    HUD_WAYPOINT_NAV,
} HudWaypoint;

void hudInit(OsdDevice *osdDevice);
void hudStart(void);
void hudStop(void);
void hudSetUnits(HudUnits units);
void hudProcess(void);

void hudSetPfdData(float speed, float vspeed, float altitude, float roll, float pitch,
        float heading);
void hudSetWaypoint(HudWaypoint wp, bool show, float distance, float heading);
void hudSetGnss(bool fix, int32_t lat, int32_t lon, float pdop, uint8_t satCount);
void hudSetBatteryVoltage(float voltage);
void hudSetBatteryCurrent(float current, float ahs);
void hudSetBatteryLimits(float minVoltage, float maxAhs);
void hudControlStopwatch(bool running, uint16_t maxSeconds);
void hudSetFlightMode(const char *mode);

#ifdef __cplusplus
}
#endif
#endif /* HUD_H_ */

/** @} */
