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
 * @file    version.h
 * @brief   firmware version
 *
 */

#ifndef VERSION_H_
#define VERSION_H_

#define DEVICE_DRACO_OSD

#if defined(DEVICE_DRACO_OSD)
    #define VARIANT "a"
    #define DEVIDV  "DRACO-OSD"
#endif

#define VMAJOR  0
#define VMINOR  01

#define V_STRINGIFY(str)   #str
#define STRINGIFY(s) V_STRINGIFY(s)

#define VERSIONSTRING   DEVIDV " " STRINGIFY(VMAJOR) "." STRINGIFY(VMINOR) VARIANT

extern const char __APP_VERSION[];

#endif /* VERSION_H_ */
