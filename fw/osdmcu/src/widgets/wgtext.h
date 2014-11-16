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
 * @file    wgtext.h
 * @brief   abstract text widget
 *
 */

// WARN: this is not standalone OsdWidget, it's just used by other widgets

#ifndef WGTEXT_H_
#define WGTEXT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "osdwidget.h"
#include "osdpainter.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const OsdBitmap *icon;
    const OsdFont *font;
    const char *text;
    bool blinkEn;
    bool blinkShow;
    uint16_t blinkInterval;
    uint16_t blinkRetries;
    uint32_t blinkLast;
} WgText;

void wgTextInit(WgText *wg, const char *iconName, const char *fontName);
void wgTextSetIconBitmap(WgText *wg, const char *name);
void wgTextBlink(WgText *wg, bool enable, uint16_t interval, uint16_t retries);
void wgTextSetFont(WgText *wg, const char *name);
void wgTextSetText(WgText *wg, const char *text);

// intended to be call inside corresponding OsdWidget ops of parent widget
bool wgTextProcees(WgText *wg);
void wgTextPaint(WgText *wg, OsdPainter *painter, int x, int y);

#ifdef __cplusplus
}
#endif
#endif /* WGTEXT_H_ */

/** @} */
