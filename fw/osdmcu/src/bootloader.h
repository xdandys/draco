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
 * @file    bootloader.h
 * @brief   bootloader
 *
 */


#ifndef SRC_BOOTLOADER_H_
#define SRC_BOOTLOADER_H_
extern unsigned long __bl_act;              // word in SRAM for communication with bootloader
/*
 * Communication with bootloader
 */
#define BL_ACT_DIRECTION_MASK           0xFFFF0000
#define BL_ACT_ACTION_MASK              0x0000FFFF
#define BL_ACT_APPTOBL                  0xAA550000
#define BL_ACT_BLTOAPP                  0x55AA0000
#define BL_ACT_APPREQ_STAY              0x00000001  /**< application request to stay in bootloader for unlimited time */
#define BL_ACT_BL_ACTIVITY              0x00000100  /**< bootloader was active */
#define BL_ACT_BL_FLASH                 0x00000200  /**< bootloader was active and flashing was performed */

#endif /* SRC_BOOTLOADER_H_ */
/** @} */
