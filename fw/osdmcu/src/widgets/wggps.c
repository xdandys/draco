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
 * @file    wggps.c
 * @brief   brief description here
 *
 */

#include "wggps.h"
#include "osdpainter.h"
#include "utils.h"
#include <math.h>
#include <string.h>

#define ABS(x)  (((x) > 0) ? (x) : -(x))
static const WidgetConfig cfg;


inline static char latPrefix(int32_t lat)
{
    if (lat < 0) return 'S'; else return 'N';
}

inline static char lonPrefix(int32_t lon)
{
    if (lon < 0) return 'W'; else return 'E';
}

static void paint(void *widget, OsdPainter *painter, int x, int y)
{
    WgGps *self = widget;
    int32_t lat = ABS(self->lat);
    int32_t lon = ABS(self->lon);
    char gpsText[45];
    lsprintf(gpsText, "%c%d.%07d\n%c%d.%07d\nPDOP:%d.%d SAT:%u",
             latPrefix(self->lat), lat / 10000000, lat % 10000000 ,
             lonPrefix(self->lon), lon / 10000000, lon % 10000000 ,
             (int)self->pdop, (int)(self->pdop * 10.0) % 10,
             self->satCount);
    wgTextSetText(&self->wgtext, gpsText);
    wgTextPaint(&self->wgtext, painter, x, y);
}


static void process(void *widget)
{
    WgGps *self = widget;
    if (wgTextProcees(&self->wgtext))
        osdWidgetRedraw(&self->widget);
}

void wgGpsInit(WgGps *wg)
{
    osdWidgetInit(&wg->widget);
    osdWidgetSetCfg(&wg->widget, &cfg);
    wgTextInit(&wg->wgtext, "satelite", "consolas10");
    // wgTextInit(&wg->wgtext, "clock", "consolas10");
}

void wgGpsSet(WgGps *wg, bool fix, int32_t lat, int32_t lon, float pdop, uint8_t satCount)
{
    if ((fix != wg->fix) || (lat != wg->lat) || (lon != wg->lon) || (pdop != wg->pdop) || (satCount != wg->satCount)) {
        wg->fix = fix;
        wg->lat = lat;
        wg->lon = lon;
        wg->pdop = pdop;
        wg->satCount = satCount;
        wgTextBlink(&wg->wgtext, !fix, 300, 0);
        osdWidgetRedraw(&wg->widget);
    }
}

static const WidgetConfig cfg = {
    .ops = {
        .paint = paint,
        .process = process,
    },
};
