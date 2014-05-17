#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <osdscreen.h>
#include <widgets/wgverticalgauge.h>
#include <widgets/wgcompass.h>
#include <widgets/wgattitude.h>
#include <widgets/wgwaypoint.h>
#include <widgets/wgvario.h>
#include <widgets/wggps.h>
#include <widgets/wgpower.h>
#include <widgets/wgstopwatch.h>
#include <widgets/wgmode.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onFrameTime(void);

private:
    Ui::MainWindow *ui;    

    QTimer _frameTimer;


    OsdPainter painter;
    OsdScreen pfdScreen;


    // widgets
    WgVerticalGauge wgAltitude;
    WgVerticalGauge wgSpeed;
    WgCompass wgCompass;
    WgAttitude wgAttitude;
    WgWaypoint wgWaypoint;
    WgVario wgVario;
    WgGps wgGps;
    WgPower wgPower;
    WgStopwatch wgStopwatch;
    WgMode wgMode;
};

#endif // MAINWINDOW_H
