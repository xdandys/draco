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
 * @file    wgwaypoint.c
 * @brief   waypoint direction and distance widget
 *
 */


#include "wgwaypoint.h"
#include "osdpainter.h"
#include "utils.h"
#include <math.h>
#include <string.h>

#define ARROW_WIDTH         7
#define ARROW_HEIGHT        12

static const WidgetConfig cfg;

static const PainterPen waypointPen = {
    .color = PAINTER_COLOR_WHITE,
    .outline = 1,
    .style = PAINTER_PEN_STYLE_SOLID,
};

static const PainterPen arrowPen = {
    .color = PAINTER_COLOR_BLACK,
    .outline = 1,
    .style = PAINTER_PEN_STYLE_SOLID,
};

static const PainterBrush arrowBrush = {
    .color = PAINTER_COLOR_BLACK,
    .style = PAINTER_PEN_STYLE_NONE,
};


static const char nameFont[] = "consolas10";
static const char distanceFont[] = "consolas18";

static void paint(void *widget, OsdPainter *painter, int x, int y)
{
    WgWaypoint *self = widget;
    osdPainterTranslate(painter, x, y);
    osdPainterSetPen(painter, &waypointPen);

    char str[16];
    // draw name
    lsprintf(str, "%s\n %s", self->name, self->units);
    int width = 0, height = 0;
    osdPainterSetFont(painter, osdFontByName(nameFont));
    osdPainterTextBounds(painter, str, &width, &height);
    osdPainterDrawText(painter, 0, -2, str);

    // draw distance
    if (self->distance >= 10)
        lsprintf(str, "%d", (int)self->distance);
    else
        lsprintf(str, "%d.%d", (int)self->distance, ((int)(self->distance * 10)) % 10);
    int width2 = 0, height2 = 0;
    osdPainterSetFont(painter, osdFontByName(distanceFont));
    osdPainterTextBounds(painter, str, &width2, &height2);
    osdPainterDrawText(painter, width + 1, 0, str);

    // draw arrow
    osdPainterSetPen(painter, &arrowPen);
    osdPainterSetBrush(painter, &arrowBrush);
    osdPainterTranslate(painter, (width + width2)  + 15, height2 / 2);
    osdPainterRotate(painter, self->heading);
    int arrow[] = {0, 0,
                 ARROW_WIDTH, ARROW_HEIGHT,
                 0, -ARROW_HEIGHT,
                 -ARROW_WIDTH, ARROW_HEIGHT};
    osdPainterDrawPoly(painter, arrow, 4);
}

void wgWaypointInit(WgWaypoint *wg, const char *name, const char *units)
{
    osdWidgetInit(&wg->widget);
    osdWidgetSetCfg(&wg->widget, &cfg);
    wg->name[0] = 0;
    wg->units[0] = 0;
    wgWaypointSetName(wg, name);
    wgWaypointSetUnits(wg, units);

    wg->heading = 0;
    wg->distance = 0;
}


void wgWaypointSet(WgWaypoint *wg, float heading, float distance)
{
    if ((heading != wg->heading) || (distance != wg->distance)) {
        wg->heading = heading;
        if (fabs(distance) > 100000) distance = 99999;
        wg->distance = distance;
        osdWidgetRedraw(&wg->widget);
    }
}

void wgWaypointSetName(WgWaypoint *wg, const char *name)
{
    strncpy(wg->name, name, sizeof(wg->name) - 1);
    wg->name[sizeof(wg->name) - 1] = 0;
    osdWidgetRedraw(&wg->widget);
}

void wgWaypointSetUnits(WgWaypoint *wg, const char *name)
{
    strncpy(wg->units, name, sizeof(wg->units) - 1);
    wg->units[sizeof(wg->units) - 1] = 0;
    osdWidgetRedraw(&wg->widget);
}

static const WidgetConfig cfg = {
    .ops = {
        .paint = paint,
        .process = 0,
    },
};
