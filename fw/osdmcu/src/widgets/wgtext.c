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
 * @file    wgtext.c
 * @brief   abstract text widget
 *
 */

#include "wgtext.h"
#include "utils.h"

static const PainterPen pen = {
    .color = PAINTER_COLOR_WHITE,
    .outline = 1,
    .style = PAINTER_PEN_STYLE_SOLID,
};

bool wgTextProcees(WgText *wg)
{
    bool ret = false;
    if (wg->blinkEn) {
        if ((elapsedMs() - wg->blinkLast) >= wg->blinkInterval) {
            wg->blinkLast = elapsedMs();
            if (wg->blinkRetries == 1) {
                wg->blinkEn = false;
                wg->blinkShow = true;
            } else {
                if (wg->blinkRetries) wg->blinkRetries--;
                if (wg->blinkShow) wg->blinkShow = false; else wg->blinkShow = true;
                ret = true;
            }
        }
    }
    return ret;
}

void wgTextPaint(WgText *wg, OsdPainter *painter, int x, int y)
{
    if ((!wg->blinkShow) && (wg->blinkEn)) return;

    osdPainterTranslate(painter, x, y);
    osdPainterSetPen(painter, &pen);
    osdPainterSetFont(painter, wg->font);
    int width = 0, height = 0;
    osdPainterTextBounds(painter, wg->text, &width, &height);
    osdPainterDrawText(painter, 0, 0, wg->text);

    if (wg->icon) {
        int iconx = -wg->icon->width - 2;
        int icony = (height / 2) - wg->icon->height / 2;
        osdPainterDrawBitmap(painter, iconx, icony, wg->icon);
    }
}

void wgTextInit(WgText *wg, const char *iconName, const char *fontName)
{
    wg->font = 0;
    wg->icon = 0;
    wg->text = 0;
    wg->blinkEn = false;
    wg->blinkShow = true;
    wgTextSetFont(wg, fontName);
    wgTextSetIconBitmap(wg, iconName);
}

void wgTextSetText(WgText *wg, const char *text)
{
    wg->text = text;
}

void wgTextSetFont(WgText *wg, const char *name)
{
    wg->font = osdFontByName(name);
}

void wgTextSetIconBitmap(WgText *wg, const char *name)
{
    wg->icon = osdBitmapByName(name);
}

void wgTextBlink(WgText *wg, bool enable, uint16_t interval, uint16_t retries)
{
    wg->blinkInterval = interval;
    wg->blinkRetries = retries;
    if (!wg->blinkEn)
        wg->blinkShow = true;
    wg->blinkEn = enable;
}
