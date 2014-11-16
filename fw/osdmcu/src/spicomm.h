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
 * @file    spicomm.h
 * @brief   brief description here
 *
 */

#ifndef SPICOMM_H_
#define SPICOMM_H_

#include "stdint.h"

//-----------------------------------------------------------------------------
//
//          hardware interface
//
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

typedef void (*OnTxDoneCb)(void *priv);
typedef void (*OnByteReceivedCb)(void *priv, uint8_t b);
typedef void (*OnTransferEndCb)(void *priv);
typedef void (*OnIoErrorCb)(void *priv);

typedef struct {
    OnTxDoneCb onTxDone;
    OnByteReceivedCb onByteReceived;
    void *priv;
}SpiCommIoConfig;

typedef struct {
    void (*tx)(const uint8_t *data, uint16_t len);
    void (*blockCb)(void);
    void (*setIrq)(int en);
    void (*unblockCb)(void);
    void (*start)(const SpiCommIoConfig *config);
    void (*stop)(void);
    const SpiCommIoConfig *cfg;
}SpiCommIo;

//-----------------------------------------------------------------------------
//
//          logic interface
//
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

typedef void (*OnRequestReceivedCb)(void *priv, uint8_t *data, uint8_t len, uint8_t *ansData, uint8_t *ansLen);
typedef void (*OnDataReceivedCb)(void *priv, uint8_t *data, uint8_t len);
typedef void (*OnAnswerReceivedCb)(void *priv, uint8_t *reqData, uint8_t reqLen, uint8_t *data, uint8_t len);
typedef void (*OnRequestTimeoutCb)(void *priv, uint8_t *reqData, uint8_t reqLen);

typedef struct {
    void *cbPrivData;
    OnRequestReceivedCb onRequestReceived;
    OnDataReceivedCb onDataReceived;
    OnAnswerReceivedCb onAnswerReceived;   // <-- TODO this is currently never called
    OnRequestTimeoutCb onRequestTimeout;   // <-- TODO this is currently never called
    SpiCommIo *io;
} SpiCommConfig;

typedef struct {
    // SpiComm IO device bindings
    const SpiCommConfig *cfg;
    SpiCommIoConfig ioConfig;
    volatile uint32_t flags;
    volatile uint8_t txFlags;
    uint8_t rxState;
    uint8_t rxLength;
    uint8_t rxCobsBlockIndex;
    uint8_t rxCobsBlockSize;
    uint8_t rxCrc;
    uint8_t rxFrameType;
    uint8_t rxBuffIndex;
    uint8_t *rxBuff;
    uint8_t rxRequestLastNo;

    uint8_t txBuffer[264];
    uint8_t rxRequestFrame[256];
    uint8_t rxRequestFramLen;
    uint8_t rxRequestFrameNo;
    uint8_t rxDataFrame[256];
    uint8_t rxDataFramLen;
    uint8_t txAnswerFrame[256];
    uint8_t txAnswerFramLastLen;
    uint8_t rxFifoBuff[284];
    volatile uint16_t rxFifoW;
    volatile uint16_t rxFifoR;

} SpiComm;

typedef enum {
    SPICOMM_OK,
    SPICOMM_BUSY,
    SPICOMM_ERROR,
} SpiCommResult;

void spiCommStart(SpiComm *spiComm, const SpiCommConfig *config);
void spiCommStop(SpiComm *spiComm);
SpiCommResult spiCommSendRequest(SpiComm *spiComm, uint8_t *data, uint8_t dataLen);
SpiCommResult spiCommSendData(SpiComm *spiComm, uint8_t *data, uint8_t dataLen);

void spiCommProcess(SpiComm *spiComm);

void spiCommTest(void);

#endif /* SPICOMM_H_ */
