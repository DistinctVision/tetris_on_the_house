HEADERS += \
    $$PWD/Image.h \
    $$PWD/ImagePyramid.h \
    $$PWD/Point2.h \
    $$PWD/ImageUtils.h \
    $$PWD/WLS.h \
    $$PWD/cast.h \
    $$PWD/Paint.h \
    $$PWD/MathUtils.h

unix: {
QMAKE_CXXFLAGS = -Wno-unused-function
}

DEFINES += "EIGEN_MAX_CPP_VER=11"
DEFINES += _USE_MATH_DEFINES

DEFINES += MODULE_GENERAL
