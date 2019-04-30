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
    objectmodel.h \
    pinholecamera.h \
    performancemonitor.h \
    poseoptimizer.h

SOURCES += \
    main.cpp \
    framehandler.cpp \
    objectedgestracking.cpp \
    objectmodel.cpp \
    pinholecamera.cpp \
    performancemonitor.cpp \
    poseoptimizer.cpp

RESOURCES += \
    qml.qrc
