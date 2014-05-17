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
 * @file    osd.h
 * @brief   brief description here
 *
 */


#ifndef OSD_H_
#define OSD_H_

#include "osddevice.h"


void osdMain(void);

#define VIDEO_BUFFER_MEM __attribute__ ((section(".videobuffer")))
#define DMA_MEM __attribute__ ((section(".dma")))

extern OsdDevice spiOsdDevice;



#endif /* OSD_H_ */

/** @} */
