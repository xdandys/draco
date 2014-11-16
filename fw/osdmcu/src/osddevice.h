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
 * @file    osddevice.h
 * @brief   OSD painting abstract device
 *
 */

#ifndef OSDDEVICE_H_
#define OSDDEVICE_H_
#include <stdint.h>

typedef void (*BufferSwappedCb)(void *ctx, uint32_t *levelBuffer, uint32_t *maskBuffer);

#define OSD_DEVICE_SUCCESS          0
#define OSD_DEVICE_FAILURE          -1

typedef struct {
    void *priv;
    struct {
        int (*start)(void *priv);
        void (*stop)(void *priv);
        uint32_t *(*getLevelBackBuffer)(void *priv);
        uint32_t *(*getMaskBackBuffer)(void *priv);
        uint16_t (*getHRes)(void *priv);
        uint16_t (*getVRes)(void *priv);
        void (*swapBuffers)(void *priv);
        int (*registerBufferSwappedCb)(void *priv, BufferSwappedCb cb, void *ctx);
    } ops;
}OsdDevice;

inline static int osdDeviceStart(OsdDevice *dev)
{
    if (dev->ops.start)
        return dev->ops.start(dev->priv);
    else
        return -1;
}

inline static void osdDeviceDeinit(OsdDevice *dev)
{
    if (dev->ops.stop)
        dev->ops.stop(dev->priv);
}

inline static uint32_t* osdDeviceGetLevelBackBuffer(OsdDevice *dev)
{
    if (dev->ops.getLevelBackBuffer)
        return dev->ops.getLevelBackBuffer(dev->priv);
    else
        return 0;
}

inline static uint32_t* osdDeviceGetMaskBackBuffer(OsdDevice *dev)
{
    if (dev->ops.getMaskBackBuffer)
        return dev->ops.getMaskBackBuffer(dev->priv);
    else
        return 0;
}

inline static uint16_t osdDeviceGetHRes(OsdDevice *dev)
{
    if (dev->ops.getHRes)
        return dev->ops.getHRes(dev->priv);
    else
        return 0;
}

inline static uint16_t osdDeviceGetVRes(OsdDevice *dev)
{
    if (dev->ops.getVRes)
        return dev->ops.getVRes(dev->priv);
    else
        return 0;
}

inline static void osdDeviceSwapBuffers(OsdDevice *dev)
{
    if (dev->ops.swapBuffers)
        dev->ops.swapBuffers(dev->priv);
}

inline static int osdDeviceRegisterBufferSwappedCb(OsdDevice *dev, BufferSwappedCb cb, void *ctx)
{
    if (dev->ops.registerBufferSwappedCb)
        return dev->ops.registerBufferSwappedCb(dev->priv, cb, ctx);
    else
        return -1;
}

#endif /* OSDDEVICE_H_ */

/** @} */
