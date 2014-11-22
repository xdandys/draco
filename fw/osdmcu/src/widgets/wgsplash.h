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
 * @file    wgsplash.h
 * @brief   splash screen widget
 *
 */

#ifndef WGSPLASH_H_
#define WGSPLASH_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "osdwidget.h"
typedef struct {
    OsdWidget widget;
    const char *version;
} WgSplash;

void wgSplashInit(WgSplash *wg, const char *version);

#ifdef __cplusplus
}
#endif
#endif /* WGSPLASH_H_ */

/** @} */
