DEFINES += OPENCV_LIB
DEFINES += DEBUG_TOOLS_ENABLED

windows {
    TEST_OPENCV_INCLUDE_PATH = $$(OPENCV_INCLUDE_PATH)
    isEmpty(TEST_OPENCV_INCLUDE_PATH) {
        error("Variable \"OPENCV_INCLUDE_PATH\" is not set")
    } else {
        TEST_OPENCV_LIB_PATH = $$(OPENCV_LIB_PATH)
        isEmpty(TEST_OPENCV_LIB_PATH) {
            error("Variable \"OPENCV_LIB_PATH\" is not set")
        } else {
            INCLUDEPATH += $$(OPENCV_INCLUDE_PATH)
            DEPENDPATH += $$(OPENCV_INCLUDE_PATH)

            CONFIG(debug, debug|release) {
                OPENCV_VERSION=341d
            } else {
                OPENCV_VERSION=341
            }
#            LIBS += -L$$(OPENCV_LIB_PATH) \
#                -lopencv_core$$OPENCV_VERSION \
#                -lopencv_imgproc$$OPENCV_VERSION \
#                -lopencv_video$$OPENCV_VERSION

#            contains(DEFINES, DEBUG_TOOLS_ENABLED) {
#                LIBS += -L$$(OPENCV_LIB_PATH) \
#                        -lopencv_highgui$$OPENCV_VERSION \
#                        -lopencv_imgcodecs$$OPENCV_VERSION
#            }
            LIBS += -L$$(OPENCV_LIB_PATH) -lopencv_world$$OPENCV_VERSION
        }
    }
} else {
    error("OpenCV not included")
}
