#-------------------------------------------------
#
# Project created by QtCreator 2015-10-24T08:35:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TIATracker
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    pianokeyboard.cpp \
    tiasound/tiasound.cpp

HEADERS  += mainwindow.h \
    pianokeyboard.h \
    tiasound/tiasound.h

FORMS    += mainwindow.ui

CONFIG += c++11
