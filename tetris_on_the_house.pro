QT += qml quick widgets multimedia
!no_desktop: QT += widgets

CONFIG += c++11

TARGET = App
TEMPLATE = app

include(eigen3.pri)
include(opencv.pri)
include(gl/gl.pri)
include(game/game.pri)

HEADERS += \
    debugimageobject.h \
    framehandler.h \
    objectedgestracker.h \
    objectmodel.h \
    pinholecamera.h \
    performancemonitor.h \
    poseoptimizer.h \
    texture2grayimageconvertor.h

SOURCES += \
    debugimageobject.cpp \
    main.cpp \
    framehandler.cpp \
    objectedgestracker.cpp \
    objectmodel.cpp \
    pinholecamera.cpp \
    performancemonitor.cpp \
    poseoptimizer.cpp \
    texture2grayimageconvertor.cpp

RESOURCES += \
    qml.qrc \
    shaders.qrc

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/../../opencv/android/libs/armeabi-v7a/libopencv_java4.so
}
