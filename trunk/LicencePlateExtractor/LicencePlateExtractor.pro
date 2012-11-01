#-------------------------------------------------
#
# Project created by QtCreator 2012-10-31T04:32:08
#
#-------------------------------------------------

TARGET = LicencePlateExtractor
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

INCLUDEPATH += D:/Programing/OpenCV/Build/install/include

LIBS += -L"D:/Programing/OpenCV/Build/install/lib" \
        -lopencv_core231 \
        -lopencv_highgui231 \
