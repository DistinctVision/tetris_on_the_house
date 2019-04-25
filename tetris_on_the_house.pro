QT += qml quick widgets multimedia
!no_desktop: QT += widgets

CONFIG += c++11

TARGET = App
TEMPLATE = app

include(eigen3.pri)

TEST_DEBUG_TOOLS_ENABLED = $$(DEBUG_TOOLS_ENABLED)
!isEmpty(TEST_DEBUG_TOOLS_ENABLED) {
    DEFINES += DEBUG_TOOLS_ENABLED
    include(opencv.pri)
    include(sonar/DebugTools/DebugTools.pri)
}

include(sonar/General/General.pri)

HEADERS += \
    framehandler.h \
    linesdetector.h

SOURCES += \
    main.cpp \
    framehandler.cpp \
    linesdetector.cpp

RESOURCES += \
    qml.qrc
