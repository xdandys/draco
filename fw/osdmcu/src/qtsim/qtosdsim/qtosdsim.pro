#-------------------------------------------------
#
# Project created by QtCreator 2014-04-08T20:01:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtosdsim
TEMPLATE = app

DEFINES += "OSD_SIMULATOR"
INCLUDEPATH += ../../

SOURCES += main.cpp\
        mainwindow.cpp\
        ../../osdpainter.c\
        ../../osdscreen.c\
        ../../utils.c \
        ../../resources/*.c \
        ../../widgets/*.c \
        qosdwidget.cpp

HEADERS  += mainwindow.h \
    qosdwidget.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
