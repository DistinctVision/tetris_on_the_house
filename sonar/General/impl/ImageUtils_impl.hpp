/**
* This file is part of sonar library
* Copyright (C) 2018 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_IMAGEUTILS_IMPL_HPP
#define SONAR_IMAGEUTILS_IMPL_HPP

namespace sonar {

namespace image_utils {

#if defined(QT_MULTIMEDIA_LIB)
Image<Rgba_u> convertQImage(const QImage & image)
{
    Image<Rgba_u> result(Point2i(image.width(), image.height()));
    Point2i p;
    if (image.allGray()) {
        for (p.y = 0; p.y < result.height(); ++p.y) {
            Rgba_u * resultStr = &result.data()[result.widthStep() * p.y];
            for (p.x = 0; p.x < result.width(); ++p.x) {
                int gray = qGray(image.pixel(p.x, p.y));
                resultStr[p.x].set(gray, gray, gray, 255);
            }
        }
    } else {
        for (p.y = 0; p.y < result.height(); ++p.y) {
            Rgba_u * resultStr = &result.data()[result.widthStep() * p.y];
            for (p.x = 0; p.x < result.width(); ++p.x) {
                QRgb rgba = image.pixel(p.x, p.y);
                resultStr[p.x].set(cast<uchar>(qRed(rgba)), cast<uchar>(qGreen(rgba)),
                                   cast<uchar>(qBlue(rgba)), cast<uchar>(qAlpha(rgba)));
            }
        }
    }
    return result;
}

QImage convertToQImage(const ImageRef<Rgba_u> & image)
{
    QImage result(image.width(), image.height(), QImage::Format_RGB32);
    Point2i p;
    for (p.y = 0; p.y < image.height(); ++p.y) {
        const Rgba_u * imageStr = &image.data()[image.widthStep() * p.y];
        for (p.x = 0; p.x < image.width(); ++p.x) {
            result.setPixel(p.x, p.y, qRgba(imageStr[p.x].red,
                                            imageStr[p.x].green,
                                            imageStr[p.x].blue,
                                            imageStr[p.x].alpha));
        }
    }
    return result;
}

QImage convertToQImage(const ImageRef<Rgb_u> & image)
{
    QImage result(image.width(), image.height(), QImage::Format_RGB32);
    Point2i p;
    for (p.y = 0; p.y < image.height(); ++p.y) {
        const Rgb_u * imageStr = &image.data()[image.widthStep() * p.y];
        for (p.x = 0; p.x < image.width(); ++p.x) {
            result.setPixel(p.x, p.y, qRgb(imageStr[p.x].red,
                                           imageStr[p.x].green,
                                           imageStr[p.x].blue));
        }
    }
    return result;
}

QImage convertToQImage(const ImageRef<uchar> & image)
{
    QImage result(image.width(), image.height(), QImage::Format_RGB32);
    Point2i p;
    for (p.y = 0; p.y < image.height(); ++p.y) {
        const uchar * imageStr = &image.data()[image.widthStep() * p.y];
        for (p.x = 0; p.x < image.width(); ++p.x) {
            result.setPixel(p.x, p.y, qRgb(imageStr[p.x],
                                           imageStr[p.x],
                                           imageStr[p.x]));
        }
    }
    return result;
}
#endif

#if defined(OPENCV_LIB)
Image<uchar> convertCvMat_u(cv::Mat image, bool copy)
{
    assert(image.type() == CV_8UC1);
    Image<uchar> im(Point2i(image.cols, image.rows), reinterpret_cast<uchar*>(image.data),
                    cast<int>(image.step1()), false);
    return copy ? im.copy() : im;
}

Image<short> convertCvMat_short(cv::Mat image, bool copy)
{
    assert(image.type() == CV_16SC1);
    Image<short> im(Point2i(image.cols, image.rows), reinterpret_cast<short*>(image.data),
                    cast<int>(image.step1()), false);
    return copy ? im.copy() : im;
}

Image<Rgb_u> convertCvMat_rgb_u(cv::Mat image, bool copy)
{
    assert(image.type() == CV_8UC3);
    Image<Rgb_u> im(Point2i(image.cols, image.rows), reinterpret_cast<Rgb_u*>(image.data),
                    cast<int>(image.step1()) / 3, false);
    return copy ? im.copy() : im;
}

Image<Rgba_u> convertCvMat_rgba_u(cv::Mat image, bool copy)
{
    assert(image.type() == CV_8UC4);
    Image<Rgba_u> im(Point2i(image.cols, image.rows), reinterpret_cast<Rgba_u*>(image.data),
                     cast<int>(image.step1()) / 4, false);
    return copy ? im.copy() : im;
}

Image<float> convertCvMat_f(cv::Mat image, bool copy)
{
    assert(image.type() == CV_32FC1);
    Image<float> im(Point2i(image.cols, image.rows), reinterpret_cast<float*>(image.data),
                    cast<int>(image.step1()), false);
    return copy ? im.copy() : im;
}

Image<Rgb_f> convertCvMat_rgb_f(cv::Mat image, bool copy)
{
    assert(image.type() == CV_32FC3);
    Image<Rgb_f> im(Point2i(image.cols, image.rows), reinterpret_cast<Rgb_f*>(image.data),
                    cast<int>(image.step1()) / 3, false);
    return copy ? im.copy() : im;
}

Image<Rgba_f> convertCvMat_rgba_f(cv::Mat image, bool copy)
{
    assert(image.type() == CV_32FC4);
    Image<Rgba_f> im(Point2i(image.cols, image.rows), reinterpret_cast<Rgba_f*>(image.data),
                     cast<int>(image.step1()) / 4, false);
    return copy ? im.copy() : im;
}

Image<int> convertCvMat_i(cv::Mat image, bool copy)
{
    assert(image.type() == CV_32S);
    Image<int> im(Point2i(image.cols, image.rows), reinterpret_cast<int*>(image.data),
                  cast<int>(image.step1()), false);
    return copy ? im.copy() : im;
}

cv::Mat convertToCvMat(const ImageRef<Rgba_u> & image)
{
    cv::Mat result(image.height(), image.width(), CV_8UC4,
                   const_cast<uchar*>(reinterpret_cast<const uchar*>(image.data())),
                   image.widthStep() * sizeof(Rgba_u));
    return result;
}

cv::Mat convertToCvMat(const ImageRef<Rgb_u> & image)
{
    cv::Mat result(image.height(), image.width(), CV_8UC3,
                   const_cast<uchar*>(reinterpret_cast<const uchar*>(image.data())),
                   image.widthStep() * sizeof(Rgb_u));
    return result;
}

cv::Mat convertToCvMat(const ImageRef<uchar> & image)
{
    cv::Mat result(image.height(), image.width(), CV_8UC1,
                   const_cast<uchar*>(reinterpret_cast<const uchar*>(image.data())),
                   image.widthStep() * sizeof(uchar));
    return result;
}

cv::Mat convertToCvMat(const ImageRef<ushort> & image)
{
    cv::Mat result(image.height(), image.width(), CV_16UC1,
                   const_cast<ushort*>(reinterpret_cast<const ushort*>(image.data())),
                   image.widthStep() * sizeof(ushort));
    return result;
}

cv::Mat convertToCvMat(const ImageRef<float> & image)
{
    cv::Mat result(image.height(), image.width(), CV_32FC1,
                   const_cast<uchar*>(reinterpret_cast<const uchar*>(image.data())),
                   image.widthStep() * sizeof(float));
    return result;
}

cv::Mat convertToCvMat(const ImageRef<Rgba_f> & image)
{
    cv::Mat result(image.height(), image.width(), CV_32FC4,
                   const_cast<uchar*>(reinterpret_cast<const uchar*>(image.data())),
                   image.widthStep() * sizeof(Rgba_f));
    return result;
}

cv::Mat convertToCvMat(const ImageRef<Rgb_f> & image)
{
    cv::Mat result(image.height(), image.width(), CV_32FC3,
                   const_cast<uchar*>(reinterpret_cast<const uchar*>(image.data())),
                   image.widthStep() * sizeof(Rgb_f));
    return result;
}
#endif

template <typename Type>
Image<Type> resizeImage(const ImageRef<Type> & image, const Point2i & newSize)
{
    Image<Type> newImage(newSize);
    paint::drawImage(newImage, image, image.size());
    return newImage;
}

template <typename Type>
void convertToGrayscale(Image<Type> & out, const ImageRef<Rgb<Type>> & image)
{
    assert(out.size() == image.size());
    Point2i p;
    for (p.y = 0; p.y < out.height(); ++p.y) {
        Type * resultStr = &out.data()[out.widthStep() * p.y];
        const Rgb<Type> * imageStr = &image.data()[image.widthStep() * p.y];
        for (p.x = 0; p.x < out.width(); ++p.x) {
            const Rgb<Type> & rgb = imageStr[p.x];
            resultStr[p.x] = cast<Type>((rgb.red + rgb.green + rgb.blue) / 3);
        }
    }
}

template < typename Type >
Image<Type> convertToGrayscale(const ImageRef<Rgb<Type>> & image)
{
    Image<Type> r(image.size());
    convertToGrayscale<Type>(r, image);
    return r;
}

template <typename Type>
void convertToGrayscale(Image<Type> & out, const ImageRef<Rgba<Type>> & image)
{
    assert(out.size() == image.size());
    Point2i p;
    for (p.y = 0; p.y < out.height(); ++p.y) {
        Type * resultStr = &out.data()[out.widthStep() * p.y];
        const Rgba<Type> * imageStr = &image.data()[image.widthStep() * p.y];
        for (p.x = 0; p.x < out.width(); ++p.x) {
            const Rgba<Type> & rgb = imageStr[p.x];
            resultStr[p.x] = cast<Type>((rgb.red + rgb.green + rgb.blue) / 3);
        }
    }
}

template <typename Type>
Image<Type> convertToGrayscale(const ImageRef<Rgba<Type>> & image)
{
    Image<Type> r(image.size());
    convertToGrayscale<Type>(r, image);
    return r;
}

template <typename SumType, typename Type>
void computeIntegralImage(Image<typename Cast<Type, SumType>::Type> & outIntegral, const ImageRef<Type> & inImage)
{
    assert(outIntegral.size() == inImage.size());

    const Type * strCur = inImage.data();
    typename Cast<Type, SumType>::Type * strOutIntegral_prev;
    typename Cast<Type, SumType>::Type * strOutIntegral = outIntegral.data();
    int w = inImage.width();
    int h = inImage.height();
    typename Cast<Type, SumType>::Type rs;
    Point2i p;
    strOutIntegral[0] = strCur[0];
    for (p.x = 1; p.x < w; ++p.x)
        strOutIntegral[p.x] = strCur[p.x] + strOutIntegral[p.x - 1];
    for (p.y = 1; p.y < h; ++p.y) {
        strCur = &strCur[inImage.widthStep()];
        strOutIntegral_prev = strOutIntegral;
        strOutIntegral = &strOutIntegral[outIntegral.widthStep()];

        rs = strCur[0];
        strOutIntegral[0] = strOutIntegral_prev[0] + rs;
        for (p.x = 1; p.x < w; ++p.x) {
            rs += strCur[p.x];
            strOutIntegral[p.x] = rs + strOutIntegral_prev[p.x];
        }
    }
}

template <typename SumType>
template <typename Type>
Type Sampler_avg<SumType>::operator () (const Type & v1) const
{
    return v1;
}

template <typename SumType>
template <typename Type>
Type Sampler_avg<SumType>::operator () (const Type & v1, const Type & v2) const
{
    return cast<typename BaseElement<Type>::Type>((cast<SumType>(v1) +
                                                   cast<SumType>(v2)) * 0.5f);
}

template <typename SumType>
template <typename Type>
Type Sampler_avg<SumType>::operator () (const Type & v1_1, const Type & v1_2,
                                        const Type & v2_1, const Type & v2_2) const
{
    return cast<typename BaseElement<Type>::Type>(
                       (cast<SumType>(v1_1) + cast<SumType>(v1_2) +
                        cast<SumType>(v2_1) + cast<SumType>(v2_2)) * 0.25f);
}

template <typename SumType>
template <typename Type>
Type Sampler_min<SumType>::operator () (const Type & v1) const
{
    return v1;
}

template <class Compare>
template <typename Type>
Type Sampler_min<Compare>::operator () (const Type & v1, const Type & v2) const
{
    return std::min(v1, v2, cmp);
}

template <class Compare>
template <typename Type>
Type Sampler_min<Compare>::operator () (const Type & v1_1, const Type & v1_2,
                                        const Type & v2_1, const Type & v2_2) const
{
    return std::min(std::min(v1_1, v1_2, cmp), std::min(v2_1, v2_2, cmp), cmp);
}

template <typename SumType>
template <typename Type>
Type Sampler_max<SumType>::operator () (const Type & v1) const
{
    return v1;
}

template <class Compare>
template <typename Type>
Type Sampler_max<Compare>::operator () (const Type & v1, const Type & v2) const
{
    return std::max(v1, v2, cmp);
}

template <class Compare>
template <typename Type>
Type Sampler_max<Compare>::operator () (const Type & v1_1, const Type & v1_2,
                               const Type & v2_1, const Type & v2_2) const
{
    return std::max(std::max(v1_1, v1_2, cmp), std::max(v2_1, v2_2, cmp), cmp);
}

template <typename SamplerType, typename Type>
void halfSample(Image<Type> & out, const ImageRef<Type> & in)
{
    assert((in.size() / 2) == out.size());
    SamplerType sampler;
    Point2i outP;
    int inPx = 0;
    Type * outStr = out.data();
    const Type * inStrA = in.data();
    const Type * inStrB = &inStrA[in.widthStep()];
    for (outP.y = 0; outP.y < out.height(); ++outP.y) {
        for (inPx = outP.x = 0; outP.x < out.width(); ++outP.x) {
            outStr[outP.x] = sampler(inStrA[inPx], inStrA[inPx + 1],
                                     inStrB[inPx], inStrB[inPx + 1]);
            inPx += 2;
        }
        outStr = &outStr[out.widthStep()];
        inStrA = &inStrB[in.widthStep()];
        inStrB = &inStrA[in.widthStep()];
    }
}

template <typename SamplerType, typename Type>
Image<Type> halfSample(const ImageRef<Type> & in)
{
    Image<Type> out(in.size() / 2);
    halfSample<SamplerType, Type>(out, in);
    return out;
}

void halfSample(Image<uchar> & out, const ImageRef<uchar> & in)
{
    halfSample<Sampler_avg<int>, uchar>(out, in);
}

Image<uchar> halfSample(const ImageRef<uchar> & in)
{
    Image<uchar> out(in.size() / 2);
    halfSample(out, in);
    return out;
}

template <typename SamplerType, typename Type>
void doubleSample(Image<Type> & out, const ImageRef<Type> & in)
{
    assert(in.size() == (out.size() / 2));
    SamplerType sampler;
    int width = in.width() * 2;
    int height = in.height() * 2;
    Point2i p;
    Type * outStrA = out.data();
    Type * outStrB = &outStrA[out.widthStep()];
    const Type * inStrA = in.data();
    const Type * inStrB = &inStrA[in.widthStep()];
    for (p.y = 0; p.y < in.height() - 1; ++p.y) {
        for (p.x = 0; p.x < in.width() - 1; ++p.x) {
            outStrA[p.x * 2] = sampler(inStrA[p.x]);
            outStrA[p.x * 2 + 1] = sampler(inStrA[p.x], inStrA[p.x + 1]);
            outStrB[p.x * 2] = sampler(inStrA[p.x], inStrB[p.x]);
            outStrB[p.x * 2 + 1] = sampler(inStrA[p.x], inStrA[p.x + 1],
                                           inStrB[p.x], inStrB[p.x + 1]);
        }
        outStrA[width - 2] = sampler(inStrA[p.x]);
        outStrB[width - 2] = sampler(inStrA[p.x], inStrB[p.x]);
        outStrA[width - 1] = outStrA[width - 2];
        outStrB[width - 1] = outStrB[width - 2];
        if (width < out.width()) {
            outStrA[width] = outStrA[width - 1];
            outStrB[width] = outStrB[width - 1];
        }
        outStrA = &outStrB[out.widthStep()];
        outStrB = &outStrA[out.widthStep()];
        inStrA = inStrB;
        inStrB = &inStrA[in.widthStep()];
    }
    {
        for (p.x = 0; p.x < in.width() - 1; ++p.x) {
            outStrB[p.x * 2 + 1] = outStrB[p.x * 2] = outStrA[p.x * 2] = sampler(inStrA[p.x]);
            outStrA[p.x * 2 + 1] = sampler(inStrA[p.x], inStrA[p.x + 1]);
        }
        outStrA[width - 2] = outStrB[width - 2] = sampler(inStrA[p.x]);
        outStrA[width - 1] = outStrA[width - 2];
        outStrB[width - 1] = outStrB[width - 2];
        if (width < out.width()) {
            outStrA[width] = outStrA[width - 1];
            outStrB[width] = outStrB[width - 1];
        }
    }
    if (height < out.height()) {
        outStrA = &outStrB[out.widthStep()];
        for (p.x = 0; p.x < width; ++p.x) {
            outStrA[p.x] = outStrB[p.x];
        }
        if (width < out.width()) {
            outStrA[width] = sampler(outStrA[width - 1], outStrB[width]);
        }
    }
}


template <typename SamplerType, typename Type>
Image<Type> doubleSample(const ImageRef<Type> & in)
{
    Image<Type> out(in.size() * 2);
    doubleSample<SamplerType, Type>(out, in);
    return out;
}

void doubleSample(Image<uchar> & out, const ImageRef<uchar> & in)
{
    doubleSample<Sampler_avg<int>, uchar>(out, in);
}

Image<uchar> doubleSample(const ImageRef<uchar> & in)
{
    Image<uchar> out(in.size() * 2);
    doubleSample<Sampler_avg<int>, uchar>(out, in);
    return out;
}

template <typename SumType, typename Type>
Image<typename Cast<Type, SumType>::Type> computeIntegralImage(const ImageRef<Type> & image)
{
    Image<typename Cast<Type, SumType>::Type> integral(image.size());
    computeIntegralImage(integral, image);
    return integral;
}

template <typename OutType, typename Type, typename FType>
OutType interpolate(const ImageRef<Type> & image, FType x, FType y)
{
    int ix = cast<int>(std::floor(x)), iy = cast<int>(std::floor(y));
    FType dx = x - ix, dy = y - iy;
    FType idx = cast<FType>(1) - dx, idy = cast<FType>(1) - dy;
    const Type * strA = image.pointer(ix, iy);
    const Type * strB = &strA[image.widthStep()];
    return cast<BaseElement<OutType>::Type>(strA[0]) * (idx * idy) +
           cast<BaseElement<OutType>::Type>(strA[1]) * (dx * idy) +
           cast<BaseElement<OutType>::Type>(strB[0]) * (idx * dy) +
           cast<BaseElement<OutType>::Type>(strB[1]) * (dx * dy);
}

template <typename OutType, typename Type, typename FType>
OutType interpolate(const ImageRef<Type> & image, const Point2<FType> & point)
{
    using BaseOutType = typename BaseElement<OutType>::Type;

    int ix = cast<int>(std::floor(point.x)), iy = cast<int>(std::floor(point.y));
    FType dx = point.x - ix, dy = point.y - iy;
    FType idx = cast<FType>(1) - dx, idy = cast<FType>(1) - dy;
    const Type * strA = image.pointer(ix, iy);
    const Type * strB = &strA[image.widthStep()];
    return cast<BaseOutType>(strA[0]) * (idx * idy) +
           cast<BaseOutType>(strA[1]) * (dx * idy) +
           cast<BaseOutType>(strB[0]) * (idx * dy) +
           cast<BaseOutType>(strB[1]) * (dx * dy);
}

void sobel(Image<int> & out, const ImageRef<uchar> & in)
{
    assert(in.size() == out.size());

    const static signed char op[2][3] = { {1, 2, 1}, {-1, -2, -1} };

    // fill borders
    Image<int>(out, Point2i(0, 0), Point2i(out.width(), 1)).fill(0);
    Image<int>(out, Point2i(0, out.height() - 1), Point2i(out.width(), 1)).fill(0);
    Image<int>(out, Point2i(0, 1), Point2i(1, out.height() - 2)).fill(0);
    Image<int>(out, Point2i(out.width() - 1, 1), Point2i(1, out.height() - 2)).fill(0);

    const uchar * strPrev;
    const uchar * strCur = in.data();
    const uchar * strNext = &strCur[in.widthStep()];
    int * strOut = out.data();
    int Gx, Gy;
    int w = in.width() - 1;
    int h = in.height() - 1;
    Point2i p;
    for (p.y=1; p.y<h; ++p.y) {
        strPrev = strCur;
        strCur = strNext;
        strNext = &strNext[in.widthStep()];
        strOut = &strOut[out.widthStep()];
        for (p.x=1; p.x<w; ++p.x) {
            Gx = strPrev[p.x-1] * op[0][0] + strPrev[p.x] * op[0][1] + strPrev[p.x+1] * op[0][2];
            Gx += strNext[p.x-1] * op[1][0] + strNext[p.x] * op[1][1] + strNext[p.x+1] * op[1][2];
            Gy = strPrev[p.x-1] * op[0][0] + strCur[p.x-1] * op[0][1] + strPrev[p.x-1] * op[0][2];
            Gy += strPrev[p.x+1] * op[1][0] + strCur[p.x+1] * op[1][1] + strPrev[p.x+1] * op[1][2];

            strOut[p.x] = Gx * Gy;
        }
    }
}

Image<int> sobel(const ImageRef<uchar> & in)
{
    Image<int> r(in.size());
    sobel(r, in);
    return r;
}

void erode(Image<uchar> & out, const ImageRef<int> & integral, int size, float k)
{
    assert(out.size() == integral.size());
    assert((out.width() > size * 2) && (out.height() > size * 2));

    // fill borders
    Image<uchar>(out, Point2i(0, 0), Point2i(out.width(), size)).fill(0);
    Image<uchar>(out, Point2i(0, out.height() - size), Point2i(out.width(), size)).fill(0);
    Image<uchar>(out, Point2i(0, size), Point2i(size, out.height() - 2 * size)).fill(0);
    Image<uchar>(out, Point2i(out.width() - size, size), Point2i(size, out.height() - 2 * size)).fill(0);

    const int minCountPixels = static_cast<int>((size * 2 + 1) * (size * 2 + 1) * k);
    uchar * strOut;
    Point2i p;
    int w = integral.width() - size;
    int h = integral.height() - size;
    for (p.y = size; p.y < h; ++p.y) {
        strOut = &out.data()[p.y * out.widthStep()];
        const int * strA = &integral.data()[(p.y - size) * integral.widthStep()];
        const int * strB = &integral.data()[(p.y + size) * integral.widthStep()];
        for (p.x = size; p.x < w; ++p.x) {
            strOut[p.x] = ((strB[p.x + size] + strA[p.x - size] -
                    (strB[p.x - size] + strA[p.x + size])) > minCountPixels);
        }
    }
}

Image<uchar> erode(const ImageRef<int> & integral, int size, float k)
{
    Image<uchar> r(integral.size());
    erode(integral, size, k);
    return r;
}

template <typename RealSumType, typename Type, typename FType>
void gaussianBlurX(Image<Type> & out, const ImageRef<Type> & in, int halfSizeBlur, FType sigma)
{
    assert(out.size() == in.size());
    assert(!in.equalsSources(out));
    assert((in.width() > halfSizeBlur * 2 + 1));

    Point2i sizeImage = in.size();
    int beginCenterRegionX = halfSizeBlur;
    int endCenterRegionX = sizeImage.x - halfSizeBlur;
    Point2i p;
    int k = 0;
    FType sigmaSquare = sigma * sigma;
    int sizeKernel = halfSizeBlur * 2 + 1;
    std::vector<FType> g(sizeKernel);
    FType k1 = cast<FType>(1) / (sigma * std::sqrt(cast<FType>(2.0 * M_PI)));
    FType invK2 = cast<FType>(1) / (cast<FType>(2) * sigmaSquare);
    FType sumG = 0;
    for (p.x =- halfSizeBlur; p.x <= halfSizeBlur; ++p.x, ++k) {
        g[k] = k1 * std::exp(- p.x * p.x * invK2);
        sumG += g[k];
    }
    for (k = 0; k < sizeKernel; ++k) {
        g[k] /= sumG;
    }
    int c;
    typename Cast<Type, RealSumType>::Type sum;
    const Type * inStr = in.data();
    Type * outStr = out.data();
    for (p.y = 0; p.y < sizeImage.y; ++p.y) {
        for (p.x = 0; p.x < beginCenterRegionX; ++p.x) {
            sum = cast<RealSumType>(0);
            c = p.x - halfSizeBlur;
            for (k = 0; c <= 0; ++k, ++c)
                sum = sum + cast<RealSumType>(inStr[0] * g[k]);
            for (; k < sizeKernel; ++k, ++c)
                sum = sum + cast<RealSumType>(inStr[c] * g[k]);
            outStr[p.x] = cast<typename BaseElement<Type>::Type>(sum);
        }
        for (p.x = beginCenterRegionX; p.x < endCenterRegionX; ++p.x) {
            c = p.x - halfSizeBlur;
            sum = cast<RealSumType>(0);
            for (k = 0; k < sizeKernel; ++k, ++c)
                sum = sum + cast<RealSumType>(inStr[c] * g[k]);
            outStr[p.x] = cast<typename BaseElement<Type>::Type>(sum);
        }
        for (p.x = endCenterRegionX; p.x < sizeImage.x; ++p.x) {
            sum = cast<RealSumType>(0);
            c = p.x - halfSizeBlur;
            for (k = 0; c < sizeImage.x - 1; ++k, ++c)
                sum = sum + cast<RealSumType>(inStr[c] * g[k]);
            for (; k < sizeKernel; ++k)
                sum = sum + cast<RealSumType>(inStr[c] * g[k]);
            outStr[p.x] = cast<typename BaseElement<Type>::Type>(sum);
        }
        inStr = &inStr[in.widthStep()];
        outStr = &outStr[out.widthStep()];
    }
}

template <typename RealSumType, typename Type, typename FType>
void gaussianBlurY(Image<Type> & out, const ImageRef<Type> & in, int halfSizeBlur, FType sigma)
{
    assert(out.size() == in.size());
    assert(in.data() != out.data());
    assert((in.height() > halfSizeBlur * 2 + 1));

    Point2i sizeImage = in.size();
    int in_str_step = in.widthStep();
    int beginCenterRegionY = halfSizeBlur;
    int endCenterRegionY = sizeImage.y - halfSizeBlur;
    Point2i p;
    int sizeKernel = halfSizeBlur * 2 + 1;
    FType sigmaSquare = sigma * sigma;
    std::vector<FType> g(sizeKernel);
    FType k1 = cast<FType>(1) / (sigma * std::sqrt(cast<FType>(2.0 * M_PI)));
    FType invK2 = cast<FType>(1) / (cast<FType>(2) * sigmaSquare);
    int k = 0;
    FType sumG = 0;
    for (p.y = - halfSizeBlur; p.y <= halfSizeBlur; ++p.y, ++k) {
        g[k] = k1 * std::exp(- p.y * p.y * invK2);
        sumG += g[k];
    }
    for (k = 0; k < sizeKernel; ++k) {
        g[k] /= sumG;
    }
    typename Cast<Type, RealSumType>::Type sum;
    int c;
    int c_offset = - halfSizeBlur * in_str_step;
    const Type * inStr = in.data();
    Type * outStr = out.data();
    for (p.y = 0; p.y < beginCenterRegionY; ++p.y) {
        int c_limit = halfSizeBlur - p.y;
        for (p.x = 0; p.x < sizeImage.x; ++p.x) {
            c = p.x + c_offset;
            sum = cast<RealSumType>(0);
            for (k = 0; k < c_limit; ++k, c += in_str_step)
                sum = sum + cast<RealSumType>(inStr[p.x] * g[k]);
            for (; k < sizeKernel; ++k, c += in_str_step)
                sum = sum + cast<RealSumType>(inStr[c] * g[k]);
            outStr[p.x] = cast<typename BaseElement<Type>::Type>(sum);
        }
        inStr = &inStr[in_str_step];
        outStr = &outStr[out.widthStep()];
    }
    for (; p.y < endCenterRegionY; ++p.y) {
        for (p.x = 0; p.x < sizeImage.x; ++p.x) {
            c = p.x + c_offset;
            sum = cast<RealSumType>(0);
            for (k = 0; k < sizeKernel; ++k, c += in_str_step)
                sum = sum + cast<RealSumType>(inStr[c] * g[k]);
            outStr[p.x] = cast<typename BaseElement<Type>::Type>(sum);
        }
        inStr = &inStr[in_str_step];
        outStr = &outStr[out.widthStep()];
    }
    for (; p.y < sizeImage.y; ++p.y) {
        int c_limit = sizeKernel - (p.y - endCenterRegionY + 2);
        for (p.x = 0; p.x < sizeImage.x; ++p.x) {
            c = p.x + c_offset;
            sum = cast<RealSumType>(0);
            for (k=0; k < c_limit; ++k, c += in_str_step)
                sum = sum + cast<RealSumType>(inStr[c] * g[k]);
            for (; k < sizeKernel; ++k)
                sum = sum + cast<RealSumType>(inStr[c] * g[k]);
            outStr[p.x] = cast<typename BaseElement<Type>::Type>(sum);
        }
        inStr = &inStr[in_str_step];
        outStr = &outStr[out.widthStep()];
    }
}

template <typename RealSumType, typename Type, typename FType>
void gaussianBlur(Image<Type> & out, const ImageRef<Type> & in, int halfSizeBlur, FType sigma)
{
    Image<Type> temp(in.size());
    gaussianBlurX<RealSumType, Type, FType>(temp, in, halfSizeBlur, sigma);
    gaussianBlurY<RealSumType, Type, FType>(out, temp, halfSizeBlur, sigma);
}

void gaussianBlurX(Image<uchar> & out, const ImageRef<uchar> & in, int halfSizeBlur, float sigma)
{
    gaussianBlurX<float, uchar, float>(out, in, halfSizeBlur, sigma);
}

void gaussianBlurY(Image<uchar> & out, const ImageRef<uchar> & in, int halfSizeBlur, float sigma)
{
    gaussianBlurY<float, uchar, float>(out, in, halfSizeBlur, sigma);
}

void gaussianBlur(Image<uchar> & out, const ImageRef<uchar> & in, int halfSizeBlur, float sigma)
{
    gaussianBlur<float, uchar, float>(out, in, halfSizeBlur, sigma);
}

template <typename T, typename S, typename P>
int transform2x2(Image<T> & out, const ImageRef<S> & in, const Eigen::Matrix<P, 2, 2> & M,
                 const Point2<P> & outOrigin, const Point2<P> & inOrigin,
                 const T defaultValue = T())
{
    int outWidth = out.width(), outHeight = out.height(), inWidth = in.width(), inHeight = in.height();
    Point2<P> across(M(0, 0), M(1, 0));
    Point2<P> down(M(0, 1), M(1, 1));

    Point2<P> p0 = inOrigin - Point2<P>(M(0, 0) * outOrigin.x + M(0, 1) * outOrigin.y,
                                        M(1, 0) * outOrigin.x + M(1, 1) * outOrigin.y);

    Point2<P> min = p0, max = p0;

    if (across.x < 0)
        min.x += outWidth * across.x;
    else
        max.x += outWidth * across.x;

    if (down.x < 0)
        min.x += outHeight * down.x;
    else
        max.x += outHeight * down.x;

    if (across.y < 0)
        min.y += outWidth * across.y;
    else
        max.y += outWidth * across.y;

    if (down.y < 0)
        min.y += outHeight * down.y;
    else
        max.y += outHeight * down.y;

    Point2<P> carriage_return = down - across * outWidth;

    Point2<P> p = p0;
    S * outStr = out.data();

    if ((min.x >= 0) && (min.y >= 0) && (max.x < inWidth-1) && (max.y < inHeight-1)) {
        for (int i = 0; i < outHeight; ++i, p += carriage_return) {
            for (int j = 0; j < outWidth; ++j, p += across)
                outStr[j] = interpolate<T>(in, p);
            outStr = &outStr[out.widthStep()];
        }
        return 0;
    }

    Point2<P> bound(cast<P>(inWidth - 1), cast<P>(inHeight - 1));
    int count = 0;
    for (int i = 0; i < outHeight; ++i, p += carriage_return) {
        for (int j = 0; j < outWidth; ++j, p += across) {
            if ((cast<P>(0) <= p.x) && (cast<P>(0) <= p.y) &&  (p.x < bound.x) && (p.y < bound.y)) {
                outStr[j] = interpolate<T>(in, p);
            } else {
                outStr[j] = defaultValue;
                ++count;
            }
        }
        outStr = &outStr[out.widthStep()];
    }
    return count;
}

template <typename Type>
void normalize(Image<Type> & inout)
{
    Type min = std::numeric_limits<Type>::max();
    Type max = - min;

    inout.for_each([&min, &max] (const Point2i & p, Type & val) {
        (void)p;
        if (val < min)
            min = val;
        if (val > max)
            max = val;
    });

    Type delta = max - min;
    if (fabs(delta) < std::numeric_limits<Type>::epsilon()) {
        inout.fill(0.0f);
        return;
    }

    inout.for_each([&min, &delta] (const Point2i & p, Type & val) {
        (void)p;
        val = (Type)((val - min) / delta);
        if (val < Type(0))
            val = Type(0);
        else if (val > Type(1))
            val = Type(1);
    });
}

template <typename Type>
void normalize(Image<Type> & inout, Type min, Type max)
{
    Type delta = max - min;
    if (abs(delta) < std::numeric_limits<Type>::epsilon()) {
        inout.fill(0.0f);
        return;
    }

    inout.for_each([&min, &delta] (const Point2i & p, Type & val) {
        (void)p;
        val = (Type)((val - min) / delta);
        if (val < Type(0))
            val = Type(0);
        else if (val > Type(1))
            val = Type(1);
    });
}

} // namespace image_utils

} // namespace sonar

#endif // ORBI_IMAGEUTILS_IMPL_HPP
