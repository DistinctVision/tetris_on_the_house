/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_CAST_H
#define SONAR_CAST_H

#include <vector>

#if defined(OPENCV_LIB)
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#endif

// Умный cast, умеющий работать с преобразованиями примитивных типов, лежащий в основе шаблонов из модуля General.
// Позволяет менять, например, Point2i в Point2f <=> Point2f a = cast<float>(Point2i(1, 1));

namespace sonar {

template <typename Type>
class Point2;

template <typename Type>
class Rgb;

template <typename Type>
class Rgba;

template <typename T>
struct BaseElement
{
    using Type = T;
};

template <typename T>
struct BaseElement<Point2<T>>
{
    using Type = typename BaseElement<T>::Type;
};

template <typename T>
struct BaseElement<Rgb<T>>
{
    using Type = typename BaseElement<T>::Type;
};

template <typename T>
struct BaseElement<Rgba<T>>
{
    using Type = typename BaseElement<T>::Type;
};

template <typename T>
struct BaseElement<std::vector<T>>
{
    using Type = typename BaseElement<T>::Type;
};

template <typename T, typename CastT>
struct Cast
{
    using Type = CastT;
};

template <typename T, typename CastT>
struct Cast<Point2<T>, CastT>
{
    using Type = Point2<typename Cast<T, CastT>::Type>;
};

template <typename T, typename CastT>
struct Cast<Rgb<T>, CastT>
{
    using Type = Rgb<typename Cast<T, CastT>::Type>;
};

template <typename T, typename CastT>
struct Cast<Rgba<T>, CastT>
{
    using Type = Rgba<typename Cast<T, CastT>::Type>;
};

template <typename T, typename CastT>
struct Cast<std::vector<T>, CastT>
{
    using Type = std::vector<typename Cast<T, CastT>::Type>;
};

template <typename CastType>
inline static CastType cast(bool value);

template <typename CastType>
inline static CastType cast(char value);

template <typename CastType>
inline static CastType cast(unsigned char value);

template <typename CastType>
inline static CastType cast(short value);

template <typename CastType>
inline static CastType cast(unsigned short value);

template <typename CastType>
inline static CastType cast(int value);

template <typename CastType>
inline static CastType cast(unsigned int value);

template <typename CastType>
inline static CastType cast(const long & value);

template <typename CastType>
inline static CastType cast(const unsigned long & value);

template <typename CastType>
inline static CastType cast(const long long & value);

template <typename CastType>
inline static CastType cast(const unsigned long long & value);

template <typename CastType>
inline static CastType cast(float value);

template <typename CastType>
inline static CastType cast(const double & value);

template <typename CastType, typename Type>
inline static Point2<typename Cast<Type, CastType>::Type> cast(const Point2<Type> & value);

#if defined(OPENCV_LIB)
template <typename CastType, typename Type>
static cv::Point_<typename Cast<Type, CastType>::Type> cv_cast(const Point2<Type> & p);

template <typename CastType, typename Type>
static Point2<typename Cast<Type, CastType>::Type> cv_cast(const cv::Point_<Type> & p);

template <typename CastType, typename Type>
static cv::Point_<typename Cast<Type, CastType>::Type> cast(const cv::Point_<Type> & p);

template <typename CastType, typename Type>
static std::vector<cv::Point_<typename Cast<Type, CastType>::Type>> cv_cast(const std::vector<Point2<Type>> & points);

template <typename CastType, typename Type>
static std::vector<Point2<typename Cast<Type, CastType>::Type>> cv_cast(const std::vector<cv::Point_<Type>> & points);
#endif

template <typename CastType, typename Type>
static Rgb<typename Cast<Type, CastType>::Type> cast(const Rgb<Type> & value);

template <typename CastType, typename Type>
static Rgba<typename Cast<Type, CastType>::Type> cast(const Rgba<Type> & value);

template <typename CastType, typename Type>
static std::vector<typename Cast<Type, CastType>::Type> cast(const std::vector<Type> & elements);

} // namespace sonar

#include "impl/cast_impl.hpp"
#endif // SONAR_CAST_H
