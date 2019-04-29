QT += qml quick widgets multimedia
!no_desktop: QT += widgets

CONFIG += c++11

TARGET = App
TEMPLATE = app

include(eigen3.pri)
include(opencv.pri)

HEADERS += \
    framehandler.h \
    objectedgestracking.h \
    objectmodel.h

SOURCES += \
    main.cpp \
    framehandler.cpp \
    objectedgestracking.cpp \
    objectmodel.cpp

RESOURCES += \
    qml.qrc
