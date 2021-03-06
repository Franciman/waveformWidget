#-------------------------------------------------
#
# Project created by QtCreator 2016-03-29T22:55:35
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT_CONFIG -= no-pkg-config

include(srtParser/srtParser.pri)
include(mediaProcessor/mediaProcessor.pri)

TARGET = waveformWidget
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    waveformview.cpp \
    waveformcontroller.cpp \
    waveformutils.cpp \
    renderer.cpp \
    rangelist.cpp \
    minblank.cpp

HEADERS  += mainwindow.h \
    waveformview.h \
    timemstoshortstring.h \
    model.h \
    constrain.h \
    renderer.h \
    rangelist.h

FORMS    += mainwindow.ui

CONFIG += c++11 link_pkgconfig

PKGCONFIG += mpv
