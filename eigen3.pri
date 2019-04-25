DEFINES += EIGEN3_LIB

linux {
    INCLUDEPATH += /usr/local/include/eigen3
    DEPENDPATH += /usr/local/include/eigen3
} else: windows {
    TEST_EIGEN_DIR = $$(EIGEN_DIR)
    isEmpty(TEST_EIGEN_DIR) {
        error("Variable \"EIGEN_DIR\" is not set")
    } else {
        INCLUDEPATH += $$(EIGEN_DIR)
        DEPENDPATH += $$(EIGEN_DIR)
    }
} else {
    error("Eigen not included")
}