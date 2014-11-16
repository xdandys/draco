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
 * @file    osdpainter.c
 * @brief   painting algorithms
 *
 */

#include "osdpainter.h"
#include <string.h>
#include <math.h>
#ifdef OSD_SIMULATOR
#include <stdio.h>
#else
#include "debug.h"
#endif

#define PI  3.14159265359f
#define ABS(a)              (((a) > 0) ? (a) : -(a))
#define DEG2RAD             (PI / 180.0f)
#define SWAP_INTS(a,b)      do {int tmp = (a); (a) = (b); (b) = tmp;} while(0);

struct EdgeState {
    struct EdgeState *nextEdge;
    int x;
    int startY;
    int wholePixelXMove;
    int xDirection;
    int errorTerm;
    int errorTermAdjUp;
    int errorTermAdjDown;
    int count;
};

struct EdgeState edgeStatePool[8];

extern const OsdFont *const osdFontsList[];
extern const OsdBitmap *const osdBitmapsList[];

static void buildGet(const int *points, int pointsCount, struct EdgeState * NextFreeEdgeStruc);
static void moveXSortedToAet(int);
static void scanOutAet(OsdPainter *painter, int yToScan);
static void advanceAet(void);
static void xSortAet(void);
static void transformPoint(OsdPainter *painter, int *x, int *y);

//------------------------------------------------------------------------
//
//         osdDevice bindings
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

static void bufferSwapped(void *ctx, uint32_t *level, uint32_t *mask)
{
    OsdPainter *painter = (OsdPainter*)ctx;
    painter->level = level;
    painter->mask = mask;
    painter->ready = 1;
    // printf("buffer swapped\n");
    // dprint("buffer swapped");

}

void osdPainterFlush(OsdPainter *painter)
{
    painter->ready = 0;
    osdDeviceSwapBuffers(painter->device);
    // printf("osdPainterFlush\n");
}

//------------------------------------------------------------------------
//
//         initialization
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

static void resetPenBrush(OsdPainter *painter)
{
    painter->pen.color = PAINTER_COLOR_WHITE;
    painter->pen.width = 1;
    painter->pen.outline = 1;
    painter->pen.style = PAINTER_PEN_STYLE_SOLID;

    painter->brush.style = PAINTER_BRUSH_STYLE_NONE;
    painter->brush.color = PAINTER_COLOR_WHITE;
    painter->font = osdFontsList[0];
}

void osdPainterInit(OsdPainter *painter)
{
    painter->ready = 0;
    resetPenBrush(painter);
    osdPainterIdentity(painter);
}

void osdPainterSetDevice(OsdPainter *painter, OsdDevice *device)
{
    painter->device = device;
    osdDeviceRegisterBufferSwappedCb(device, bufferSwapped, painter);
    painter->hres = osdDeviceGetHRes(device);
    painter->vres = osdDeviceGetVRes(device);
    painter->level = osdDeviceGetLevelBackBuffer(device);
    painter->mask = osdDeviceGetMaskBackBuffer(device);
    osdPainterClear(painter);
    osdPainterSetClipping(painter, 0, 0, painter->hres - 1, painter->vres - 1);

    painter->ready = 1;
}

void osdPainterReset(OsdPainter *painter)
{
    if (!painter->ready) return;
    resetPenBrush(painter);
    osdPainterIdentity(painter);
    osdPainterTranslateOrigin(painter);
    osdPainterResetClipping(painter);
}

//------------------------------------------------------------------------
//
//         transformations
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void osdPainterTransform(OsdPainter *painter, float *matrix)
{
    float newmatrix[4];
    newmatrix[0] = painter->matrix[0] * matrix[0] + painter->matrix[1] * matrix[2];
    newmatrix[1] = painter->matrix[0] * matrix[1] + painter->matrix[1] * matrix[3];
    newmatrix[2] = painter->matrix[2] * matrix[0] + painter->matrix[3] * matrix[2];
    newmatrix[3] = painter->matrix[2] * matrix[1] + painter->matrix[3] * matrix[3];

    painter->matrix[0] = newmatrix[0];
    painter->matrix[1] = newmatrix[1];
    painter->matrix[2] = newmatrix[2];
    painter->matrix[3] = newmatrix[3];
}

void osdPainterRotate(OsdPainter *painter, float angle)
{
    angle = DEG2RAD * angle;
    float c = cosf(angle);
    float s = sinf(angle);

    float matrix[4];
    matrix[0] = c;
    matrix[1] = -s;
    matrix[2] = s;
    matrix[3] = c;

    osdPainterTransform(painter, matrix);
}

void osdPainterTranslate(OsdPainter *painter, int x, int y)
{
    painter->translateX += x;
    painter->translateY += y;
}

void osdPainterTranslateOrigin(OsdPainter *painter)
{
    painter->translateX = 0;
    painter->translateY = 0;
}

void osdPainterSetClipping(OsdPainter *painter, int x0, int y0, int x1, int y1)
{
    transformPoint(painter, &x0, &y0);
    transformPoint(painter, &x1, &y1);

    painter->clipXmin = (x0 < 0) ? 0 : x0;
    painter->clipXmin = (x0 > (painter->hres - 1)) ?  painter->hres : x0;
    painter->clipXmax = (x1 < 0) ? 0 : x1;
    painter->clipXmax = (x1 > (painter->hres - 1)) ?  painter->hres : x1;

    painter->clipYmin = (y0 < 0) ? 0 : y0;
    painter->clipYmin = (y0 > (painter->vres - 1)) ?  painter->vres : y0;
    painter->clipYmax = (y1 < 0) ? 0 : y1;
    painter->clipYmax = (y1 > (painter->vres - 1)) ?  painter->vres : y1;
}

void osdPainterResetClipping(OsdPainter *painter)
{
    painter->clipXmin = 0;
    painter->clipXmax = painter->hres - 1;
    painter->clipYmin = 0;
    painter->clipYmax = painter->vres - 1;
}

void osdPainterIdentity(OsdPainter *painter)
{
    painter->matrix[0] = 1.0f;
    painter->matrix[1] = 0.0f;
    painter->matrix[2] = 0.0f;
    painter->matrix[3] = 1.0f;
}

static inline int clipPoint(OsdPainter *painter, int *x, int *y)
{
    int ret = 0;
    if (*x < painter->clipXmin) {
        *x = (int)painter->clipXmin;
        ret = 1;
    }

    if (*x > painter->clipXmax) {
        *x = (int)painter->clipXmax;
        ret = 1;
    }

    if (*y < painter->clipXmin) {
        *y = (int)painter->clipYmin;
        ret = 1;
    }

    if (*y > painter->clipYmax) {
        *y = (int)painter->clipYmax;
        ret = 1;
    }
    return ret;
}

static void transformPoint(OsdPainter *painter, int *x, int *y)
{
    if ((painter->matrix[0] != 1.0f) || (painter->matrix[1] != 0.0f) ||
            (painter->matrix[2] != 0.0f) || (painter->matrix[3] != 1.0f)) {

        float xt = painter->matrix[0] * *x + painter->matrix[1] * *y;
        float yt = painter->matrix[2] * *x + painter->matrix[3] * *y;

        *x = (int) xt;
        *y = (int) yt;
    }

    *x += painter->translateX;
    *y += painter->translateY;
}

//------------------------------------------------------------------------
//
//         setters / getters
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void osdPainterSetBrush(OsdPainter *painter, const PainterBrush *brush)
{
    memcpy(&painter->brush, brush, sizeof(PainterBrush));
}

void osdPainterSetPen(OsdPainter *painter, const PainterPen *pen)
{
    memcpy(&painter->pen, pen, sizeof(PainterPen));
}

void osdPainterGetBrush(OsdPainter *painter, PainterBrush *brush)
{
    memcpy(brush, &painter->brush, sizeof(PainterBrush));
}

void osdPainterGetPen(OsdPainter *painter, PainterPen *pen)
{
    memcpy(pen, &painter->pen, sizeof(PainterPen));
}

const OsdBitmap *osdBitmapByName(const char *name)
{
    int i;
    for (i = 0; osdBitmapsList[i] != 0; i++) {
        const OsdBitmap * bitmap = osdBitmapsList[i];
        if (strcmp(name, bitmap->name) == 0) {
            return bitmap;
        }
    }
    return 0;
}

const OsdFont* osdFontByName(const char *name)
{
    int i;
    for (i = 0; osdFontsList[i] != 0; i++) {
        const OsdFont *font = osdFontsList[i];
        if (strcmp(name, font->name) == 0) {
            return font;
        }
    }
    return 0;
}

void osdPainterSetFont(OsdPainter *painter, const OsdFont *font)
{
    painter->font = font;
}

//------------------------------------------------------------------------
//
//         private drawing
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

static inline int wordAddr(OsdPainter *painter, int x, int y)
{
    return ((painter->hres / 32) * y + x / 32);
}

static inline int bitAddr(int x)
{
    return (x % 32);
}

static inline uint8_t getOutlineColor(OsdPainter * painter)
{
    return (painter->pen.color == PAINTER_COLOR_BLACK) ? PAINTER_COLOR_WHITE : PAINTER_COLOR_BLACK;
}

static inline void drawBits(OsdPainter *painter, int wordAddr, uint32_t bitmask)
{
    painter->mask[wordAddr] |= bitmask;
    switch(painter->pen.color) {
    case PAINTER_COLOR_BLACK:
        painter->level[wordAddr] &= ~ bitmask;
        break;

    case PAINTER_COLOR_WHITE:
        painter->level[wordAddr] |=  bitmask;
        break;

    case PAINTER_COLOR_TRANSPARENT:
        painter->mask[wordAddr] &= ~ bitmask;
        break;

    default:
        break;
    }
}

static void drawPixel(OsdPainter *painter, int x, int y)
{
    if (clipPoint(painter, &x, &y)) return;

    int word = wordAddr(painter, x, y);
    drawBits(painter, word, 1 << bitAddr(x));
}

static void drawHlineSimple(OsdPainter *painter, int x0, int x1, int y)
{
    if ((y < painter->clipYmin) || (y > painter->clipYmax)) return;

    int startWord = wordAddr(painter, x0, y);
    int endWord = wordAddr(painter, x1, y);

    if (startWord == endWord) {
        uint32_t mask = (((uint64_t)1 << (x1 - x0 + 1)) - 1) << (x0 % 32);
        drawBits(painter, startWord, mask);
    } else {
        uint32_t mask = ~(((uint64_t)1 << (x0 % 32 )) - 1);
        drawBits(painter, startWord, mask);
        mask = (((uint64_t)1 << (x1 % 32 + 1)) - 1);
        drawBits(painter, endWord, mask);
        if (endWord - startWord > 1) {
            int fill;
            for (fill = (startWord + 1); fill < endWord; fill++) {
                drawBits(painter, fill, 0xFFFFFFFF);
            }
        }
    }
}

static void drawHline(OsdPainter *painter, int x0, int x1, int y)
{
    if (x0 > x1)
        SWAP_INTS(x0, x1);

    int clipped0 = 0;
    if (x0 < painter->clipXmin) {
        x0 = painter->clipXmin;
        clipped0 = 1;
    }
    if (x0 > painter->clipXmax) {
        x0 = painter->clipXmax;
        clipped0 = 1;
    }
    if (x1 < painter->clipXmin) {
        x1 = painter->clipXmin;
        if (clipped0) return;
    }
    if (x1 > painter->clipXmax) {
        x1 = painter->clipXmax;
        if (clipped0) return;
    }

    if (painter->pen.outline) {
        uint8_t color = painter->pen.color;
        painter->pen.color = getOutlineColor(painter);
        drawHlineSimple(painter, x0, x1, y - 1);
        drawHlineSimple(painter, x0, x1, y + 1);
        painter->pen.color = color;
    }    

    if (painter->pen.style != PAINTER_PEN_STYLE_NONE)
        drawHlineSimple(painter, x0, x1, y);
}

static void drawVlineSimple(OsdPainter *painter, int y0, int y1, int x)
{
    if ((x < painter->clipXmin) || (x > painter->clipXmax)) return;
    int y;
    for (y = y0; y < (y1 + 1); y++) {
        int word = wordAddr(painter, x, y);
        drawBits(painter, word, 1 << bitAddr(x));
    }
}

static void drawVline(OsdPainter *painter, int y0, int y1, int x)
{
    if (y0 > y1)
        SWAP_INTS(y0, y1);

    int clipped0 = 0;
    if (y0 < painter->clipYmin) {
        y0 = painter->clipYmin;
        clipped0 = 1;
    }
    if (y0 > painter->clipYmax) {
        y0 = painter->clipYmax;
        clipped0 = 1;
    }
    if (y1 < painter->clipYmin) {
        y1 = painter->clipYmin;
        if (clipped0) return;
    }
    if (y1 > painter->clipYmax) {
        y1 = painter->clipYmax;
        if (clipped0) return;
    }

    uint8_t color = painter->pen.color;

    if (painter->pen.outline) {
        painter->pen.color = getOutlineColor(painter);
        drawVlineSimple(painter, y0, y1, x - 1);
        drawVlineSimple(painter, y0, y1, x + 1);
    }
    painter->pen.color = color;

    if (painter->pen.style != PAINTER_PEN_STYLE_NONE)
        drawVlineSimple(painter, y0, y1, x);

}

static void drawMisalignedWord(OsdPainter *painter, int x, int y, uint32_t word, int clipping)
{
    if (clipping) {
        if ((x > painter->clipXmax) || ((x + 31) < painter->clipXmin))
            return;

        if ((y > painter->clipYmax) || (y < painter->clipYmin))
            return;

        if (x < painter->clipXmin) {
            uint8_t diff = painter->clipXmin - x;
            word >>= diff;
            x += diff;
        }

        if ((x + 31) > painter->clipXmax) {
            uint8_t diff = (x + 31) - painter->clipXmax;
            word &= (uint32_t)0xffffffff >> diff;
        }
    }

    uint32_t addr = wordAddr(painter, x, y);
    uint32_t left = word << (x % 32);
    uint32_t right = word >> (32 - (x % 32));
    if (x % 32 == 0) right = 0;

    if (left)
        drawBits(painter, addr, left);
    if (right)
        drawBits(painter, addr + 1, right);
}

static void bitblit(OsdPainter *painter, const uint8_t *src, int width, int height, int x, int y)
{
    int bytesPerLine = (width - 1) / 8 + 1;
    int line = 0, yp = 0, clipping = 0;

    if ((y < painter->clipYmin) || ((y + height -1) > painter->clipYmax)) {
        clipping = 1;
    }

    if ((x < painter->clipXmin) || ((x + width -1) > painter->clipXmax)) {
        clipping = 1;
    }

    for (line = 0, yp = 0; line < height; line++, yp += bytesPerLine) {
        int xp;
        uint32_t word = 0;
        for (xp = 0; xp < bytesPerLine; xp++) {
            if ((xp != 0) && ((xp % 4) == 0)) {
                drawMisalignedWord(painter,x + ((xp - 1) / 4) * 32, y + line, word, clipping);
                word = 0;
            }
            word >>= 8;
            word |= src[yp + xp] << 24;
        }
        if (width % 8) word &= ~(0xff000000 << (width % 8));
        word >>= (8 *  (4 - (xp % 4)));
        drawMisalignedWord(painter,x + (((xp - 1) / 4))  * 32, y + line, word, clipping);
    }
}

static const char* drawTextOneLine(OsdPainter *painter, int x, int y, const char *text)
{
    int p = 0;
    const OsdFont *font = painter->font;
    int bodyBytesPerChar = ((font->width - 1) / 8 + 1) * font->height;
    int outlineBytesPerChar = ((font->width + 2 - 1) / 8 + 1) * (font->height + 2);    
    while((text[p] != 0) && (text[p] != '\n')) {
        int ci = text[p] - font->rangeMin;
        if ((ci >= 0) && (p <= font->rangeMax)) {
            // draw background if brush is selected
            if (painter->brush.style) {
                uint8_t penColor = painter->pen.color;
                uint8_t penStyle = painter->pen.style;
                uint8_t penOutline = painter->pen.outline;
                painter->pen.style = PAINTER_PEN_STYLE_SOLID;
                painter->pen.color = painter->brush.color;
                painter->pen.outline = 0;

                int yl;
                for(yl = y; yl < (y + painter->font->height + painter->font->lineSpace); yl++) {
                    drawHline(painter, x, x + painter->font->width - 1 , yl);
                }
                painter->pen.color = penColor;
                painter->pen.style = penStyle;
                painter->pen.outline = penOutline;
            }
            // draw outline
            if (painter->pen.outline) {
                uint8_t penColor = painter->pen.color;
                painter->pen.color = getOutlineColor(painter);
                bitblit(painter, &font->maskBuff[ci * outlineBytesPerChar], font->width + 2,
                        font->height + 2, x - 1, y - 1);
                painter->pen.color = penColor;
            }            
            bitblit(painter, &font->bodyBuff[ci * bodyBytesPerChar], font->width,
                    font->height, x, y);
        }
        p++;
        x += font->width;
    }    
    return &text[p];
}

//------------------------------------------------------------------------
//
//         polygon filling routines
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

/* Pointers to global edge table (GET) and active edge table (AET) */
static struct EdgeState *getPtr, *aetPtr;

void fillPolygon(OsdPainter *painter, const int *points, int pointsCount)
{
    struct EdgeState *edgeTableBuffer;
    int currentY;

    /* It takes a minimum of 3 vertices to cause any pixels to be
       drawn; reject polygons that are guaranteed to be invisible */
    if (pointsCount < 3)
        return;
    /* Get enough memory to store the entire edge table */
    if ((sizeof(edgeStatePool) / sizeof(struct EdgeState)) < (unsigned int)pointsCount)
        return;

    uint8_t pencolor = painter->pen.color;
    uint8_t penstyle = painter->pen.style;
    uint8_t penoutline = painter->pen.outline;
    painter->pen.color = painter->brush.color;
    painter->pen.style = PAINTER_PEN_STYLE_SOLID;
    painter->pen.outline = 0;

    edgeTableBuffer = edgeStatePool;
    /* Build the global edge table */
    buildGet(points, pointsCount, edgeTableBuffer);
    /* Scan down through the polygon edges, one scan line at a time,
       so long as at least one edge remains in either the GET or AET */
    aetPtr = 0;    /* initialize the active edge table to empty */
    currentY = getPtr->startY; /* start at the top polygon vertex */
    while ((getPtr != 0) || (aetPtr != 0)) {
        moveXSortedToAet(currentY);  /* update AET for this scan line */
        scanOutAet(painter, currentY); /* draw this scan line from AET */
        advanceAet();                /* advance AET edges 1 scan line */
        xSortAet();                  /* resort on X */
        currentY++;                  /* advance to the next scan line */
    }

    painter->pen.color = pencolor;
    painter->pen.style = penstyle;
    painter->pen.outline = penoutline;

    return;
}

/* Creates a GET in the buffer pointed to by NextFreeEdgeStruc from
    the vertex list. Edge endpoints are flipped, if necessary, to
    guarantee all edges go top to bottom. The GET is sorted primarily
    by ascending Y start coordinate, and secondarily by ascending X
    start coordinate within edges with common Y coordinates. */
static void buildGet(const int *points, int pointsCount,
        struct EdgeState * NextFreeEdgeStruc)
{
    int i, startX, startY, endX, endY, deltaY, deltaX, width;
    struct EdgeState *newEdgePtr;
    struct EdgeState *followingEdge, **followingEdgeLink;

    /* Scan through the vertex list and put all non-0-height edges into
       the GET, sorted by increasing Y start coordinate */
    getPtr = 0;    /* initialize the global edge table to empty */
    for (i = 0; i < pointsCount; i++) {
        /* Calculate the edge height and width */
        startX = points[i * 2];
        startY = points[i * 2 + 1];
        /* The edge runs from the current point to the previous one */
        if (i == 0) {
            /* Wrap back around to the end of the list */
            endX = points[(pointsCount - 1) * 2];
            endY = points[(pointsCount - 1) * 2 + 1];
        } else {
            endX = points[(i - 1) * 2];
            endY = points[(i - 1) * 2 + 1];
        }
        /* Make sure the edge runs top to bottom */
        if (startY > endY) {
            SWAP_INTS(startX, endX);
            SWAP_INTS(startY, endY);
        }
        /* Skip if this can’t ever be an active edge (has 0 height) */
        if ((deltaY = endY - startY) != 0) {
            /* Allocate space for this edge’s info, and fill in the
             structure */
            newEdgePtr = NextFreeEdgeStruc++;
            newEdgePtr->xDirection =   /* direction in which X moves */
                    ((deltaX = endX - startX) > 0) ? 1 : -1;
            width = ABS(deltaX);
            newEdgePtr->x = startX;
            newEdgePtr->startY = startY;
            newEdgePtr->count = deltaY;
            newEdgePtr->errorTermAdjDown = deltaY;
            if (deltaX >= 0)  /* initial error term going L->R */
                newEdgePtr->errorTerm = 0;
            else              /* initial error term going R->L */
                newEdgePtr->errorTerm = -deltaY + 1;
            if (deltaY >= width) {     /* Y-major edge */
                newEdgePtr->wholePixelXMove = 0;
                newEdgePtr->errorTermAdjUp = width;
            } else {                   /* X-major edge */
                newEdgePtr->wholePixelXMove =
                        (width / deltaY) * newEdgePtr->xDirection;
                newEdgePtr->errorTermAdjUp = width % deltaY;
            }
            /* Link the new edge into the GET so that the edge list is
             still sorted by Y coordinate, and by X coordinate for all
             edges with the same Y coordinate */
            followingEdgeLink = &getPtr;
            for (;;) {
                followingEdge = *followingEdgeLink;
                if ((followingEdge == 0) ||
                        (followingEdge->startY > startY) ||
                        ((followingEdge->startY == startY) &&
                                (followingEdge->x >= startX))) {
                    newEdgePtr->nextEdge = followingEdge;
                    *followingEdgeLink = newEdgePtr;
                    break;
                }
                followingEdgeLink = &followingEdge->nextEdge;
            }
        }
    }
}

/* Sorts all edges currently in the active edge table into ascending
    order of current X coordinates */
static void xSortAet(void) {
    struct EdgeState *currentEdge, **currentEdgePtr, *tempEdge;
    int swapOccurred;

    /* Scan through the AET and swap any adjacent edges for which the
       second edge is at a lower current X coord than the first edge.
       Repeat until no further swapping is needed */
    if (aetPtr != 0) {
        do {
            swapOccurred = 0;
            currentEdgePtr = &aetPtr;
            while ((currentEdge = *currentEdgePtr)->nextEdge != 0) {
                if (currentEdge->x > currentEdge->nextEdge->x) {
                    /* The second edge has a lower X than the first;
                   swap them in the AET */
                    tempEdge = currentEdge->nextEdge->nextEdge;
                    *currentEdgePtr = currentEdge->nextEdge;
                    currentEdge->nextEdge->nextEdge = currentEdge;
                    currentEdge->nextEdge = tempEdge;
                    swapOccurred = 1;
                }
                currentEdgePtr = &(*currentEdgePtr)->nextEdge;
            }
        } while (swapOccurred != 0);
    }
}

/* Advances each edge in the AET by one scan line.
    Removes edges that have been fully scanned. */
static void advanceAet(void) {
    struct EdgeState *currentEdge, **currentEdgePtr;

    /* Count down and remove or advance each edge in the AET */
    currentEdgePtr = &aetPtr;
    while ((currentEdge = *currentEdgePtr) != 0) {
        /* Count off one scan line for this edge */
        if ((--(currentEdge->count)) == 0) {
            /* This edge is finished, so remove it from the AET */
            *currentEdgePtr = currentEdge->nextEdge;
        } else {
            /* Advance the edge’s X coordinate by minimum move */
            currentEdge->x += currentEdge->wholePixelXMove;
            /* Determine whether it’s time for X to advance one extra */
            if ((currentEdge->errorTerm +=
                    currentEdge->errorTermAdjUp) > 0) {
                currentEdge->x += currentEdge->xDirection;
                currentEdge->errorTerm -= currentEdge->errorTermAdjDown;
            }
            currentEdgePtr = &currentEdge->nextEdge;
        }
    }
}

/* Moves all edges that start at the specified Y coordinate from the
    GET to the AET, maintaining the X sorting of the AET. */
static void moveXSortedToAet(int yToMove) {
    struct EdgeState *aetEdge, **aetEdgePtr, *tempEdge;
    int currentX;

    /* The GET is Y sorted. Any edges that start at the desired Y
       coordinate will be first in the GET, so we’ll move edges from
       the GET to AET until the first edge left in the GET is no longer
       at the desired Y coordinate. Also, the GET is bX sorted within
       each Y coordinate, so each successive edge we add to the AET is
       guaranteed to belong later in the AET than the one just added. */
    aetEdgePtr = &aetPtr;
    while ((getPtr != 0) && (getPtr->startY == yToMove)) {
        currentX = getPtr->x;
        /* Link the new edge into the AET so that the AET is still
          sorted by X coordinate */
        for (;;) {
            aetEdge = *aetEdgePtr;
            if ((aetEdge == 0) || (aetEdge->x >= currentX)) {
                tempEdge = getPtr->nextEdge;
                *aetEdgePtr = getPtr;  /* link the edge into the AET */
                getPtr->nextEdge = aetEdge;
                aetEdgePtr = &getPtr->nextEdge;
                getPtr = tempEdge;   /* unlink the edge from the GET */
                break;
            } else {
                aetEdgePtr = &aetEdge->nextEdge;
            }
        }
    }
}

/* Fills the scan line described by the current AET at the specified Y
    coordinate in the specified color, using the odd/even fill rule */
static void scanOutAet(OsdPainter *painter, int yToScan) {
    int leftX;
    struct EdgeState *currentEdge;

    /* Scan through the AET, drawing line segments as each pair of edge
       crossings is encountered. The nearest pixel on or to the right
       of left edges is drawn, and the nearest pixel to the left of but
       not on right edges is drawn */
    currentEdge = aetPtr;
    while (currentEdge != 0) {
        leftX = currentEdge->x;
        currentEdge = currentEdge->nextEdge;
        osdPainterDrawLine(painter, leftX, yToScan, currentEdge->x - 1, yToScan);
        currentEdge = currentEdge->nextEdge;
    }
}

static void fillCircle(OsdPainter *painter, int x0, int y0, int radius)
{
    int x = radius, y = 0;
    int radiusError = 1 - x;

    uint8_t penOutline = painter->pen.outline;
    uint8_t penColor = painter->pen.color;
    uint8_t penStyle = painter->pen.style;
    painter->pen.color = painter->brush.color;
    painter->pen.outline = 0;
    painter->pen.style = PAINTER_PEN_STYLE_SOLID;

    while(x >= y)
    {
        // LUU - RUU
        drawHline(painter, -y + x0, y + x0, -x + y0);
        // LUD - RUD
        drawHline(painter, -x + x0, x + x0, -y + y0);
        // LDU - RDU
        drawHline(painter, -x + x0, x + x0, y + y0);
        // LDD - RDD
        drawHline(painter, -y + x0, y + x0, x + y0);

        y++;
        if (radiusError<0)
        {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError+= 2 * (y - x + 1);
        }
    }
    painter->pen.color = penColor;
    painter->pen.outline = penOutline;
    painter->pen.style = penStyle;
}

static void outlineCircle(OsdPainter *painter, int x0, int y0, int radius)
{
    int x = radius, y = 0;
    int radiusError = 1 - x;

    uint8_t penOutline = painter->pen.outline;
    uint8_t penColor = painter->pen.color;
    uint8_t penStyle = painter->pen.style;
    painter->pen.color = getOutlineColor(painter);
    painter->pen.outline = 0;
    painter->pen.style = PAINTER_PEN_STYLE_SOLID;

    while(x >= y)
    {
        // inner ring
        drawPixel(painter, x + x0 - 1, y + y0);  // RDU
        drawPixel(painter, y + x0, x + y0 - 1); // RDD
        drawPixel(painter, -x + x0 + 1, y + y0); // LDU
        drawPixel(painter, -y + x0, x + y0 - 1); // LDD
        drawPixel(painter, -x + x0 + 1, -y + y0); // LUD
        drawPixel(painter, -y + x0, -x + y0 + 1); // LUU
        drawPixel(painter, x + x0 - 1, -y + y0); // RUD
        drawPixel(painter, y + x0, -x + y0 + 1); // RUU

        // outer ring
        drawPixel(painter, x + x0 + 1, y + y0);  // RDU
        drawPixel(painter, y + x0, x + y0 + 1); // RDD
        drawPixel(painter, -x + x0 - 1, y + y0); // LDU
        drawPixel(painter, -y + x0, x + y0 + 1); // LDD
        drawPixel(painter, -x + x0 - 1, -y + y0); // LUD
        drawPixel(painter, -y + x0, -x + y0 - 1); // LUU
        drawPixel(painter, x + x0 + 1, -y + y0); // RUD
        drawPixel(painter, y + x0, -x + y0 - 1); // RUU

        y++;
        if (radiusError<0)
        {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError+= 2 * (y - x + 1);
        }
    }
    painter->pen.color = penColor;
    painter->pen.outline = penOutline;
    painter->pen.style = penStyle;
}

//------------------------------------------------------------------------
//
//         public drawing - primitives
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void osdPainterClear(OsdPainter *painter)
{
    int size = (painter->hres * painter->vres) / 32;

    int i = 0;
    for (i = 0; i < size; i++) {
        // painter->level[i] = 0;
        painter->mask[i] = 0x00;
    }
}

void osdPainterDrawPixel(OsdPainter *painter, int x, int y)
{
    transformPoint(painter, &x, &y);
    drawPixel(painter, x, y);
}

void osdPainterDrawLine(OsdPainter *painter, int x0, int y0, int x1, int y1)
{
    if (!painter->ready) return;

    transformPoint(painter, &x0, &y0);
    transformPoint(painter, &x1, &y1);

    if (y0 == y1) {
        drawHline(painter, x0, x1, y0);
        return;
    }

    if (x0 == x1) {
        drawVline(painter, y0, y1, x0);
        return;
    }

    int steep = ABS(y1 - y0) > ABS(x1 - x0);

    if (steep) {
        SWAP_INTS(x0, y0);
        SWAP_INTS(x1, y1);
    }
    if (x0 > x1) {
        SWAP_INTS(x0, x1);
        SWAP_INTS(y0, y1);
    }
    int y = y0;
    int deltax = x1 - x0;
    int deltay = ABS(y1 - y0);
    int error = deltax / 2;
    int ystep;
    if (y0 < y1) ystep = 1; else ystep = -1;

    int x = x0;
    int xlast = x;
    for (; x < (x1 + 1); x++) {
        error -= deltay;
        if ((error < 0) || (x == x1)) {
            if (steep)
                drawVline(painter, xlast, x, y);
            else
                drawHline(painter, xlast, x, y);
            xlast = x;
            y += ystep;
            error += deltax;
        }
    }
}

void osdPainterDrawRectangle(OsdPainter *painter, int x0, int y0, int x1, int y1)
{
    if (!painter->ready) return;
    uint8_t outline = painter->pen.outline;
    painter->pen.outline = 0;

    transformPoint(painter, &x0, &y0);
    transformPoint(painter, &x1, &y1);

    if (y0 > y1) {
        SWAP_INTS(y0, y1);
        SWAP_INTS(x0, x1);
    }

    if (outline) {
        uint8_t color = painter->pen.color;
        uint8_t penstyle = painter->pen.style;
        painter->pen.color = getOutlineColor(painter);
        painter->pen.style = PAINTER_PEN_STYLE_SOLID;
        drawHline(painter, x0 - 1, x1 + 1, y0 - 1);
        drawHline(painter, x0, x1, y0 + 1);
        drawHline(painter, x0, x1, y1 - 1);
        drawHline(painter, x0 -1 , x1 + 1, y1 + 1);

        drawVline(painter, y0, y1, x0 + 1);
        drawVline(painter, y0, y1, x0 - 1);
        drawVline(painter, y0, y1, x1 + 1);
        drawVline(painter, y0, y1, x1 - 1);
        painter->pen.color = color;
        painter->pen.style = penstyle;
    }

    if (painter->brush.style) {
        uint8_t penColor = painter->pen.color;
        uint8_t penStyle = painter->pen.style;
        uint8_t penOutline = painter->pen.outline;

        painter->pen.style = PAINTER_PEN_STYLE_SOLID;
        painter->pen.color = painter->brush.color;
        painter->pen.outline = 0;

        int y;
        for(y = y0 + 1; y < y1; y++) {
            drawHline(painter, x0, x1, y);
        }
        painter->pen.color = penColor;
        painter->pen.style = penStyle;
        painter->pen.outline = penOutline;
    }

    if (painter->pen.style != PAINTER_PEN_STYLE_NONE) {
        drawHline(painter, x0, x1, y0);
        drawHline(painter, x0, x1, y1);
        drawVline(painter, y0, y1, x0);
        drawVline(painter, y0, y1, x1);
    }

    painter->pen.outline = outline;
}

void osdPainterDrawPoly(OsdPainter *painter, const int *points, uint8_t count)
{
    if ((!painter->ready) || (count < 3)) return;

    const int *pts = points;

    uint8_t outline = painter->pen.outline;
    uint8_t penstyle = painter->pen.style;
    int j;
    for (j = 0; j < 2; j++) {
        // first iteration just outline
        // second iteration brush and then pen

        if ((outline == 0) && (j == 0)) continue;
        if (j == 0) {
            painter->pen.style = PAINTER_PEN_STYLE_NONE;
        } else {
            painter->pen.style = penstyle;
            painter->pen.outline = 0;
        }

        if ((painter->brush.style != PAINTER_BRUSH_STYLE_NONE) && (j == 1)) {
            fillPolygon(painter, points, count);
        }

        int i;
        for (i = 0; i < count; i++) {
            int line[4];
            if (i == count - 1) {
                line[0] = pts[i * 2];
                line[1] = pts[i * 2 + 1];
                line[2] = pts[0];
                line[3] = pts[1];
            } else {
                line[0] = pts[i * 2];
                line[1] = pts[i * 2 + 1];
                line[2] = pts[i * 2 + 2];
                line[3] = pts[i * 2 + 3];
            }
            osdPainterDrawLine(painter, line[0], line[1], line[2], line[3]);
        }
    }

    painter->pen.style = penstyle;
    painter->pen.outline = outline;
}

void osdPainterDrawLines(OsdPainter *painter, int *points, uint8_t count)
{
    if ((!painter->ready) || (count < 2)) return;
    int *pts = points;

    uint8_t outline = painter->pen.outline;
    uint8_t penstyle = painter->pen.style;
    int j;
    for (j = 0; j < 2; j++) {
        // first iteration just outline
        // second iteration pen

        if ((outline == 0) && (j == 0)) continue;
        if (j == 0) {
            painter->pen.style = PAINTER_PEN_STYLE_NONE;
        } else {
            painter->pen.style = penstyle;
            painter->pen.outline = 0;
        }
        int i;
        for (i = 0; i <  (count - 1); i++) {
            int line[] = {pts[i * 2], pts[i * 2 + 1], pts[i * 2 + 2], pts[i * 2 + 3]};
            osdPainterDrawLine(painter, line[0], line[1], line[2], line[3]);
        }
    }

    painter->pen.style = penstyle;
    painter->pen.outline = outline;
}

void osdPainterDrawCircle(OsdPainter *painter, int x0, int y0, int radius)
{
    if (!painter->ready) return;

    transformPoint(painter, &x0, &y0);

    int x = radius, y = 0;
    int radiusError = 1 - x;

    if (painter->pen.outline) {
        outlineCircle(painter, x0, y0, radius);
    }

    if (painter->brush.style) {
        fillCircle(painter, x0, y0, radius);
    }

    while(x >= y)
    {
        if (painter->pen.style) {
            drawPixel(painter, x + x0, y + y0);  // RDU
            drawPixel(painter, y + x0, x + y0); // RDD
            drawPixel(painter, -x + x0, y + y0); // LDU
            drawPixel(painter, -y + x0, x + y0); // LDD
            drawPixel(painter, -x + x0, -y + y0); // LUD
            drawPixel(painter, -y + x0, -x + y0); // LUU
            drawPixel(painter, x + x0, -y + y0); // RUD
            drawPixel(painter, y + x0, -x + y0); // RUU
        }

        y++;
        if (radiusError<0)
        {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError+= 2 * (y - x + 1);
        }
    }
}

void osdPainterDrawBitmap(OsdPainter *painter, int x, int y, const OsdBitmap *bitmap)
{
    if ((!painter->ready) || (painter->pen.style == PAINTER_PEN_STYLE_NONE) || (!bitmap)) return;
    transformPoint(painter, &x, &y);
    if ((painter->pen.outline) && (bitmap->outlineBuff)) {
        uint8_t penColor = painter->pen.color;
        painter->pen.color = getOutlineColor(painter);
        bitblit(painter, bitmap->outlineBuff, bitmap->width + 2, bitmap->height + 2, x - 1, y - 1);
        painter->pen.color = penColor;
    }
    bitblit(painter, bitmap->buff, bitmap->width, bitmap->height, x, y);
}

void osdPainterTextBounds(OsdPainter *painter, const char *text, int *width, int *height)
{
    if ((!painter->font) || (text == 0)) {
        *width = 0;
        *height = 0;
        return;
    }

    *height = painter->font->height;
    int lineCharsMax = 0;
    int lineChars = 0;
    while(*text != 0) {
        if (*text == '\n') {
            if (text[1] == 0) break;
            *height += painter->font->height + painter->font->lineSpace;
            lineChars = 0;
        } else {
            lineChars++;
            if (lineCharsMax < lineChars) lineCharsMax = lineChars;
        }
        text++;
    }

    *width = lineCharsMax * painter->font->width;
}

void osdPainterDrawText(OsdPainter *painter, int x, int y, const char *text)
{
    if ((!painter->ready) || (painter->font == 0) || (text == 0)) return;
    transformPoint(painter, &x, &y);

    while(text[0] != 0) {
        if (text[0] == '\n') {
            if (text[1] == 0) break;
            y += painter->font->height + painter->font->lineSpace;
            text++;
        }
        text = drawTextOneLine(painter, x, y, text);
    }
}

void osdPainterDrawTextAligned(OsdPainter *painter, int x, int y, int width, int height,
                               int alignment, const char *text)
{
    // TODO: fix horizontal alignment bug when text has multiple lines with different
    //       lengths

    if (!alignment) {
        osdPainterDrawText(painter, x, y, text);
        return;
    }

    if ((!painter->ready) || (painter->font == 0)) return;
    int realWidth = 0;
    int realHeight = 0;
    osdPainterTextBounds(painter, text, &realWidth, &realHeight);

    int y0 = y;
    int x0 = x;

    if (alignment & PAINTER_TEXT_ALIGN_CENTER) {
        x0 = (width - realWidth) / 2 + x;
    } else if (alignment & PAINTER_TEXT_ALIGN_RIGHT) {
        x0 = x + width - realWidth;
    }

    if (alignment & PAINTER_TEXT_VALIGN_CENTER) {
        y0 = (height - realHeight) / 2 + y;
    } else if (alignment & PAINTER_TEXT_VALIGN_DOWN) {
        y0 = y + height - realHeight;
    }

    osdPainterDrawText(painter, x0, y0, text);
}
