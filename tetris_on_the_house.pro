QT += qml quick widgets multimedia
!no_desktop: QT += widgets

CONFIG += c++11

TARGET = App
TEMPLATE = app

SOURCES += \
    main.cpp

RESOURCES += \
    qml.qrc
