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
 * @file    wgmode.c
 * @brief   brief description here
 *
 */

#include "wgmode.h"
#include "osdpainter.h"
#include "utils.h"
#include <math.h>
#include <string.h>


static const WidgetConfig cfg;


static void paint(void *widget, OsdPainter *painter, int x, int y)
{
    WgMode *self = widget;
    wgTextSetText(&self->wgtext, self->mode);
    wgTextPaint(&self->wgtext, painter, x, y);
}

static void process(void *widget)
{
    WgMode *self = widget;
    if (wgTextProcees(&self->wgtext))
        osdWidgetRedraw(&self->widget);
}


void wgModeInit(WgMode *wg)
{
    osdWidgetInit(&wg->widget);
    osdWidgetSetCfg(&wg->widget, &cfg);
    wgTextInit(&wg->wgtext, "mode", "consolas14");
    wg->mode[0] = 0;
}

void wgModeSetMode(WgMode *wg, const char *mode)
{
    if (strcmp(wg->mode, mode) != 0) {
        strncpy(wg->mode, mode, sizeof(wg->mode) - 1);
        wg->mode[sizeof(wg->mode) - 1] = 0;
        wgTextBlink(&wg->wgtext, true, 250, 8);
        osdWidgetRedraw(&wg->widget);
    }
}


static const WidgetConfig cfg = {
    .ops = {
        .paint = paint,
        .process = process,
    },
};


