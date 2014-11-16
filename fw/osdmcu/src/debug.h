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
 * @file    debug.h
 * @brief   debugging routines
 *
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#define DEBUG_LISTING_ON

#include "delay.h"

#define DEBUG_STR_FORMAT    "[%03u.%03u] [%s:%s] "

void DebugSendString(const char *str);
int DebugPrintf(const char *fmt,...);
void _dhexprint(const char *file, const char *function, const uint8_t *dataPointer, uint16_t len);
void _dbarprint(const char *file, const char *function, uint32_t value, uint32_t rangeMin, uint32_t rangeMax, uint8_t size);

#ifdef DEBUG_LISTING_ON

#define dprint(format, ...) DebugPrintf(DEBUG_STR_FORMAT format "\r\n",  \
        getElapsedMs() / 1000, getElapsedMs() % 1000, __FILE__, __FUNCTION__, ##__VA_ARGS__)

#define dhexprint(data, len) _dhexprint(__FILE__, __FUNCTION__, (data), (len))

#else
#define dprint(format,...)
#define dhexprint(data,len)
#endif

void DebugInit(void);
int sprintf(char *out, const char *format, ...);

#endif /* DEBUG_H_ */
