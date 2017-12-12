#-------------------------------------------------
#
# Project created by QtCreator 2017-12-05T20:59:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyServer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

include("http/http.pri")
#include("tcp/tcp.pri")


SOURCES += \
        main.cpp \
        dialog.cpp \
    test.cpp

HEADERS += \
        dialog.h \
    test.h

FORMS += \
        dialog.ui
