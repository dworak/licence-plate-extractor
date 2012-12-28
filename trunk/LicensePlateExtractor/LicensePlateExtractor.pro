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
    utils.h \
    cvimagewidget.h

FORMS    += mainwindow.ui

INCLUDEPATH += $$(OPENCVPATH)/include

LIBS += -L$$(OPENCVPATH)/lib \
        -lopencv_core231 \
        -lopencv_highgui231 \
        -lopencv_imgproc231

RESOURCES += \
    resources.qrc
