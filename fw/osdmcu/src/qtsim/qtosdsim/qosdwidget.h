#ifndef OSDWIDGET_H
#define OSDWIDGET_H

#include <QWidget>
#include <stdint.h>
#include <osdpainter.h>
class QOsdWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QOsdWidget(QWidget *parent = 0);
    OsdDevice *osdDevice();

protected:
    void paintEvent(QPaintEvent *e);    

signals:

public slots:

private:
    void drawBuff();
    static int deviceStart(void *priv);
    static void deviceStop(void *priv);
    static quint32 *deviceGetLevelBuffer(void *priv);
    static quint32 *deviceGetMaskBuffer(void *priv);
    static quint16 deviceGetHres(void *priv);
    static quint16 deviceGetVres(void *priv);
    static void deviceSwapBuffers(void *priv);
    static int deviceRegisterBufferSwappedCb(void *priv, BufferSwappedCb cb, void *ctx);

    QPixmap _background;
    QImage _whiteImg;
    QImage _blackImg;
    void *_devCtx;
    BufferSwappedCb _deviceSwappedCb;
    OsdDevice _osdDev;
    quint32 _level[(320 * 240) / 32];
    quint32 _mask[(320 * 240) / 32];
};

#endif // OSDWIDGET_H
