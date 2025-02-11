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
    utils.cpp \
    detectedframe.cpp \
    detectedplate.cpp \
    powiaty.cpp

HEADERS  += mainwindow.h \
    utils.h \
    cvimagewidget.h \
    detectedframe.h \
    detectedplate.h \
    powiaty.h

FORMS    += mainwindow.ui

INCLUDEPATH += $$(OPENCVPATH)/include

LIBS += -L$$(OPENCVPATH)/lib \
        -lopencv_core231 \
        -lopencv_highgui231 \
        -lopencv_imgproc231

RESOURCES += \
    resources.qrc
