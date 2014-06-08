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
 * @file    wgattitude.c
 * @brief   brief description here
 *
 */


#include "wgattitude.h"
#include "osdpainter.h"
#include "utils.h"
#include <math.h>
#include <string.h>

#define LONG_HORIZONT_LINE_LEN      80
#define SHORT_HORIZONT_LINE_LEN     40

static const WidgetConfig cfg;

static const PainterPen attitudePen = {
    .color = PAINTER_COLOR_WHITE,
    .outline = 1,
    .style = PAINTER_PEN_STYLE_SOLID,
};



static void paint(void *widget, OsdPainter *painter, int x, int y)
{
    WgAttitude *self = widget;
    osdPainterTranslate(painter, x, y);
    osdPainterSetClipping(painter, -200, -50, +200, +50);
    osdPainterSetPen(painter, &attitudePen);

    osdPainterTranslate(painter, 0, (int)(self->pitchAngle * self->pitchScale));

    osdPainterRotate(painter, -self->rollAngle);
    osdPainterDrawLine(painter, -LONG_HORIZONT_LINE_LEN / 2, -3, +LONG_HORIZONT_LINE_LEN / 2, -3);
    osdPainterDrawLine(painter, -SHORT_HORIZONT_LINE_LEN / 2, 3, +SHORT_HORIZONT_LINE_LEN / 2, 3);

    osdPainterIdentity(painter);
    osdPainterTranslateOrigin(painter);
    osdPainterTranslate(painter, x, y);
    osdPainterDrawCircle(painter, 0, 0, 4);
    osdPainterDrawLine(painter, -10, 0, -5, 0);
    osdPainterDrawLine(painter, 10, 0, 5, 0);
    osdPainterDrawLine(painter, 0, -5, 0, -10);
}


static float normRoll(float angle)
{
    angle = fmodf(angle + 180, 360);
    if (angle < 0)
        angle += 360;

    return angle - 180;
}

static float normPitch(float angle)
{
    angle = fmodf(angle + 90, 180);
    if (angle < 0)
        angle += 180;

    return angle - 90;
}

void wgAttitudeInit(WgAttitude *wg, float pitchScale)
{
    osdWidgetInit(&wg->widget);
    osdWidgetSetCfg(&wg->widget, &cfg);
    wg->rollAngle = 0;
    wg->pitchAngle = 0;
    wg->pitchScale = pitchScale;
}

void wgAttitudeSetAngles(WgAttitude *wg, float roll, float pitch)
{
    roll = normRoll(roll);
    pitch = normPitch(pitch);
    if ((roll != wg->rollAngle) || (pitch != wg->pitchAngle)) {
        wg->rollAngle = roll;
        wg->pitchAngle = pitch;
        osdWidgetRedraw(&wg->widget);
    }
}

void wgAttitudeSetPitchScale(WgAttitude *wg, float pitchScale)
{
    if (wg->pitchScale != pitchScale) {
        wg->pitchScale = pitchScale;
        osdWidgetRedraw(&wg->widget);
    }
}

static const WidgetConfig cfg = {
    .ops = {
        .paint = paint,
        .process = 0,
    },
};
