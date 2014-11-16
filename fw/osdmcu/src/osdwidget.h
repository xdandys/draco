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
 * @file    osdwidget.h
 * @brief   OSD abstract widget
 *
 */

#ifndef OSDWIDGET_H_
#define OSDWIDGET_H_

#include <stdint.h>
#include <stdbool.h>
#include "osdpainter.h"
#define WIDGET_PARENT(w)    ((w)->widget)

typedef struct {
    void (*paint)(void *widget, OsdPainter *painter, int x, int y);
    void (*process)(void *widget);
} WidgetOps;

typedef struct {
    WidgetOps ops;
    int boundingWidth;
    int boudingHeight;
} WidgetConfig;

typedef struct {
    const WidgetConfig *cfg;
    bool redrawFlag;
    bool visible;
} OsdWidget;

inline static void osdWidgetRedraw(OsdWidget *widget)
{    
    if (widget->visible)
        widget->redrawFlag = true;
}

inline static void osdWidgetInit(OsdWidget *widget)
{
    widget->redrawFlag = 0;
    widget->cfg = 0;
    widget->visible = true;
}

inline static bool osdWidgetRedrawNeeded(const OsdWidget *widget)
{
    return widget->redrawFlag;
}

inline static void osdWidgetSetCfg(OsdWidget *widget, const WidgetConfig *cfg)
{
    widget->cfg = cfg;
    osdWidgetRedraw(widget);
}

inline static void osdWidgetSetVisible(OsdWidget *widget, bool visible)
{
    if (widget->visible != visible) {
        widget->visible = visible;
        widget->redrawFlag = true;
    }
}

inline static bool osdWidgetIsVisible(const OsdWidget *widget)
{
    return widget->visible;
}

#endif /* OSDWIDGET_H_ */

/** @} */
