#-------------------------------------------------
#
# Project created by QtCreator 2012-11-01T20:04:50
#
#-------------------------------------------------

QT       += core gui

TARGET = LicensePlateExtractor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    utils.cpp

HEADERS  += mainwindow.h \
    utils.h

FORMS    += mainwindow.ui

INCLUDEPATH += C:/opencv_build/install/include

LIBS += -L"C:/opencv_build/install/lib" \
        -lopencv_core231 \
        -lopencv_highgui231 \
