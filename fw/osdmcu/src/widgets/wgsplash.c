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
 * @file    wgsplash.c
 * @brief   brief description here
 *
 */

#include "wgsplash.h"
#include "utils.h"
#include "osdpainter.h"
#include <string.h>

static const WidgetConfig cfg;

void wgSplashInit(WgSplash *wg, uint8_t major, uint8_t minor)
{
    osdWidgetInit(&wg->widget);
    osdWidgetSetCfg(&wg->widget, &cfg);
    wg->major = major;
    wg->minor = minor;
}

static void paint(void *widget, OsdPainter *painter, int x, int y)
{
    WgSplash *self = widget;
    osdPainterTranslate(painter, x, y);

    osdPainterDrawBitmap(painter, 0, 0, osdBitmapByName("dracologo"));
    osdPainterSetFont(painter, osdFontByName("consolas12"));

    char buff[64];
    lsprintf(buff, "OSD FW version: %u.%02u\n powered by TauLabs", self->major, self->minor);
    osdPainterDrawText(painter, 30, 7 + osdBitmapByName("dracologo")->height, buff);
}




static const WidgetConfig cfg = {
    .ops = {
        .paint = paint,
        .process = 0,
    },
};
