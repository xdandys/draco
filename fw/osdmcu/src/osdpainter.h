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
 * @file    osdpainter.h
 * @brief   painting routines
 *
 */

#ifndef OSDPAINTER_H_
#define OSDPAINTER_H_
#include "osddevice.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PAINTER_COLOR_BLACK         0
#define PAINTER_COLOR_WHITE         1
#define PAINTER_COLOR_TRANSPARENT   2

#define PAINTER_BRUSH_STYLE_NONE    0
#define PAINTER_BRUSH_STYLE_SOLID   1

#define PAINTER_PEN_STYLE_NONE      0
#define PAINTER_PEN_STYLE_SOLID     1

#define PAINTER_BITMAP_MODE_MASK            0x01
#define PAINTER_BITMAP_MODE_WHITE           0x01

#define PAINTER_TEXT_ALIGN_LEFT             0x00
#define PAINTER_TEXT_ALIGN_CENTER           0x01
#define PAINTER_TEXT_ALIGN_RIGHT            0x02
#define PAINTER_TEXT_VALIGN_TOP             0x00
#define PAINTER_TEXT_VALIGN_CENTER          0x10
#define PAINTER_TEXT_VALIGN_DOWN            0x20

typedef struct {
    uint8_t style;
    uint8_t color;
}PainterBrush;

typedef struct {
    uint8_t style;
    uint8_t width;  /**< TODO, width ignored currently */
    uint8_t color;
    uint8_t outline;
}PainterPen;

typedef struct {
    const char *name;
    int lineSpace;
    int width;
    int height;
    const uint8_t *bodyBuff;
    const uint8_t *maskBuff;
    int rangeMin;
    int rangeMax;
} OsdFont;

typedef struct {
    const char *name;
    int width;
    int height;
    const uint8_t *buff;
    const uint8_t *outlineBuff;
}OsdBitmap;

typedef struct {
    OsdDevice *device;
    int ready;
    uint16_t hres;
    uint16_t vres;
    uint32_t *level;
    uint32_t *mask;

    float matrix[4];
    int translateX;
    int translateY;

    PainterBrush brush;
    PainterPen pen;
    const OsdFont *font;
    int clipXmin;
    int clipYmin;
    int clipXmax;
    int clipYmax;

}OsdPainter;

void osdPainterInit(OsdPainter *painter);
void osdPainterSetDevice(OsdPainter *painter, OsdDevice *device);
void osdPainterReset(OsdPainter *painter);
void osdPainterFlush(OsdPainter *painter);
static inline bool osdPainterIsReady(OsdPainter *painter) {return painter->ready != 0;}

void osdPainterSetBrush(OsdPainter *painter, const PainterBrush *brush);
void osdPainterSetPen(OsdPainter *painter, const PainterPen *pen);
void osdPainterGetBrush(OsdPainter *painter, PainterBrush *brush);
void osdPainterGetPen(OsdPainter *painter, PainterPen *pen);
const OsdFont *osdFontByName(const char *name);
const OsdBitmap *osdBitmapByName(const char *name);

void osdPainterSetFont(OsdPainter *painter, const OsdFont *font);

void osdPainterTransform(OsdPainter *painter, float *matrix);
void osdPainterRotate(OsdPainter *painter, float angle);
void osdPainterTranslate(OsdPainter *painter, int x, int y);
void osdPainterTranslateOrigin(OsdPainter *painter);
void osdPainterSetClipping(OsdPainter *painter, int xmin, int ymin, int xmax, int ymax);
void osdPainterResetClipping(OsdPainter *painter);
void osdPainterIdentity(OsdPainter *painter);

void osdPainterClear(OsdPainter *painter);

void osdPainterDrawPixel(OsdPainter *painter, int x, int y);
void osdPainterDrawLine(OsdPainter *painter, int x0, int y0, int x1, int y1);
void osdPainterDrawLines(OsdPainter *painter, int *points, uint8_t count);
void osdPainterDrawRectangle(OsdPainter *painter, int x0, int y0, int x1, int y1);
void osdPainterDrawPoly(OsdPainter *painter, const int *points, uint8_t count);
void osdPainterDrawCircle(OsdPainter *painter, int x, int y, int radius);

void osdPainterDrawBitmap(OsdPainter *painter, int x, int y, const OsdBitmap *bitmap);
void osdPainterDrawText(OsdPainter *painter, int x, int y, const char *text);
void osdPainterTextBounds(OsdPainter *painter, const char *text, int *width, int *height);
void osdPainterDrawTextAligned(OsdPainter *painter, int x, int y, int width, int height,
                               int alignment, const char *text);

#ifdef __cplusplus
}
#endif

#endif /* OSDPAINTER_H_ */

/** @} */
