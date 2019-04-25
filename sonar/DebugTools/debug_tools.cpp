/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#include <sonar/DebugTools/debug_tools.h>

using namespace std;
using namespace sonar::image_utils;
using namespace Eigen;

namespace sonar {

namespace debug {

Image<Rgb_u> loadImage(const string & filename)
{
#if defined(DEBUG_TOOLS_ENABLED)
#if defined(OPENCV_LIB)
    cv::Mat mat = cv::imread(filename, cv::IMREAD_COLOR);
    if (mat.empty())
        return Image<Rgb_u>();
    assert(mat.type() == CV_8UC3);
    return convertCvMat_rgb_u(mat, true);
#else
    (void)filename;
    return Image<Rgb_u>();
#endif
#else
    (void)filename;
    assert(false);
    return Image<Rgb_u>();
#endif
}

void showImage(const string & name, const ImageRef<uchar> & image)
{
#if defined(DEBUG_TOOLS_ENABLED)
#if defined(OPENCV_LIB)
    cv::imshow(name, convertToCvMat(image).clone());
#else
    (void)name;
    (void)image;
    assert(false);
#endif
#else
    (void)name;
    (void)image;
#endif
}

void showImage(const string & name, const ImageRef<Rgb_u> & image)
{
#if defined(DEBUG_TOOLS_ENABLED)
#if defined(OPENCV_LIB)
    cv::imshow(name, convertToCvMat(image).clone());
#else
    (void)name;
    (void)image;
    assert(false);
#endif
#else
    (void)name;
    (void)image;
#endif
}

void showImage(const string & name, const ImageRef<Rgba_u> & image)
{
#if defined(DEBUG_TOOLS_ENABLED)
#if defined(OPENCV_LIB)
    cv::imshow(name, convertToCvMat(image).clone());
#else
    (void)name;
    (void)image;
    assert(false);
#endif
#else
    (void)name;
    (void)image;
#endif
}

void showImage(const string & name, const ImageRef<float> & map)
{
#if defined(DEBUG_TOOLS_ENABLED)
    Image<float> cMap = map.copy();
    normalize(cMap);
    Image<Rgb_u> rgb_map = cMap.convert<Rgb_u>([](const Point2i & p, const float & val) {
        (void)p;
        float l = val * 4.0f;
        if (l < 1.0f) {
            return cast<uchar>(Rgb_f(255.0f * l, 0.0f, 0.0f));
        } else if (l < 2.0f) {
            l -= 1.0f;
            return cast<uchar>(Rgb_f(255.0f * (1.0f - l), 255.0f * l, 0.0f));
        } else if (l < 3.0f) {
            l -= 2.0f;
            return cast<uchar>(Rgb_f(0.0f, 255.0f * (1.0f - l), 255.0f * l));
        }
        l -= 3.0f;
        return cast<uchar>(Rgb_f(255.0f * l, 255.0f * l, 255.0f));
    });
    showImage(name, rgb_map);
#else
    (void)name;
    (void)map;
#endif
}

void showImage(const string & name, const ImageRef<float> & map, float min, float max)
{
#if defined(DEBUG_TOOLS_ENABLED)
    Image<float> cMap = map.copy();
    normalize(cMap, min, max);
    Image<Rgb_u> rgb_map = cMap.convert<Rgb_u>([](const Point2i & p, const float & val) {
        (void)p;
        float l = val * 4.0f;
        if (l < 1.0f) {
            return cast<uchar>(Rgb_f(255.0f * l, 0.0f, 0.0f));
        } else if (l < 2.0f) {
            l -= 1.0f;
            return cast<uchar>(Rgb_f(255.0f * (1.0f - l), 255.0f * l, 0.0f));
        } else if (l < 3.0f) {
            l -= 2.0f;
            return cast<uchar>(Rgb_f(0.0f, 255.0f * (1.0f - l), 255.0f * l));
        }
        l -= 3.0f;
        return cast<uchar>(Rgb_f(255.0f * l, 255.0f * l, 255.0f));
    });
    showImage(name, rgb_map);
#else
    (void)name;
    (void)map;
    (void)min;
    (void)max;
#endif
}


int waitKey(int delay)
{
#if defined(DEBUG_TOOLS_ENABLED)
#if defined(OPENCV_LIB)
    return cv::waitKey(delay);
#else
    (void)delay;
    assert(false);
    return -1;
#endif
#else
    (void)(delay);
    return -1;
#endif
}

void closeImage(const string & name)
{
#if defined(DEBUG_TOOLS_ENABLED)
#if defined(OPENCV_LIB)
    cv::destroyWindow(name);
#else
    (void)name;
    assert(false);
#endif
#else
    (void)name;
#endif
}

bool writeToFile(const string & name, MatrixXd matrix)
{
#if defined(DEBUG_TOOLS_ENABLED)
    ofstream out;
    out.open(name, ios_base::out);
    if (!out.is_open())
        return false;
    out << matrix.cols() << "x" << matrix.rows() << endl;
    for (int i = 0; i < matrix.rows(); ++i) {
        for (int j = 0; j < matrix.cols(); ++j) {
            out << matrix(i, j) << " ";
        }
        out << endl;
    }
    out.close();
#else
    (void)name;
    (void)matrix;
#endif
    return true;
}


} // namespace debug

} // namespace sonar
