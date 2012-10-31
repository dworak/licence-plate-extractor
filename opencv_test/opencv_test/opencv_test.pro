#-------------------------------------------------
#
# Project created by QtCreator 2012-10-31T17:58:47
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = opencv_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

INCLUDEPATH += C:/opencv_build/install/include

LIBS += -L"C:/opencv_build/install/lib" \
        -lopencv_core231 \
        -lopencv_highgui231 \
