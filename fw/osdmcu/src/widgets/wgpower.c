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
 * @file    wgpower.c
 * @brief   brief description here
 *
 */

#include "wgpower.h"
#include "osdpainter.h"
#include "utils.h"
#include <math.h>
#include <string.h>
static const WidgetConfig cfg;


static void paint(void *widget, OsdPainter *painter, int x, int y)
{
    WgPower *self = widget;
    char text[48];
    char voltageText[16];
    char mahsText[16];
    char currentText[16];
    voltageText[0] = 0;
    mahsText[0] = 0;
    currentText[0] = 0;

    if (self->showVoltage)
        lsprintf(voltageText, "%02d.%02dV\n", (int)self->voltage, (int)(self->voltage * 100.0f) % 100);
    if (self->showCurrent)
        lsprintf(currentText, "%02d.%01dA\n", (int)self->current, (int)(self->current * 10.0f) % 10);
    if (self->showMahs)
        lsprintf(mahsText, "%umAhs\n", self->mAhs);

    lsprintf(text, "%s%s%s", voltageText, currentText, mahsText);

    wgTextSetText(&self->wgtext, text);
    wgTextPaint(&self->wgtext, painter, x, y);
}

static void process(void *widget)
{
    WgPower *self = widget;
    if (wgTextProcees(&self->wgtext))
        osdWidgetRedraw(&self->widget);
}

static void evalLimits(WgPower *wg)
{
    bool warn = false;
    if ((wg->mAhs > wg->mAhsHigh) && (wg->showMahs))
        warn = true;

    if ((wg->voltage < wg->voltageLow) && (wg->showVoltage))
        warn = true;

    if (wg->warning != warn) {
        wg->warning = warn;
        if (warn)
            wgTextBlink(&wg->wgtext, true, 250, 0);
        else
            wgTextBlink(&wg->wgtext, false, 250, 0);
        osdWidgetRedraw(&wg->widget);
    }
}

void wgPowerInit(WgPower *wg)
{
    osdWidgetInit(&wg->widget);
    osdWidgetSetCfg(&wg->widget, &cfg);
    wgTextInit(&wg->wgtext, "battery", "consolas14");
    wg->showVoltage = false;
    wg->showCurrent = false;
    wg->showMahs = false;
    wg->mAhsHigh = 65535;
    wg->voltageLow = 0;
    wg->warning = false;
}

void wgPowerSetCurrent(WgPower *wg, float current)
{
    if ((wg->current != current) || (!wg->showCurrent)) {
        wg->current = current;
        wg->showCurrent = true;
        osdWidgetRedraw(&wg->widget);
    }
}

void wgPowerSetVoltage(WgPower *wg, float voltage)
{
    if ((wg->voltage != voltage) || (!wg->showVoltage)) {
        wg->voltage = voltage;
        wg->showVoltage = true;
        evalLimits(wg);
        osdWidgetRedraw(&wg->widget);
    }
}

void wgPowerSetMahs(WgPower *wg, uint16_t mahs)
{
    if ((wg->mAhs != mahs) || (!wg->showMahs)) {
        wg->mAhs = mahs;
        wg->showMahs = true;
        evalLimits(wg);
        osdWidgetRedraw(&wg->widget);
    }
}

void wgPowerSetLimits(WgPower *wg, float voltageLow, uint16_t mAhsHigh)
{
    wg->voltageLow = voltageLow;
    wg->mAhsHigh = mAhsHigh;
    evalLimits(wg);
}

static const WidgetConfig cfg = {
    .ops = {
        .paint = paint,
        .process = process,
    },
};
