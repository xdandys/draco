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
 * @file    wgStopwatch.c
 * @brief   brief description here
 *
 */


#include "wgstopwatch.h"
#include "osdpainter.h"
#include "utils.h"
#include <math.h>
#include <string.h>

static const WidgetConfig cfg;
static void evalLimit(WgStopwatch *wg);

static void paint(void *widget, OsdPainter *painter, int x, int y)
{
    WgStopwatch *self = widget;
    char text[12];
    lsprintf(text, "%02d:%02d", self->seconds / 60, self->seconds % 60);
    wgTextSetText(&self->wgtext, text);
    wgTextPaint(&self->wgtext, painter, x, y);
}

static void process(void *widget)
{
    WgStopwatch *self = widget;
    if (wgTextProcees(&self->wgtext))
        osdWidgetRedraw(&self->widget);

    if (self->running) {
        int sec = (elapsedMs() - self->milis) / 1000;
        if (sec != self->seconds) {
            self->seconds = sec;
            evalLimit(self);
            osdWidgetRedraw(&self->widget);
        }
    }
}

static void evalLimit(WgStopwatch *wg)
{
    if (wg->warn != (wg->seconds > wg->secondsHigh)) {
        wg->warn = (wg->seconds > wg->secondsHigh);
        if (wg->warn)
            wgTextBlink(&wg->wgtext, true, 250, 0);
        else
            wgTextBlink(&wg->wgtext, false, 0, 0);
        osdWidgetRedraw(&wg->widget);
    }
}

void wgStopwatchInit(WgStopwatch *wg)
{
    osdWidgetInit(&wg->widget);
    osdWidgetSetCfg(&wg->widget, &cfg);
    wgTextInit(&wg->wgtext, "clock", "consolas14");
    wg->warn = false;
    wg->running = false;
    wg->seconds = 0;
    wg->secondsHigh = 1000000;
}

void wgStopwatchSetLimit(WgStopwatch *wg, int secondsHigh)
{
    wg->secondsHigh = secondsHigh;
    evalLimit(wg);
}

void wgStopwatchStart(WgStopwatch *wg, bool reset)
{
    if (reset) {
        wg->seconds = 0;
    }
    wg->milis = elapsedMs();
    wg->running = true;
    osdWidgetRedraw(&wg->widget);
}

void wgStopwatchStop(WgStopwatch *wg)
{
    wg->running = false;
}

static const WidgetConfig cfg = {
    .ops = {
        .paint = paint,
        .process = process,
    },
};
