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
 * @file    spicomm.c
 * @brief   brief description here
 *
 */

#include "spicomm.h"
#include "board.h"
#include <string.h>

#define TX_FLAG_IN_PROGRESS                     0x01

#define FRAME_SOFD                              0x00

#define FRAME_TYPE_DATA                         0x08
#define FRAME_TYPE_REQUEST_0                    0x02
#define FRAME_TYPE_REQUEST_1                    0x03
#define FRAME_TYPE_ANSWER_0                     0x04
#define FRAME_TYPE_ANSWER_1                     0x05
#define FRAME_TYPE_SEQNO                        0x01
#define FRAME_TYPE_SYNC                         0xff

#define RX_STATE_WAIT_START                     0
#define RX_STATE_WAIT_FRAME_TYPE                1
#define RX_STATE_WAIT_LENGTH                    2
#define RX_STATE_WAIT_DATA                      3
#define RX_STATE_WAIT_CRC                       4

#define FLAGS_RX_REQUEST_PENDING                0x0002

// CRC lookup table
// compatible with TauLabs PiOS CRC
static const uint8_t crcLookup[256] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
};

static void setIrq(SpiComm *spiComm, uint8_t enable);

static uint8_t crcUpdate(uint8_t crc, uint8_t data)
{
    return crcLookup[crc ^ data];
}

static void rxFifoPutInterrupt(SpiComm *spiComm, uint8_t b)
{
    spiComm->rxFifoBuff[spiComm->rxFifoW] = b;
    if (spiComm->rxFifoW == (sizeof(spiComm->rxFifoBuff) - 1))
        spiComm->rxFifoW = 0;
    else
        spiComm->rxFifoW++;

}

static int rxFifoGet(SpiComm *spiComm, uint8_t *b)
{
    spiComm->cfg->io->blockCb();
    if (spiComm->rxFifoR != spiComm->rxFifoW) {
        *b = spiComm->rxFifoBuff[spiComm->rxFifoR];
        if (spiComm->rxFifoR == (sizeof(spiComm->rxFifoBuff) - 1))
            spiComm->rxFifoR = 0;
        else
            spiComm->rxFifoR++;
        spiComm->cfg->io->unblockCb();
        return 1;
    }
    spiComm->cfg->io->unblockCb();
    return 0;
}



static void onByteReceived(void *priv, uint8_t b)
{
    SpiComm *spiComm = (SpiComm*)priv;
    rxFifoPutInterrupt(spiComm, b);
}

static void onTxDone(void *priv)
{
    SpiComm *spiComm = (SpiComm*)priv;

    spiComm->txFlags &= ~TX_FLAG_IN_PROGRESS;
    setIrq(spiComm, 0);
}


static void rxByte(SpiComm *spiComm, uint8_t b)
{
    // on the fly COBS unstuffing
    if (b) {
        if (spiComm->rxCobsBlockIndex >= (spiComm->rxCobsBlockSize - 1)) {
            if (spiComm->rxCobsBlockSize < 0xff) {
                spiComm->rxCobsBlockSize = b;
                spiComm->rxCobsBlockIndex = 0;
                b = 0;
            } else {
                spiComm->rxCobsBlockSize = b;
                spiComm->rxCobsBlockIndex = 0;
                return;
            }
        } else {
            spiComm->rxCobsBlockIndex++;
        }
    } else {
        spiComm->rxState = RX_STATE_WAIT_START;
    }

    if (spiComm->rxState != RX_STATE_WAIT_CRC) spiComm->rxCrc = crcUpdate(spiComm->rxCrc, b);
    switch (spiComm->rxState) {
    case RX_STATE_WAIT_DATA:
        if (spiComm->rxBuff)
            spiComm->rxBuff[spiComm->rxBuffIndex] = b;

        spiComm->rxBuffIndex++;
        if (spiComm->rxBuffIndex >= spiComm->rxLength)
            spiComm->rxState = RX_STATE_WAIT_CRC;
        break;

    case RX_STATE_WAIT_START:
        if (b == FRAME_SOFD) {
            spiComm->rxState = RX_STATE_WAIT_FRAME_TYPE;
            spiComm->rxCrc = 0;
            spiComm->rxCobsBlockSize = 255;
            spiComm->rxCobsBlockIndex = 255;
        }
        break;

    case RX_STATE_WAIT_FRAME_TYPE:
        spiComm->rxBuff = 0;
        spiComm->rxBuffIndex = 0;
        spiComm->rxFrameType = b;
        if (b == FRAME_TYPE_DATA) {
            spiComm->rxBuff = spiComm->rxDataFrame;
        } else if ((b == FRAME_TYPE_REQUEST_0) || (b == FRAME_TYPE_REQUEST_1)) {
            if ((spiComm->flags & FLAGS_RX_REQUEST_PENDING) == 0) {
                spiComm->rxBuff = spiComm->rxRequestFrame;
                spiComm->rxRequestFrameNo = b & FRAME_TYPE_SEQNO;
            }
        } else if (b == FRAME_TYPE_SYNC) {
            spiComm->rxRequestLastNo = 255;
        }

        spiComm->rxState = RX_STATE_WAIT_LENGTH;
        break;

    case RX_STATE_WAIT_LENGTH:
        spiComm->rxLength = b;
        if (spiComm->rxLength != 0) {
            spiComm->rxState = RX_STATE_WAIT_DATA;
        } else {
            spiComm->rxState = RX_STATE_WAIT_CRC;
        }

        break;


    case RX_STATE_WAIT_CRC:
        if (b == spiComm->rxCrc) {
            if (spiComm->rxBuff == spiComm->rxRequestFrame) {
                spiComm->rxRequestFramLen = spiComm->rxLength;
                spiComm->flags |= FLAGS_RX_REQUEST_PENDING;
            } else if (spiComm->rxBuff == spiComm->rxDataFrame) {
                spiComm->rxDataFramLen = spiComm->rxLength;
                if (spiComm->cfg->onDataReceived) {
                    spiComm->cfg->onDataReceived(spiComm->cfg->cbPrivData, spiComm->rxDataFrame, spiComm->rxDataFramLen);
                }
            }
        }
        spiComm->rxState = RX_STATE_WAIT_START;
        break;

    default:
        spiComm->rxState = RX_STATE_WAIT_START;
        break;
    }
}

static void setIrq(SpiComm *spiComm, uint8_t enable)
{
    spiComm->cfg->io->setIrq(enable);
}

static void tx(SpiComm *spiComm, uint8_t *data, uint16_t len)
{
    if (spiComm->txFlags & TX_FLAG_IN_PROGRESS)
        return;

    spiComm->txFlags |= TX_FLAG_IN_PROGRESS;
    spiComm->cfg->io->tx(data, len);
    setIrq(spiComm, 1);
}


static void txGenericFrame(SpiComm *spiComm, const uint8_t *payload, uint8_t len, uint8_t frameType)
{
    // calculate CRC
    uint8_t crc = 0;
    crc = crcUpdate(crc, frameType);
    crc = crcUpdate(crc, len);
    int i;
    for (i = 0; i < len; i++) crc = crcUpdate(crc, payload[i]);

    spiComm->txBuffer[0] = 0;

    // COBS stuffing
    int cobsStartIdx = 0;
    int blockIdx = 1;
    for (i = 0; i < (len + 2 + 1); i++) {
        uint8_t b = (i == 0) ? frameType : (i == 1) ? len : (i == (len + 2)) ? crc : payload[i - 2];
        if (b == 0) {
            spiComm->txBuffer[cobsStartIdx + 1] = blockIdx;
            cobsStartIdx += blockIdx;
            blockIdx = 1;
        } else {
            spiComm->txBuffer[cobsStartIdx + blockIdx + 1] = b;
            blockIdx++;
        }

        if (blockIdx == 255) {
            spiComm->txBuffer[cobsStartIdx + 1] = blockIdx;
            cobsStartIdx += blockIdx;
            blockIdx = 1;
        }
    }
    spiComm->txBuffer[cobsStartIdx + 1] = blockIdx;

    // SPI transmitt
    tx(spiComm, spiComm->txBuffer, cobsStartIdx + blockIdx + 1);
}

static void txAnswer(SpiComm *spiComm, const uint8_t *data, uint8_t len, uint8_t no)
{
    uint8_t frameType = FRAME_TYPE_ANSWER_0;
    if (no)
        frameType |= FRAME_TYPE_SEQNO;

    txGenericFrame(spiComm, data, len, frameType);
}

static void txData(SpiComm *spiComm, const uint8_t *data, uint8_t len)
{
    uint8_t frameType = FRAME_TYPE_DATA;
    txGenericFrame(spiComm, data, len, frameType);
}


static void setDefaults(SpiComm *spiComm)
{
    spiComm->txFlags = 0;
    spiComm->rxState = 0;
    spiComm->rxCrc = 0;
    spiComm->rxLength = 0;
    spiComm->flags = 0;
    spiComm->rxBuff = 0;
    spiComm->rxRequestLastNo = 255;
    spiComm->rxFifoR = 0;
    spiComm->rxFifoW = 0;
}


void spiCommProcess(SpiComm *spiComm)
{

    uint8_t b = 0;
    int fifoState = 0;
    while ((fifoState = rxFifoGet(spiComm, &b)) != 0)
        rxByte(spiComm, b);

    if ((spiComm->txFlags & TX_FLAG_IN_PROGRESS) == 0) {
        if (spiComm->flags & FLAGS_RX_REQUEST_PENDING) {
            if ((spiComm->rxRequestFrameNo != spiComm->rxRequestLastNo) || (spiComm->rxRequestLastNo == 255)) {
                if (spiComm->cfg->onRequestReceived) {
                    uint8_t *ans = spiComm->txAnswerFrame;
                    uint8_t ansLen;
                    spiComm->cfg->onRequestReceived(spiComm->cfg->cbPrivData,
                            spiComm->rxRequestFrame,
                            spiComm->rxRequestFramLen,
                            ans,
                            &ansLen);
                    spiComm->txAnswerFramLastLen = ansLen;
                    txAnswer(spiComm, spiComm->txAnswerFrame, spiComm->txAnswerFramLastLen, spiComm->rxRequestFrameNo);
                    spiComm->rxRequestLastNo = spiComm->rxRequestFrameNo;
                }
            } else {
                // repeat last answer
                txAnswer(spiComm, spiComm->txAnswerFrame, spiComm->txAnswerFramLastLen, spiComm->rxRequestFrameNo);
            }
            spiComm->flags &= ~ FLAGS_RX_REQUEST_PENDING;
        }
    }
}

SpiCommResult spiCommSendRequest(SpiComm *spiComm, uint8_t *data, uint8_t dataLen)
{
    // TODO we don't need sending requests on OSD side now
    // can be implemented in the future when needed


    return SPICOMM_ERROR;
}

SpiCommResult spiCommSendData(SpiComm *spiComm, uint8_t *data, uint8_t dataLen)
{
    if (spiComm->txFlags & TX_FLAG_IN_PROGRESS)
        return SPICOMM_BUSY;

    txData(spiComm, data, dataLen);
    return SPICOMM_OK;
}

void spiCommStart(SpiComm *spiComm, const SpiCommConfig *config)
{
    spiComm->cfg = config;
    spiComm->ioConfig.onByteReceived = onByteReceived;
    spiComm->ioConfig.onTxDone = onTxDone;
    spiComm->ioConfig.priv = spiComm;

    setDefaults(spiComm);

    spiComm->cfg->io->start(&spiComm->ioConfig);
    setIrq(spiComm, 0);
}

void spiCommStop(SpiComm *spiComm)
{
    spiComm->cfg->io->stop();
}
