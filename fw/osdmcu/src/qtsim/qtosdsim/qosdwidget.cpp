#include "qosdwidget.h"
#include <QPainter>
#include <QDebug>

QOsdWidget::QOsdWidget(QWidget *parent) :
    QWidget(parent),
    _whiteImg(QImage(320, 240, QImage::Format_MonoLSB)),
    _blackImg(QImage(320, 240, QImage::Format_MonoLSB)),
    _deviceSwappedCb(0)


{
    _background.load(":/images/osd_back2.jpg");
    _whiteImg.setColor(0, Qt::transparent);
    _whiteImg.setColor(1, 0xffffffff);
    _whiteImg.fill(0);
    _blackImg.setColor(0, Qt::transparent);
    _blackImg.setColor(1, 0xff000000);
    _blackImg.fill(0);

    _osdDev.priv = this;
    _osdDev.ops.getHRes = deviceGetHres;
    _osdDev.ops.getVRes = deviceGetVres;
    _osdDev.ops.getLevelBackBuffer = deviceGetLevelBuffer;
    _osdDev.ops.getMaskBackBuffer = deviceGetMaskBuffer;
    _osdDev.ops.registerBufferSwappedCb = deviceRegisterBufferSwappedCb;
    _osdDev.ops.swapBuffers = deviceSwapBuffers;
    _osdDev.ops.start = deviceStart;
    _osdDev.ops.stop = deviceStop;
}

OsdDevice *QOsdWidget::osdDevice()
{
    return &_osdDev;
}

void QOsdWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), _background);
    painter.drawImage(0, 0, _whiteImg.scaled(width(), height()));
    painter.drawImage(0, 0, _blackImg.scaled(width(), height()));
}


void QOsdWidget::drawBuff()
{
    if (_level == 0 || _mask == 0) return;
    quint8 *whitePtr = _whiteImg.bits();
    quint8 *blackPtr = _blackImg.bits();
    for(int i = 0; i < ((_whiteImg.width() * _whiteImg.height()) / 8); i++)
    {
        quint8 levelB = (_level[i / 4] >> ((i % 4) * 8)) & 0xFF;
        quint8 maskB = (_mask[i / 4] >> ((i % 4) * 8)) & 0xFF;
        quint8 wimgB = levelB & maskB;
        quint8 bimgB = (~levelB) & maskB;;
        whitePtr[i] = wimgB;
        blackPtr[i] = bimgB;
    }

    update();
}



int QOsdWidget::deviceStart(void *priv)
{
    Q_UNUSED(priv);
    return 0;
}

void QOsdWidget::deviceStop(void *priv)
{
    Q_UNUSED(priv);
}

quint16 QOsdWidget::deviceGetHres(void *priv)
{
    Q_UNUSED(priv);
    return 320;
}

quint16 QOsdWidget::deviceGetVres(void *priv)
{
    Q_UNUSED(priv);
    return 240;
}

quint32 *QOsdWidget::deviceGetLevelBuffer(void *priv)
{
    return ((QOsdWidget*)(priv))->_level;
}

quint32 *QOsdWidget::deviceGetMaskBuffer(void *priv)
{
    return ((QOsdWidget*)(priv))->_mask;
}


void QOsdWidget::deviceSwapBuffers(void *priv)
{
    QOsdWidget *owidget = (QOsdWidget*)priv;
    owidget->drawBuff();
    owidget->_deviceSwappedCb(owidget->_devCtx, owidget->_level, owidget->_mask);

}

int QOsdWidget::deviceRegisterBufferSwappedCb(void *priv, BufferSwappedCb cb, void *ctx)
{
    ((QOsdWidget*)(priv))->_deviceSwappedCb = cb;
    ((QOsdWidget*)(priv))->_devCtx = ctx;
    return 0;
}
