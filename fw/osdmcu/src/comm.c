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
#include <string.h>
#include <board.h>


SpiComm spiComm;

#define REQ_ID_VERSION              0
#define DATA_ID_LED_CONTROL         0



static void onRequestReceived(void *priv, uint8_t *data, uint8_t len, uint8_t *ansData, uint8_t *ansLen);
static void onDataReceived(void *priv, uint8_t *data, uint8_t len);

static const SpiCommConfig spiCommConfig = {
        .cbPrivData = 0,
        .io = &spiCommIo,
        .onDataReceived = onDataReceived,
        .onRequestReceived = onRequestReceived,
};


static void onRequestReceived(void *priv, uint8_t *data, uint8_t len, uint8_t *ansData, uint8_t *ansLen)
{
    switch (data[0]) {
    case REQ_ID_VERSION:
        ansData[0] = VMAJOR;
        ansData[1] = VMINOR;
        strcpy((char*)(&ansData[2]), DEVIDV);
        *ansLen = 2 + strlen(DEVIDV);
        break;

    }
}

static void onDataReceived(void *priv, uint8_t *data, uint8_t len)
{
    int i;
    switch(data[0]) {
    case DATA_ID_LED_CONTROL:
        for (i = 0; i < (len / 2); i++)
            ledControl(data[1 + i * 2], data[1 + i * 2 + 1]);
        break;

    default:
        break;
    }
}

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
