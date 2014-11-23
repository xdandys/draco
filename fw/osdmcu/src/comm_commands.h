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
 * @file    comm_commands.h
 * @brief   brief description here
 *
 */

#ifndef SRC_COMM_COMMANDS_H_
#define SRC_COMM_COMMANDS_H_

#define REQ_ID_VERSION              0
#define REQ_ID_HUD_ENABLE           1
#define REQ_ID_POWER_LIMITS         10
#define REQ_ID_FLIGHT_MODE          11
#define REQ_ID_SET_UNITS            12
#define REQ_ID_FORCE_TV_SYSTEM      20

#define REQ_ID_WRITE_BEGIN          128
#define REQ_ID_WRITE_CHUNK          129
#define REQ_ID_START_READ           130
#define REQ_ID_READ_CHUNK           131
#define REQ_ID_EXIT_BOOT            132
#define REQ_ID_ENTER_BOOT           133

#define DATA_ID_LED_CONTROL         0
#define DATA_ID_PFD                 1
#define DATA_ID_WAYPOINT_HOME       2
#define DATA_ID_WAYPOINT_NAVI       3
#define DATA_ID_GNSS                4
#define DATA_ID_POWER               5
#define DATA_ID_STOPWATCH           6

#define COMM_VERSION_MODE_FIRMWARE      1
#define COMM_VERSION_MODE_BOOTLOADER    0

#define COMM_RESULT_OK                  0
#define COMM_RESULT_ERROR               1
#endif /* SRC_COMM_COMMANDS_H_ */

/** @} */
