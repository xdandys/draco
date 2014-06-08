#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hud.h"
#include "swtimer.h"

#include <osdpainter.h>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    _frameTimer.start(40);
    connect(&_frameTimer, SIGNAL(timeout()), this, SLOT(onFrameTime()));

    OsdDevice *osdDevice = ui->osdWidget->osdDevice();
    hudInit(osdDevice);
    hudStart();
    hudSetUnits(HUD_UNITS_METRIC);
    hudSetPfdData(4, 2.5, 112, 19, 35, 163);
    hudSetWaypoint(HUD_WAYPOINT_LOS, true, 540, 100);
    hudSetGnss(true, 459526, 2445587, 1.9, 7);
    hudSetBatteryVoltage(11.61);
    hudSetBatteryCurrent(34.1, 2.1);
    hudControlStopwatch(true, 30);
    hudSetFlightMode("DISARMED");

//    osdPainterInit(&painter);
//    osdPainterSetDevice(&painter, osdDevice);

//    osdScreenInit(&pfdScreen, &painter);
//    wgVerticalGaugeInit(&wgAltitude, 5, false, false, "m");
//    wgVerticalGaugeInit(&wgSpeed, 5, true, true, "km/h");
//    wgCompassInit(&wgCompass);
//    wgAttitudeInit(&wgAttitude, 1.0);
//    wgWaypointInit(&wgWaypoint, "LOS", "m");
//    wgVarioInit(&wgVario, 8, "m/s");
//    wgGpsInit(&wgGps);
//    wgPowerInit(&wgPower);
//    wgStopwatchInit(&wgStopwatch);
//    wgModeInit(&wgMode);
//    wgSplashInit(&wgSplash, 1, 0);

//    osdScreenAddWidget(&pfdScreen, &wgSpeed.widget, 60, 120);
//    osdScreenAddWidget(&pfdScreen, &wgAltitude.widget, 260, 120);
//    osdScreenAddWidget(&pfdScreen, &wgCompass.widget, 160, 20);
//    osdScreenAddWidget(&pfdScreen, &wgAttitude.widget, 160, 120);
//    osdScreenAddWidget(&pfdScreen, &wgWaypoint.widget, 220, 5);
//    osdScreenAddWidget(&pfdScreen, &wgWaypoint.widget, 220, 30);
//    osdScreenAddWidget(&pfdScreen, &wgVario.widget, 240, 185);
//    osdScreenAddWidget(&pfdScreen, &wgGps.widget, 22, 5);
//    osdScreenAddWidget(&pfdScreen, &wgPower.widget, 10, 195);
//    osdScreenAddWidget(&pfdScreen, &wgStopwatch.widget, 142, 220);
//    osdScreenAddWidget(&pfdScreen, &wgMode.widget, 250, 220);
//    osdScreenAddWidget(&pfdScreen, &wgSplash.widget, 60, 80);

//    wgStopwatchStart(&wgStopwatch, true);
}

void MainWindow::onFrameTime()
{
//    static float value = 0;
//    static int heading = 0;
//    static float roll = 0;
//    static float pitch = 0;
//    static float vario = 0;
//    static bool varioDirUp = true;
//    value += 0.2;
//    roll += 1;
//    pitch += 1;
//    if (vario > 12.5)
//        varioDirUp = false;
//    if (vario < -12.5)
//        varioDirUp = true;
//    if (varioDirUp) vario += 0.3; else vario -= 0.3;


//    heading += 1;
//    wgVerticalGaugeSetValue(&wgSpeed, value);
//    wgVerticalGaugeSetValue(&wgAltitude, value);
//    wgCompassSetHeading(&wgCompass, heading);
//    wgAttitudeSetAngles(&wgAttitude, roll, pitch);
//    wgWaypointSet(&wgWaypoint, heading * 2, value);
//    wgVarioSetSpeed(&wgVario, vario);
//    wgGpsSet(&wgGps, true, 471238467, 128845627, 1.7, 7);
//    wgPowerSetVoltage(&wgPower, 11.4);
//    wgPowerSetCurrent(&wgPower, 85.5);
//    wgPowerSetMahs(&wgPower, 2471);
//    wgPowerSetLimits(&wgPower, 11.5, 5000);
//    wgModeSetMode(&wgMode, "ATTITUDE");


//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);
//    osdScreenProcess(&pfdScreen, false);

    int i;
    for (i = 0; i < 20; i++) {
        hudProcess();
        swTimerProcess();
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}
