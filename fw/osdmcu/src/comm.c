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
 * @file    comm.c
 * @brief   brief description here
 *
 */

#include "comm.h"
#include "spicomm.h"
#include "spicommio_stm32f3.h"
#include "version.h"
#include "led.h"
#include "utils.h"
#include "hud.h"
#include "debug.h"
#include <string.h>
#include <board.h>


SpiComm spiComm;

#define REQ_ID_VERSION              0
#define REQ_ID_HUD_ENABLE           1
#define REQ_ID_POWER_LIMITS         10
#define REQ_ID_FLIGHT_MODE          11
#define REQ_ID_SET_UNITS            12

#define DATA_ID_LED_CONTROL         0
#define DATA_ID_PFD                 1
#define DATA_ID_WAYPOINT_HOME       2
#define DATA_ID_WAYPOINT_NAVI       3
#define DATA_ID_GNSS                4
#define DATA_ID_POWER               5
#define DATA_ID_STOPWATCH           6

struct DataPfd {
    int16_t speed;      // 10^-2 m/s
    int16_t vspeed;     // 10^-2 m/s
    int32_t altitude;   // 10^-2 m
    int16_t roll;       // 10^-1 degrees
    int16_t pitch;      // 10^-1 degrees
    int16_t heading;    // 10^-1 degrees
} __attribute__((packed));

struct DataWp {
    uint8_t show;
    uint32_t distance;      // 10^-2 m
    int16_t heading;        // 10^-1 degrees
} __attribute__((packed));

struct DataGnss {
    uint8_t fix;
    uint8_t satCount;
    uint16_t pdop;          // 10^-2
    int32_t lat;            // 10^-7 degrees
    int32_t lon;            // 10^-7 degrees
} __attribute__((packed));


struct DataPower {
    uint16_t voltage;       // 10^-2 V
    uint16_t current;       // 10^-2 A
    uint16_t mahs;          // 10^-3 A
} __attribute__ ((packed));

struct DataPowerLimits {
    uint16_t minVoltage;    // 10^-2 V
    uint16_t maxmahs;       // 10^-3 A
} __attribute__ ((packed));

struct DataStopwatch {
    uint8_t running;
    uint16_t maxSeconds;
} __attribute__((packed));

static void onRequestReceived(void *priv, uint8_t *data, uint8_t len, uint8_t *ansData, uint8_t *ansLen);
static void onDataReceived(void *priv, uint8_t *data, uint8_t len);

static const SpiCommConfig spiCommConfig = {
        .cbPrivData = 0,
        .io = &spiCommIo,
        .onDataReceived = onDataReceived,
        .onRequestReceived = onRequestReceived,
};


//------------------------------------------------------------------------
//
//         data frame processing
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------


static void processDataLed(const uint8_t *data, uint8_t len)
{
    int i;
    for (i = 0; i < (len / 2); i++)
        ledControl(data[i * 2], data[i * 2 + 1]);
}


static void processDataPfd(const uint8_t *data, uint8_t len)
{
    struct DataPfd dataPfd;
    if (len != sizeof(struct DataPfd))
        return;


    memcpy(&dataPfd, data, sizeof(struct DataPfd));
    hudSetPfdData(dataPfd.speed / 100.0f,
            dataPfd.vspeed / 100.0f,
            dataPfd.altitude / 100.0f,
            dataPfd.roll / 10.0f,
            dataPfd.pitch / 10.0f,
            dataPfd.heading / 10.0f);
}

static void processDataWpHome(const uint8_t *data, uint8_t len)
{
    struct DataWp dataWp;
    if (!len)
        return;

    memcpy(&dataWp, data, sizeof(struct DataWp));

    hudSetWaypoint(HUD_WAYPOINT_LOS, dataWp.show != 0, dataWp.distance / 100.0f, dataWp.heading / 10.0f);
}

static void processDataWpNavi(const uint8_t *data, uint8_t len)
{
    struct DataWp dataWp;
    if (!len)
        return;

    memcpy(&dataWp, data, sizeof(struct DataWp));

    hudSetWaypoint(HUD_WAYPOINT_NAV, dataWp.show != 0, dataWp.distance / 100.0f, dataWp.heading / 10.0f);
}


static void processDataGnss(const uint8_t *data, uint8_t len)
{
    struct DataGnss dataGnss;
    if (len != sizeof(struct DataGnss))
        return;

    memcpy(&dataGnss, data, sizeof(struct DataGnss));

    hudSetGnss(dataGnss.fix, dataGnss.lat, dataGnss.lon, dataGnss.pdop / 100.0f, dataGnss.satCount);
}

static void processDataPower(const uint8_t *data, uint8_t len)
{
    struct DataPower dataPower;
    if (len < 2)
        return;

    memcpy(&dataPower, data, sizeof(struct DataPower));
    hudSetBatteryVoltage(dataPower.voltage / 100.0f);
    if (len > 2)
        hudSetBatteryCurrent(dataPower.current / 100.0f, dataPower.mahs / 1000.0f);
}


static void processDataStopwatch(const uint8_t *data, uint8_t len)
{
    struct DataStopwatch stopwatch;

    if (len != sizeof(struct DataStopwatch))
        return;

    memcpy(&stopwatch, data, sizeof(struct DataStopwatch));

    hudControlStopwatch(stopwatch.running != 0, stopwatch.maxSeconds);
}


static void processRequestPowerLimits(const uint8_t *data, uint8_t len)
{
    struct DataPowerLimits limits;

    if (len != sizeof(struct DataPowerLimits))
        return;

    memcpy(&limits, data, sizeof(struct DataPowerLimits));

    hudSetBatteryLimits(limits.minVoltage / 100.0f, limits.maxmahs / 1000.0f);
}

static void processRequestFlightMode(const uint8_t *data, uint8_t len)
{
    if (!len)
        return;

    hudSetFlightMode((const char*)data);
}

static void processRequestSetUnits(const uint8_t *data, uint8_t len)
{
    if (!len)
        return;

    HudUnits units = (data[0] == 0) ? HUD_UNITS_METRIC : HUD_UNITS_IMPERIAL;
    hudSetUnits(units);
}

//------------------------------------------------------------------------
//
//         SPIComm bindings
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------


static void onRequestReceived(void *priv, uint8_t *data, uint8_t len, uint8_t *ansData, uint8_t *ansLen)
{
    if (!len) {
        *ansLen = 0;
        return;
    }

    switch (data[0]) {
    case REQ_ID_VERSION:
        ansData[0] = VMAJOR;
        ansData[1] = VMINOR;
        strcpy((char*)(&ansData[2]), DEVIDV);
        *ansLen = 2 + strlen(DEVIDV);
        break;

    case REQ_ID_HUD_ENABLE:
        if (data[1])
            hudStart();
        else
            hudStop();
        *ansLen = 0;
        break;

    case REQ_ID_POWER_LIMITS:
        processRequestPowerLimits(&data[1], len - 1);
        *ansLen = 0;
        break;

    case REQ_ID_FLIGHT_MODE:
        processRequestFlightMode(&data[1], len - 1);
        *ansLen = 0;
        break;

    case REQ_ID_SET_UNITS:
        processRequestSetUnits(&data[1], len - 1);
        *ansLen = 0;
        break;

    }
}

static void onDataReceived(void *priv, uint8_t *data, uint8_t len)
{
    if (!len) return;
    uint8_t cmd = data[0];
    len--;
    data++;
    switch(cmd) {
    case DATA_ID_LED_CONTROL:       processDataLed(data, len); break;
    case DATA_ID_PFD:               processDataPfd(data, len); break;
    case DATA_ID_WAYPOINT_HOME:     processDataWpHome(data, len); break;
    case DATA_ID_WAYPOINT_NAVI:     processDataWpNavi(data, len); break;
    case DATA_ID_GNSS:              processDataGnss(data, len); break;
    case DATA_ID_POWER:             processDataPower(data, len); break;
    case DATA_ID_STOPWATCH:         processDataStopwatch(data, len); break;
    default:
        break;
    }
}


//------------------------------------------------------------------------
//
//         public interface
//
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void commProcess(void)
{
    spiCommProcess(&spiComm);
}

void commStart(void)
{
    spiCommStart(&spiComm, &spiCommConfig);
}

void commStop(void)
{
    spiCommStop(&spiComm);
}
