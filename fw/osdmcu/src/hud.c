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
 * @file    hud.c
 * @brief   managing all HUD widgets
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <osdpainter.h>
#include <osdscreen.h>
#include <widgets/wgverticalgauge.h>
#include <widgets/wgcompass.h>
#include <widgets/wgattitude.h>
#include <widgets/wgwaypoint.h>
#include <widgets/wgvario.h>
#include <widgets/wggps.h>
#include <widgets/wgpower.h>
#include <widgets/wgstopwatch.h>
#include <widgets/wgmode.h>
#include <widgets/wgsplash.h>
#include "hud.h"
#include "version.h"
#include "swtimer.h"

//------------------------------------------------------------------------
//
//         definitions
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

#define SPLASH_TIMEOUT                          3000

#define UNIT_MPS_TO_KNOTS                       0.514444f
#define UNIT_MPS_TO_KMH                         3.6f
#define UNIT_M_TO_FT                            3.280839f

typedef struct
{
    OsdDevice *device;
    bool running;

    // painter
    OsdPainter painter;

    // widgets
    WgVerticalGauge wgAltitude;
    WgVerticalGauge wgSpeed;
    WgCompass wgCompass;
    WgAttitude wgAttitude;
    WgWaypoint wgLosWp;
    WgWaypoint wgNaviWp;
    WgVario wgVario;
    WgGps wgGps;
    WgPower wgPower;
    WgStopwatch wgStopwatch;
    WgMode wgMode;
    WgSplash wgSplash;

    // screens
    OsdScreen screenSplash;
    OsdScreen screenPfd;
    OsdScreen *screenActive;

    HudUnits units;
    // timers
    int8_t swtSplash;
} Hud;

// HUD is currently singleton
static Hud hud_p;
static Hud *hud = &hud_p;

//------------------------------------------------------------------------
//
//         private functions
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

static void createPfdLayout(void)
{
    osdScreenAddWidget(&hud->screenPfd, &hud->wgAttitude.widget, 160, 120);
    osdScreenAddWidget(&hud->screenPfd, &hud->wgSpeed.widget, 60, 120);
    osdScreenAddWidget(&hud->screenPfd, &hud->wgAltitude.widget, 260, 120);
    osdScreenAddWidget(&hud->screenPfd, &hud->wgCompass.widget, 160, 20);
    osdScreenAddWidget(&hud->screenPfd, &hud->wgLosWp.widget, 220, 5);
    osdScreenAddWidget(&hud->screenPfd, &hud->wgNaviWp.widget, 220, 30);
    osdScreenAddWidget(&hud->screenPfd, &hud->wgVario.widget, 240, 185);
    osdScreenAddWidget(&hud->screenPfd, &hud->wgGps.widget, 22, 5);
    osdScreenAddWidget(&hud->screenPfd, &hud->wgPower.widget, 10, 195);
    osdScreenAddWidget(&hud->screenPfd, &hud->wgStopwatch.widget, 142, 220);
    osdScreenAddWidget(&hud->screenPfd, &hud->wgMode.widget, 250, 220);

    // these widgets are implicitly invisible
    osdWidgetSetVisible(&hud->wgLosWp.widget, false);
    osdWidgetSetVisible(&hud->wgNaviWp.widget, false);
    osdWidgetSetVisible(&hud->wgPower.widget, false);
    osdWidgetSetVisible(&hud->wgGps.widget, false);

}

static void createSplahLayout(void)
{
    osdScreenAddWidget(&hud->screenSplash, &hud->wgSplash.widget, 60, 80);
}

static void changeScreen(OsdScreen *screen)
{
    hud->screenActive = screen;
    if (hud->running)
        osdScreenProcess(hud->screenActive, true);
}

static void onSplashTimeout(void *arg)
{
    (void)arg;
    if (hud->screenActive == &hud->screenSplash)
        changeScreen(&hud->screenPfd);
}

//------------------------------------------------------------------------
//
//         public interface
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void hudInit(OsdDevice *osdDevice)
{
    hud->device = osdDevice;

    osdDeviceStart(hud->device);
    // initialize painter
    osdPainterInit(&hud->painter);

    // initialize screen
    osdScreenInit(&hud->screenSplash, &hud->painter);
    osdScreenInit(&hud->screenPfd, &hud->painter);
    changeScreen(&hud->screenSplash);

    // initialize widgets
    wgVerticalGaugeInit(&hud->wgAltitude, 5, false, false, "m");
    wgVerticalGaugeInit(&hud->wgSpeed, 5, true, true, "km/h");
    wgCompassInit(&hud->wgCompass);
    wgAttitudeInit(&hud->wgAttitude, 1.33);
    wgWaypointInit(&hud->wgLosWp, "LOS", "m");
    wgWaypointInit(&hud->wgNaviWp, "NAV", "m");
    wgVarioInit(&hud->wgVario, 8, "m/s");
    wgGpsInit(&hud->wgGps);
    wgPowerInit(&hud->wgPower);
    wgStopwatchInit(&hud->wgStopwatch);
    wgModeInit(&hud->wgMode);
    wgSplashInit(&hud->wgSplash, VMAJOR, VMINOR);

    // create timers
    hud->swtSplash = swTimerInstall(onSplashTimeout, hud);

    // set units
    hudSetUnits(HUD_UNITS_METRIC);

    hudStart();
}

void hudStart(void)
{
    if (hud->running)
        return;

    osdPainterSetDevice(&hud->painter, hud->device);

    createPfdLayout();
    createSplahLayout();

    swTimerStart(hud->swtSplash, SPLASH_TIMEOUT, 1);

    hud->running = true;
}

void hudSetUnits(HudUnits units)
{
    hud->units = units;
    switch(units) {
    case HUD_UNITS_METRIC:
        wgVerticalGaugeInit(&hud->wgAltitude, 5, false, false, "m");
        wgVerticalGaugeInit(&hud->wgSpeed, 5, true, true, "km/h");
        wgWaypointSetUnits(&hud->wgLosWp, "m");
        wgWaypointSetUnits(&hud->wgNaviWp, "m");
        wgVarioInit(&hud->wgVario, 8, "m/s");
        break;

    case HUD_UNITS_IMPERIAL:
        wgVerticalGaugeInit(&hud->wgAltitude, 15, false, false, "ft");
        wgVerticalGaugeInit(&hud->wgSpeed, 4, true, true, "knots");
        wgWaypointSetUnits(&hud->wgLosWp, "ft");
        wgWaypointSetUnits(&hud->wgNaviWp, "ft");
        wgVarioInit(&hud->wgVario, 24, "ft/s");
        break;
    }
}

void hudSetPfdData(float speed, float vspeed, float altitude, float roll, float pitch,
        float heading)
{
    if (!hud->running)
        return;

    if (hud->units == HUD_UNITS_IMPERIAL) {
        speed *= UNIT_MPS_TO_KNOTS;
        vspeed *= UNIT_M_TO_FT;
        altitude *= UNIT_M_TO_FT;
    } else {
        speed *= UNIT_MPS_TO_KMH;
    }

    wgAttitudeSetAngles(&hud->wgAttitude, roll, pitch);
    wgCompassSetHeading(&hud->wgCompass, (int)heading);
    wgVerticalGaugeSetValue(&hud->wgAltitude, altitude);
    wgVerticalGaugeSetValue(&hud->wgSpeed, speed);
    wgVarioSetSpeed(&hud->wgVario, vspeed);
}

void hudSetWaypoint(HudWaypoint wp, bool show, float distance, float heading)
{
    WgWaypoint *wwg = 0;
    if (hud->units == HUD_UNITS_IMPERIAL)
        distance *= UNIT_M_TO_FT;

    switch(wp) {
    case HUD_WAYPOINT_LOS:
        wwg = &hud->wgLosWp;
        break;

    case HUD_WAYPOINT_NAV:
        wwg = &hud->wgNaviWp;
        break;

    default:
        return;
    }

    osdWidgetSetVisible(&wwg->widget, show);
    wgWaypointSet(wwg, heading, distance);
}

void hudSetGnss(bool fix, int32_t lat, int32_t lon, float pdop, uint8_t satCount)
{
    osdWidgetSetVisible(&hud->wgGps.widget, true);
    wgGpsSet(&hud->wgGps, fix, lat, lon, pdop, satCount);
}

void hudSetBatteryVoltage(float voltage)
{
    osdWidgetSetVisible(&hud->wgPower.widget, true);
    wgPowerSetVoltage(&hud->wgPower, voltage);
}

void hudSetBatteryCurrent(float current, float ahs)
{
    osdWidgetSetVisible(&hud->wgPower.widget, true);
    wgPowerSetCurrent(&hud->wgPower, current);
    wgPowerSetMahs(&hud->wgPower, (uint16_t)(ahs * 1000.0f));
}

void hudSetBatteryLimits(float minVoltage, float maxAhs)
{
    wgPowerSetLimits(&hud->wgPower, minVoltage, (uint16_t)(maxAhs * 1000.0f));
}

void hudControlStopwatch(bool running, uint16_t maxSeconds)
{
    wgStopwatchSetLimit(&hud->wgStopwatch, maxSeconds);
    if (running)
        wgStopwatchStart(&hud->wgStopwatch, false);
    else
        wgStopwatchStop(&hud->wgStopwatch);
}

void hudSetFlightMode(const char *mode)
{
    wgModeSetMode(&hud->wgMode, mode);
}

void hudStop(void)
{
    if (!hud->running)
        return;

    // osdDeviceDeinit(hud->device);
    hud->running = false;
}

void hudProcess(void)
{
    if (!hud->running)
        return;

    osdScreenProcess(hud->screenActive, false);
}
