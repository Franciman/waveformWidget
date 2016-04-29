#-------------------------------------------------
#
# Project created by QtCreator 2016-03-29T22:55:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = waveformWidget
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    waveformview.cpp

HEADERS  += mainwindow.h \
    waveformview.h \
    timemstoshortstring.h

FORMS    += mainwindow.ui

CONFIG += c++11
