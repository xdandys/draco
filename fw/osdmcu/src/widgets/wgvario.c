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
 * @file    wgvario.c
 * @brief   brief description here
 *
 */

#include "wgvario.h"
#include "osdpainter.h"
#include "utils.h"
#include <math.h>
#include <string.h>

#define MAX_RANGE_LENGTH       30
#define POINTER_WIDTH          4

#define ABS(x)  (((x) > 0) ? (x) : -(x))
static const WidgetConfig cfg;


static const PainterPen varioPen = {
    .color = PAINTER_COLOR_WHITE,
    .outline = 1,
    .style = PAINTER_PEN_STYLE_SOLID,
};


static const char varioFont[] = "consolas14";

static void paint(void *widget, OsdPainter *painter, int x, int y)
{
    WgVario *self = widget;
    osdPainterTranslate(painter, x, y);
    osdPainterSetPen(painter, &varioPen);
    int ydeflection = (int)((-self->vspeed / self->range) * (float)MAX_RANGE_LENGTH);
    ydeflection = (ydeflection > MAX_RANGE_LENGTH) ? MAX_RANGE_LENGTH : ydeflection;
    ydeflection = (ydeflection < -MAX_RANGE_LENGTH) ? -MAX_RANGE_LENGTH : ydeflection;

    osdPainterDrawLine(painter, 0, 0, 0, ydeflection);
    osdPainterDrawLine(painter, -POINTER_WIDTH, ydeflection, POINTER_WIDTH, ydeflection);

    osdPainterSetFont(painter, osdFontByName(varioFont));
    char str[12];
    if (self->vspeed <= 0)
        lsprintf(str, "%d.%d%s", (int)self->vspeed, ABS((int)(self->vspeed * 10) % 10), self->units);
    else
        lsprintf(str, "+%d.%d%s", (int)self->vspeed, ABS((int)(self->vspeed * 10) % 10), self->units);
    osdPainterDrawText(painter, 6, -7, str);
}






void wgVarioInit(WgVario *wg, float range, const char *units)
{
    osdWidgetInit(&wg->widget);
    osdWidgetSetCfg(&wg->widget, &cfg);
    wg->units[0] = 0;
    wgVarioSetUnits(wg, units);
    wg->vspeed = 0;
    wg->range = range;
}

void wgVarioSetSpeed(WgVario *wg, float vspeed)
{
    if (vspeed != wg->vspeed) {
        if (fabs(vspeed) >= 1000) vspeed = 9999;
        wg->vspeed = vspeed;
        osdWidgetRedraw(&wg->widget);
    }
}

void wgVarioSetUnits(WgVario *wg, const char *name)
{
    strncpy(wg->units, name, sizeof(wg->units) - 1);
    wg->units[sizeof(wg->units) - 1] = 0;
    osdWidgetRedraw(&wg->widget);
}

void wgVarioSetRange(WgVario *wg, float range)
{
    wg->range = range;
    osdWidgetRedraw(&wg->widget);
}

static const WidgetConfig cfg = {
    .ops = {
        .paint = paint,
        .process = 0,
    },
};
