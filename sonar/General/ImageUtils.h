/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_IMAGEUTILS_H
#define SONAR_IMAGEUTILS_H

#include <cmath>
#include <limits>
#include <climits>
#include <cassert>

#include <Eigen/Core>

#include <sonar/General/Image.h>

#include <sonar/General/cast.h>
#include <sonar/General/MathUtils.h>
#include <sonar/General/Paint.h>

#if QT_MULTIMEDIA_LIB
#include <QImage>
#endif

#if defined(OPENCV_LIB)
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#endif

///TODO moveCvMat and moveImage

namespace sonar {

namespace image_utils {

#if defined(QT_MULTIMEDIA_LIB)
static Image<Rgba_u> convertQImage(const QImage & image);
static QImage convertToQImage(const ImageRef<Rgba_u> & image);
static QImage convertToQImage(const ImageRef<Rgb_u> & image);
static QImage convertToQImage(const ImageRef<uchar> & image);
#endif

#if defined(OPENCV_LIB)
static Image<uchar> convertCvMat_u(cv::Mat image, bool copy = true);
static Image<short> convertCvMat_short(cv::Mat image, bool copy = true);
static Image<Rgb_u> convertCvMat_rgb_u(cv::Mat image, bool copy = true);
static Image<Rgba_u> convertCvMat_rgba_u(cv::Mat image, bool copy = true);
static Image<float> convertCvMat_f(cv::Mat image, bool copy = true);
static Image<int> convertCvMat_i(cv::Mat image, bool copy = true);
static Image<Rgb_f> convertCvMat_rgb_f(cv::Mat image, bool copy = true);
static Image<Rgba_f> convertCvMat_rgba_f(cv::Mat image, bool copy = true);

static cv::Mat convertToCvMat(const ImageRef<Rgba_u> & image);
static cv::Mat convertToCvMat(const ImageRef<Rgb_u> & image);
static cv::Mat convertToCvMat(const ImageRef<uchar> & image);
static cv::Mat convertToCvMat(const ImageRef<ushort> & image);
static cv::Mat convertToCvMat(const ImageRef<Rgba_f> & image);
static cv::Mat convertToCvMat(const ImageRef<Rgb_f> & image);
static cv::Mat convertToCvMat(const ImageRef<float> & image);
#endif

template <typename Type>
static Image<Type> resizeImage(const ImageRef<Type> & image, const Point2i & newSize);

// conversion rgb image to grayscale image
template <typename Type>
static Image<Type> convertToGrayscale(const ImageRef<Rgb<Type>> & image);

template <typename Type>
static void convertToGrayscale(Image<Type> & out, const ImageRef<Rgb<Type>> & in);

template <typename Type>
static Image<Type> convertToGrayscale(const ImageRef<Rgba<Type>> & image);

template <typename Type>
static void convertToGrayscale(Image<Type> & out, const ImageRef<Rgba<Type>> & in);

// computing integral image
template <typename SumType, typename Type>
static Image<typename Cast<Type, SumType>::Type> computeIntegralImage(const ImageRef<Type> & image);

template <typename SumType, typename Type>
static void computeIntegralImage(Image<typename Cast<Type, SumType>::Type> & outIntegral,
                                 const ImageRef<Type> & inImage);

template <typename SumType>
struct Sampler_avg
{
    template <typename Type>
    inline Type operator () (const Type & v1) const;

    template <typename Type>
    inline Type operator () (const Type & v1, const Type & v2) const;

    template <typename Type>
    inline Type operator () (const Type & v1_1, const Type & v1_2,
                             const Type & v2_1, const Type & v2_2) const;
};

template <typename _Ty = void>
struct less
{

    constexpr bool operator() (const _Ty& _Left, const _Ty& _Right) const
        {
            return (_Left < _Right);
        }
};

template <>
struct less<void>
{

    template<typename _Ty1, typename _Ty2>
        constexpr auto operator() (_Ty1&& _Left, _Ty2&& _Right) const ->
            decltype(static_cast<_Ty1&&>(_Left) < static_cast<_Ty2&&>(_Right))
        {	// transparently apply operator< to operands
            return (static_cast<_Ty1&&>(_Left) < static_cast<_Ty2&&>(_Right));
        }
};

template <class Compare = less<void>>
struct Sampler_min
{
    Compare cmp;

    template <typename Type>
    inline Type operator () (const Type & v1) const;

    template <typename Type>
    inline Type operator ()(const Type & v1, const Type & v2) const;

    template <typename Type>
    inline Type operator ()(const Type & v1_1, const Type & v1_2,
                            const Type & v2_1, const Type & v2_2) const;
};

template <class Compare = less<void>>
struct Sampler_max
{
    Compare cmp;

    template <typename Type>
    inline Type operator () (const Type & v1) const;

    template <typename Type>
    inline Type operator ()(const Type & v1, const Type & v2) const;

    template <typename Type>
    inline Type operator ()(const Type & v1_1, const Type & v1_2,
                            const Type & v2_1, const Type & v2_2) const;
};

// generating image with half size of source
static Image<uchar> halfSample(const ImageRef<uchar> & in);

static void halfSample(Image<uchar> & out, const ImageRef<uchar> & in);

template <typename SamplerType, typename Type>
static Image<Type> halfSample(const ImageRef<Type> & in);

template <typename SamplerType, typename Type>
static void halfSample(Image<Type> & out, const ImageRef<Type> & in);


// generating image with double size of source
static Image<uchar> doubleSample(const ImageRef<uchar> & in);

static void doubleSample(Image<uchar> & out, const ImageRef<uchar> & in);

template <typename SamplerType, typename Type>
static void doubleSample(Image<Type> & out, const ImageRef<Type> & in);

template <typename SamplerType, typename Type>
static Image<Type> doubleSample(const ImageRef<Type> & in);


// bicubic interpolation
template <typename OutType, typename Type, typename FType>
inline static OutType interpolate(const ImageRef<Type> & image, FType x, FType y);

template <typename OutType, typename Type, typename FType>
inline static OutType interpolate(const ImageRef<Type> & image, const Point2<FType> & point);


// operator sobel and
static void sobel(Image<int> & out, const ImageRef<uchar> & in);

static Image<int> sobel(const ImageRef<uchar> & in);

static void erode(Image<uchar> & out, const ImageRef<int> & integral, int size, float k);

static Image<uchar> erode(const ImageRef<int> & integral, int size, float k);

// fast gaussian blur
static void gaussianBlurX(Image<uchar> & out, const ImageRef<uchar> & in, int halfSizeBlur, float sigma);
static void gaussianBlurY(Image<uchar> & out, const ImageRef<uchar> & in, int halfSizeBlur, float sigma);
static void gaussianBlur(Image<uchar> & out, const ImageRef<uchar> & in, int halfSizeBlur, float sigma);

template <typename RealSumType, typename Type, typename FType>
static void gaussianBlurX(Image<Type> & out, const ImageRef<Type> & in, int halfSizeBlur, FType sigma);

template <typename RealSumType, typename Type, typename FType>
static void gaussianBlurY(Image<Type> & out, const ImageRef<Type> & in, int halfSizeBlur, FType sigma);

template <typename RealSumType, typename Type, typename FType>
static void gaussianBlur(Image<Type> & out, const ImageRef<Type> & in, int halfSizeBlur, FType sigma);

template <typename T, typename S, typename P>
static int transform2x2(Image<T> & out, const ImageRef<S> & in,
                        const Point2<P> & out_origin, const Point2<P> & in_origin,
                        const Eigen::Matrix<P, 2, 2> & transfromMatrix, const T defaultValue = T());

template <typename Type>
static void normalize(Image<Type> & inout);

template <typename Type>
static void normalize(Image<Type> & inout, Type min, Type max);

} // namespace image_utils

} // namespace sonar

#include "impl/ImageUtils_impl.hpp"
#endif // SONAR_IMAGEUTILS_H
