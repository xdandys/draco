/*
 * main.c
 *
 *  Created on: 29.10.2012
 *      Author: dan
 */


/*
 * TIM4  .. Motor
 * TIM19 .. Cutter
 * TIM13 .. Burn pulses
 * TIM3  .. Light
 *
 */


#include "debug.h"
#include "delay.h"
#include "board.h"
#include "osdspi.h"
#include "led.h"
#include "delay.h"
#include "osdpainter.h"
#include "osdscreen.h"
#include "utils.h"
#include "comm.h"
#include <widgets/wgverticalgauge.h>
#include <widgets/wgcompass.h>
#include <widgets/wgattitude.h>
#include <widgets/wgwaypoint.h>
#include <widgets/wgvario.h>
#include <widgets/wggps.h>
#include <widgets/wgpower.h>
#include <widgets/wgstopwatch.h>

static OsdPainter painter;
void osdMain(void)
{
    //------------------------------------------------------------------------
    //
    //         TEST CODE
    //
    //------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------

    osdDeviceStart(&spiOsdDevice);
    osdPainterInit(&painter);
    osdPainterSetDevice(&painter, &spiOsdDevice);


    OsdScreen pfdScreen;
    WgVerticalGauge wgAltitude;
    WgVerticalGauge wgSpeed;
    WgCompass wgCompass;
    WgAttitude wgAttitude;
    WgWaypoint wgWaypoint;
    WgVario wgVario;
    WgGps wgGps;
    WgPower wgPower;
    WgStopwatch wgStopwatch;


    osdScreenInit(&pfdScreen, &painter);
    wgVerticalGaugeInit(&wgAltitude, 5, false, false, "m");
    wgVerticalGaugeInit(&wgSpeed, 5, true, true, "km/h");
    wgCompassInit(&wgCompass);
    wgAttitudeInit(&wgAttitude, 1.0);
    wgWaypointInit(&wgWaypoint, "LOS", "m");
    wgVarioInit(&wgVario, 8, "m/s");
    wgGpsInit(&wgGps);
    wgPowerInit(&wgPower);
    wgStopwatchInit(&wgStopwatch);

    osdScreenAddWidget(&pfdScreen, &wgSpeed.widget, 60, 120);
    osdScreenAddWidget(&pfdScreen, &wgAltitude.widget, 260, 120);
    osdScreenAddWidget(&pfdScreen, &wgCompass.widget, 160, 20);
    osdScreenAddWidget(&pfdScreen, &wgAttitude.widget, 160, 120);
    osdScreenAddWidget(&pfdScreen, &wgWaypoint.widget, 220, 5);
    osdScreenAddWidget(&pfdScreen, &wgWaypoint.widget, 220, 30);
    osdScreenAddWidget(&pfdScreen, &wgVario.widget, 240, 185);
    osdScreenAddWidget(&pfdScreen, &wgGps.widget, 22, 5);
    osdScreenAddWidget(&pfdScreen, &wgPower.widget, 10, 195);
    osdScreenAddWidget(&pfdScreen, &wgStopwatch.widget, 142, 220);
    wgStopwatchStart(&wgStopwatch, true);


    while(1) {
        static uint32_t tm = 0;
        static float value = 0;
        static int heading = 0;
        static float roll = 0;
        static float pitch = 0;
        static float vario = 0;
        static bool varioDirUp = true;

        if ((elapsedMs() - tm) > 20) {
            tm = elapsedMs();
            heading++;
            roll += 1;
            pitch += 1;
            if (vario > 12.5)
                varioDirUp = false;
            if (vario < -12.5)
                varioDirUp = true;
            if (varioDirUp) vario += 0.3; else vario -= 0.3;
            value += 0.2;
        }

//        value += 0.00002;
//        heading++;
        wgVerticalGaugeSetValue(&wgSpeed, value);
        wgVerticalGaugeSetValue(&wgAltitude, value);
        wgCompassSetHeading(&wgCompass, heading);
        wgAttitudeSetAngles(&wgAttitude, roll, pitch);
        wgWaypointSet(&wgWaypoint, heading * 2, value);
        wgVarioSetSpeed(&wgVario, vario);
        wgGpsSet(&wgGps, true, 471238467, 128845627, 1.7, 7);
        wgPowerSetVoltage(&wgPower, 11.4);
        wgPowerSetCurrent(&wgPower, 85.5);
        wgPowerSetMahs(&wgPower, 2471);
        wgPowerSetLimits(&wgPower, 11.5, 5000);
        osdScreenProcess(&pfdScreen, false);
        commProcess();
    }
}





int main(void)
{
    boardInit();
    sysTimerInit();
    ledInit();
    commStart();
    osdMain();
    while(1);
    return 0;
}
