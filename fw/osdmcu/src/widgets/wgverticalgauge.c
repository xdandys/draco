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
 * @file    wgverticalgauge.c
 * @brief   brief description here
 *
 */

#include "wgverticalgauge.h"
#include "osdpainter.h"
#include "utils.h"
#include <math.h>
#include <string.h>

#define GAUGE_HEIGHT                100
#define SHORT_NOTCH_SPACE           4
#define LONG_NOTCH_SPACE            28
#define SHORT_NOTCH_LEN             4
#define LONG_NOTCH_LEN              6
#define NEEDLE_LENGTH               15


static const WidgetConfig cfg;
static const PainterPen gaugePen = {
    .color = PAINTER_COLOR_WHITE,
    .outline = 0,
    .style = PAINTER_PEN_STYLE_SOLID,
};

static const PainterPen needlePen = {
    .color = PAINTER_COLOR_WHITE,
    .outline = 1,
    .style = PAINTER_PEN_STYLE_SOLID,
};

static const PainterPen textPen = {
    .color = PAINTER_COLOR_WHITE,
    .outline = 1,
    .style = PAINTER_PEN_STYLE_SOLID,
};

static const PainterBrush needleBrush = {
    .color = PAINTER_COLOR_TRANSPARENT,
    .style = PAINTER_BRUSH_STYLE_SOLID,
};

static const PainterBrush gaugeBrush = {
    .style = PAINTER_BRUSH_STYLE_NONE,
};

static const char captionFont[] = "consolas10";
static const char needleFont[] = "consolas18";

static void paint(void *widget, OsdPainter *painter, int x, int y)
{        
    WgVerticalGauge *self = widget;
    osdPainterTranslate(painter, x, y);
    osdPainterSetPen(painter, &gaugePen);
    osdPainterSetBrush(painter, &gaugeBrush);
    osdPainterDrawLine(painter, 0, -GAUGE_HEIGHT / 2, 0, GAUGE_HEIGHT / 2);

    // draw scale
    osdPainterSetFont(painter, osdFontByName("consolas10"));
    float rem = fmodf(self->value, self->scale);
    int offset = (int)((rem / self->scale) * LONG_NOTCH_SPACE);
    float centerNotchVal =self->scale * ((int)(self->value / self->scale));
    float upNotchVal = centerNotchVal;
    float downNotchVal = upNotchVal;
    int ygamut = 0;
    while(1) {
        bool longNotch = (((ygamut) % LONG_NOTCH_SPACE) == 0);
        int notchLen = (longNotch) ? LONG_NOTCH_LEN : SHORT_NOTCH_LEN;
        if (self->leftSide) notchLen = -notchLen;
        if ((ygamut + offset) <= (GAUGE_HEIGHT / 2)) {
            if ((!self->noNegative) || (downNotchVal >= 0)) {
                osdPainterDrawLine(painter, 0, ygamut + offset, notchLen, ygamut + offset);
                // draw notch caption (lower)
                if (longNotch) {
                    char strVal[8];
                    itoa((int)downNotchVal, strVal, 10);
                    int textX;
                    if (self->leftSide) {
                        int width = 0;
                        int height = 0;
                        osdPainterTextBounds(painter, strVal, &width, &height);
                        textX = -LONG_NOTCH_LEN - 2 - width;
                    } else {
                        textX = LONG_NOTCH_LEN + 2;
                    }
                    osdPainterDrawText(painter, textX, ygamut + offset - 5, strVal);
                }
            }
            if (longNotch) downNotchVal -= self->scale;
        }
        if ((-ygamut + offset) < (-GAUGE_HEIGHT / 2)) break;


        osdPainterDrawLine(painter, 0, -ygamut + offset, notchLen, -ygamut + offset);

        // draw notch caption (upper)
        if (longNotch) {
            char strVal[8];
            itoa((int)upNotchVal, strVal, 10);
            upNotchVal += self->scale;
            int textX;
            if (self->leftSide) {
                int width = 0;
                int height = 0;
                osdPainterTextBounds(painter, strVal, &width, &height);
                textX = -LONG_NOTCH_LEN - 2 - width;
            } else {
                textX = LONG_NOTCH_LEN + 2;
            }
            osdPainterDrawText(painter, textX, -ygamut + offset - 5, strVal);
        }
        ygamut += SHORT_NOTCH_SPACE;
    }

    // draw arrow with value
    char strVal[8];
    if (fabsf(self->value) < 10) {
        itoa((int)self->value, strVal, 10);
        int i = (self->value < 0) ? 2 : 1;
        strVal[i] = '.';
        itoa((int)(self->value * 10.0f) % 10, &strVal[i + 1], 10);
    } else {
        itoa((int)self->value, strVal, 10);
    }
    int width = 0;
    int height = 0;
    osdPainterSetFont(painter, osdFontByName(needleFont));
    osdPainterTextBounds(painter, strVal, &width, &height);

    int textValueX = 0;
    int textUnitX = 0;
    int poly[10];
    if (!self->leftSide) {
        poly[0] = 1; poly[1] =  0;
        poly[2] = NEEDLE_LENGTH; poly[3] =  -height / 2 - 1;
        poly[4] = NEEDLE_LENGTH + width + 1; poly[5] =  -height / 2 -1;
        poly[6] = NEEDLE_LENGTH + width + 1; poly[7] = height / 2 + 1;
        poly[8] = NEEDLE_LENGTH;  poly[9] = height / 2 +1;
        textValueX = NEEDLE_LENGTH + 1;
        textUnitX = LONG_NOTCH_LEN + 28;
    } else {
        poly[0] = -1; poly[1] =  0;
        poly[2] = -NEEDLE_LENGTH; poly[3] =  -height / 2 - 1;
        poly[4] = -NEEDLE_LENGTH - width - 1; poly[5] =  -height / 2 -1;
        poly[6] = -NEEDLE_LENGTH - width - 1; poly[7] = height / 2 + 1;
        poly[8] = -NEEDLE_LENGTH;  poly[9] = height / 2 +1;

        textValueX = - NEEDLE_LENGTH - width + 1;
        textUnitX = - LONG_NOTCH_LEN - 45;

    }

    osdPainterSetPen(painter, &needlePen);
    osdPainterSetBrush(painter, &needleBrush);
    osdPainterDrawPoly(painter, poly, 5);
    osdPainterSetBrush(painter, &gaugeBrush);
    osdPainterSetPen(painter, &textPen);
    osdPainterDrawText(painter, textValueX, -height / 2 + 1, strVal);
    osdPainterSetFont(painter, osdFontByName(captionFont));
    osdPainterDrawText(painter, textUnitX, 12, self->units);
}


void wgVerticalGaugeInit(WgVerticalGauge *wg, float scale, bool leftSide, bool noNegative, const char *units)
{
    osdWidgetInit(&wg->widget);
    osdWidgetSetCfg(&wg->widget, &cfg);
    wg->scale = scale;
    wg->noNegative = noNegative;
    wg->leftSide = leftSide;
    wg->value = 0;
    wgVerticalGaugeSetUnits(wg, units);
}

void wgVerticalGaugeSetValue(WgVerticalGauge *wg, float value)
{
    if (value != wg->value) {
        if (fabs(value) > 100000) value = 99999;
        wg->value = value;
        osdWidgetRedraw(&wg->widget);
    }
}

void wgVerticalGaugeSetUnits(WgVerticalGauge *wg, const char *units)
{
    strncpy(wg->units, units, sizeof(wg->units) - 1);
    wg->units[sizeof(wg->units) - 1] = 0;
    osdWidgetRedraw(&wg->widget);
}

static const WidgetConfig cfg = {
    .ops = {
        .paint = paint,
        .process = 0,
    },
};
