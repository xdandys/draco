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
 * @file    osdscreen.c
 * @brief   brief description here
 *
 */

#include "osdscreen.h"
#ifdef OSD_SIMULATOR
#include <stdio.h>
#else
#include "debug.h"
#endif


#define MAX_SLOT_COUNT                  16

#define SCREEN_STATE_SCANNING           0
#define SCREEN_STATE_REDRAWING          1
#define SCREEN_STATE_REDRAW_WAIT_SYNC   2

static WidgetSlot slotPool[MAX_SLOT_COUNT];


#define SLOT_FOREACH(s, h)  for (s = h; s != 0; s = s->next)

static WidgetSlot *allocateSlot(void)
{
    int i;
    for (i = 0; i < MAX_SLOT_COUNT; i++) {
        if (slotPool[i].widget == 0) return &slotPool[i];
    }

    return 0;
}

static void freeSlot(WidgetSlot *slot)
{
    int i;
    for (i = 0; i < MAX_SLOT_COUNT; i++) {
        if (&slotPool[i] == slot) slotPool[i].widget = 0;
    }
}

static void drawSlot(OsdScreen *screen, WidgetSlot *slot)
{
    OsdPainter *painter = screen->painter;
    slot->widget->redrawFlag = 0;
    osdPainterReset(painter);
    if ((slot->widget->cfg->ops.paint) && (osdWidgetIsVisible(slot->widget))) {
        slot->widget->cfg->ops.paint(slot->widget, painter, slot->x, slot->y);
    }    
}

void osdScreenInit(OsdScreen *screen, OsdPainter *painter)
{
    screen->painter = painter;
    screen->wgSlots = 0;
    screen->needRedraw = false;
    screen->state = SCREEN_STATE_SCANNING;
}


void osdScreenAddWidget(OsdScreen *screen, OsdWidget *widget, int x, int y)
{
    WidgetSlot **head =  &screen->wgSlots;
    WidgetSlot *next = 0;
    if (!*head) {
        *head = allocateSlot();
        if (!*head) return;
    } else {
        next = *head;
        *head = allocateSlot();
    }

    if (*head) {
        (*head)->widget = widget;
        (*head)->x = x;
        (*head)->y = y;
        (*head)->next = next;

        osdScreenProcess(screen, true);
    }
}


void osdScreenRemoveWidget(OsdScreen *screen, OsdWidget *widget)
{

    WidgetSlot *slot = 0;
    WidgetSlot *prevSlot = 0;
    SLOT_FOREACH(slot, screen->wgSlots) {
        if (slot->widget == widget) {
            if (prevSlot) {
                prevSlot->next = slot->next;
            } else {
                screen->wgSlots = 0;
            }
            freeSlot(slot);
        }
        prevSlot = slot;
    }
    screen->state = SCREEN_STATE_REDRAW_WAIT_SYNC;
}


void osdScreenMoveWidget(OsdScreen *screen, OsdWidget *widget, int x, int y)
{
    WidgetSlot *slot = 0;
    SLOT_FOREACH(slot, screen->wgSlots) {
        if (slot->widget == widget) {
            slot->x = x;
            slot->y = y;
        }
    }
    screen->state = SCREEN_STATE_REDRAW_WAIT_SYNC;
}

void osdScreenProcess(OsdScreen *screen, bool forceRedraw)
{
// TODO: in future it would be nice to have selective redrawing according
//       widget boundaries and collisions with other widgets.
//       Not sure about real effectivity of such solution.


    WidgetSlot *slot = 0;
    if (forceRedraw) screen->state = SCREEN_STATE_REDRAW_WAIT_SYNC;
    SLOT_FOREACH(slot, screen->wgSlots) {
        const WidgetConfig *wcfg = slot->widget->cfg;
        if (wcfg) {            
            if (osdWidgetRedrawNeeded(slot->widget)) screen->needRedraw = true;
            if (wcfg->ops.process) wcfg->ops.process(slot->widget);            
        }        
    }

    if ((screen->state == SCREEN_STATE_SCANNING) && (screen->needRedraw)) {
        screen->state = SCREEN_STATE_REDRAW_WAIT_SYNC;
    }

    if (screen->state == SCREEN_STATE_REDRAW_WAIT_SYNC) {
        if (screen->painter->ready) {
            osdPainterClear(screen->painter);
            screen->state = SCREEN_STATE_REDRAWING;
            screen->slotToRedraw = screen->wgSlots;
        }
    }

    if (screen->state == SCREEN_STATE_REDRAWING) {
        // every pass draw just one widget and yield control to main loop
        if (screen->slotToRedraw) {
            drawSlot(screen, screen->slotToRedraw);
            screen->slotToRedraw = screen->slotToRedraw->next;
        }

        if (!screen->slotToRedraw) {
            screen->state = SCREEN_STATE_SCANNING;
            screen->needRedraw = false;
            osdPainterFlush(screen->painter);
        }
    }
}
