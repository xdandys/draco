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
 * @file    wgcompass.c
 * @brief   brief description here
 *
 */

#include "wgcompass.h"
#include "osdpainter.h"
#include "utils.h"
#include <math.h>
#include <string.h>

#define SCALE_LENGTH            100
#define SHORT_NOTCH_SPACE       8
#define LONG_NOTCH_SPACE        40
#define SHORT_NOTCH_LEN         4
#define LONG_NOTCH_LEN          8
#define SCALE                   90

static const WidgetConfig cfg;

static const PainterPen scalePen = {
    .color = PAINTER_COLOR_WHITE,
    .outline = 0,
    .style = PAINTER_PEN_STYLE_SOLID,
};

static const PainterPen scalePenInv = {
    .color = PAINTER_COLOR_BLACK,
    .outline = 0,
    .style = PAINTER_PEN_STYLE_SOLID,
};


static const PainterPen captionTextPen = {
    .color = PAINTER_COLOR_WHITE,
    .outline = 1,
    .style = PAINTER_PEN_STYLE_SOLID,
};

static const PainterPen headingTextPen = {
    .color = PAINTER_COLOR_WHITE,
    .outline = 1,
    .style = PAINTER_PEN_STYLE_SOLID,
};

static const PainterBrush arrowBrush = {
    .color = PAINTER_COLOR_BLACK,
    .style = PAINTER_BRUSH_STYLE_SOLID,
};

static const PainterPen arrowPen = {
    .color = PAINTER_COLOR_WHITE,
    .style = PAINTER_BRUSH_STYLE_SOLID,
    .outline = 0,
};


static const char captionFont[] = "consolas10";
static const char headingFont[] = "consolas18";

static const int arrowPoly[] = {
    +0, +1,
    -5, +9,
    +5, +9,
};


static int normHeading(int deg)
{
    int norm = deg % 360;
    if (norm < 0)
        norm += 360;
    return norm;
}

static char *deg2WorldSide(int deg)
{
    deg = normHeading(deg);

    switch (deg) {
    case 0:   return "N";
    case 45:  return "NE";
    case 90:  return "E";
    case 135: return "SE";
    case 180: return "S";
    case 225: return "SW";
    case 270: return "W";
    case 315: return "NW";
    default:  return 0;
    }
}

static void paint(void *widget, OsdPainter *painter, int x, int y)
{
    WgCompass *self = widget;
    osdPainterTranslate(painter, x, y);
    osdPainterSetPen(painter, &scalePen);
    // osdPainterDrawLine(painter, -SCALE_LENGTH / 2, 0, SCALE_LENGTH / 2, 0);

    // draw scale
    int rem = self->heading % SCALE;
    int xoffset = (int)(((float)rem / (float)SCALE) * (float)LONG_NOTCH_SPACE);
    int xscale0 = (-(SCALE_LENGTH / 2) / LONG_NOTCH_SPACE - 1) * LONG_NOTCH_SPACE - xoffset;
    int xscale = xscale0;
    int deg =  (+(self->heading / SCALE) -(SCALE_LENGTH / 2) / LONG_NOTCH_SPACE - 1) * SCALE;
    osdPainterSetFont(painter, osdFontByName(captionFont));
    while(xscale <= (SCALE_LENGTH / 2)) {
        bool longNotch = ((xscale0 - xscale) % LONG_NOTCH_SPACE == 0);
        if ((xscale >= (-SCALE_LENGTH / 2)) && (xscale <= (SCALE_LENGTH / 2))) {
            if (longNotch) {
                // draw long notch
                osdPainterSetPen(painter, &scalePen);
                osdPainterDrawLine(painter, xscale, 0, xscale, -LONG_NOTCH_LEN);
                osdPainterSetPen(painter, &scalePenInv);
                osdPainterDrawLine(painter, xscale + 1, 0, xscale + 1, -LONG_NOTCH_LEN);
                // draw caption
                osdPainterSetPen(painter, &captionTextPen);
                osdPainterDrawText(painter,  xscale - 2, -LONG_NOTCH_LEN - 10, deg2WorldSide(deg));
            } else {
                // draw short notch
                osdPainterSetPen(painter, &scalePen);
                osdPainterDrawLine(painter, xscale, 0, xscale, -SHORT_NOTCH_LEN);
                osdPainterSetPen(painter, &scalePenInv);
                osdPainterDrawLine(painter, xscale + 1, 0, xscale + 1, -SHORT_NOTCH_LEN);
            }
        }
        if (longNotch) deg += SCALE;
        xscale += SHORT_NOTCH_SPACE;
    }

    // draw heading value
    char str[5];
    lsprintf(str, "%03d", self->heading);
    osdPainterSetFont(painter, osdFontByName(headingFont));
    osdPainterSetPen(painter, &headingTextPen);
    int width = 0, height = 0;
    osdPainterTextBounds(painter, str, &width, &height);
    osdPainterDrawText(painter, -width / 2, 10, str);

    // draw arrow

    osdPainterSetPen(painter, &arrowPen);
    osdPainterSetBrush(painter, &arrowBrush);
    osdPainterDrawPoly(painter, arrowPoly, 3);
}



void wgCompassInit(WgCompass *wg)
{
    osdWidgetInit(&wg->widget);
    osdWidgetSetCfg(&wg->widget, &cfg);
    wg->heading = 0;
}


void wgCompassSetHeading(WgCompass *wg, int heading)
{
    heading = normHeading(heading);
    if (heading != wg->heading) {
        wg->heading = heading;
        osdWidgetRedraw(&wg->widget);
    }
}


static const WidgetConfig cfg = {
    .ops = {
        .paint = paint,
        .process = 0,
    },
};
