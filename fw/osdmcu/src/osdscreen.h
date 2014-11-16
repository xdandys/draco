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
 * @file    osdscreen.h
 * @brief   screens are containers for OSD widgets
 *
 */

#ifndef OSDSCREEN_H_
#define OSDSCREEN_H_
#include "osdwidget.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WidgetSlot;
typedef struct WidgetSlot {
    struct WidgetSlot *next;
    OsdWidget *widget;
    int x;
    int y;
} WidgetSlot;

typedef struct {
    WidgetSlot *wgSlots;
    OsdPainter *painter;
    bool needRedraw;
    WidgetSlot *slotToRedraw;
    int state;
} OsdScreen;

void osdScreenInit(OsdScreen *screen, OsdPainter *painter);
void osdScreenAddWidget(OsdScreen *screen, OsdWidget *widget, int x, int y);
void osdScreenMoveWidget(OsdScreen *screen, OsdWidget *widget, int x, int y);
void osdScreenRemoveWidget(OsdScreen *screen, OsdWidget *widget);
void osdScreenProcess(OsdScreen *screen, bool forceRedraw);

#ifdef __cplusplus
}
#endif

#endif /* OSDSCREEN_H_ */

/** @} */
