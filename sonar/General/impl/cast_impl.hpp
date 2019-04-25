/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_CAST_IMPL_HPP
#define SONAR_CAST_IMPL_HPP

namespace sonar {

template <typename CastType>
CastType cast(bool value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(char value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(unsigned char value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(short value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(unsigned short value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(int value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(unsigned int value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(const long & value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(const unsigned long & value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(const long long & value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(const unsigned long long & value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(float value)
{
    return static_cast<CastType>(value);
}

template <typename CastType>
CastType cast(const double & value)
{
    return static_cast<CastType>(value);
}

template <typename CastType, typename Type>
Point2<typename Cast<Type, CastType>::Type> cast(const Point2<Type> & p)
{
    return Point2<typename Cast<Type, CastType>::Type>(cast<CastType>(p.x),
                                                       cast<CastType>(p.y));
}

#if defined(OPENCV_LIB)
template <typename CastType, typename Type>
cv::Point_<typename Cast<Type, CastType>::Type> cv_cast(const Point2<Type> & p)
{
    return cv::Point_<typename Cast<Type, CastType>::Type>(cast<CastType>(p.x),
                                                           cast<CastType>(p.y));
}

template <typename CastType, typename Type>
Point2<typename Cast<Type, CastType>::Type> cv_cast(const cv::Point_<Type> & p)
{
    return Point2<typename Cast<Type, CastType>::Type>(cast<CastType>(p.x),
                                                       cast<CastType>(p.y));
}

template <typename CastType, typename Type>
static cv::Point_<typename Cast<Type, CastType>::Type> cast(const cv::Point_<Type> & p)
{
    return cv::Point_<typename Cast<Type, CastType>::Type>(cast<CastType>(p.x),
                                                           cast<CastType>(p.y));
}

template <typename CastType, typename Type>
static std::vector<cv::Point_<typename Cast<Type, CastType>::Type>> cv_cast(const std::vector<Point2<Type>> & points)
{
    std::vector<cv::Point_<typename Cast<Type, CastType>::Type>> castPoints(points.size());
    for (std::size_t i = 0; i < points.size(); ++i)
        castPoints[i] = cv_cast<CastType>(points[i]);
    return castPoints;
}

template <typename CastType, typename Type>
static std::vector<Point2<typename Cast<Type, CastType>::Type>> cv_cast(const std::vector<cv::Point_<Type>> & points)
{
    std::vector<Point2<typename Cast<Type, CastType>::Type>> castPoints(points.size());
    for (std::size_t i = 0; i < points.size(); ++i)
        castPoints[i] = cv_cast<CastType>(points[i]);
    return castPoints;
}
#endif

template <typename CastType, typename Type>
Rgb<typename Cast<Type, CastType>::Type> cast(const Rgb<Type> & value)
{
    return Rgb<typename Cast<Type, CastType>::Type>(cast<CastType>(value.red),
                                                    cast<CastType>(value.green),
                                                    cast<CastType>(value.blue));
}

template <typename CastType, typename Type>
Rgba<typename Cast<Type, CastType>::Type> cast(const Rgba<Type> & value)
{
    return Rgba<typename Cast<Type, CastType>::Type>(cast<CastType>(value.red),
                                                     cast<CastType>(value.green),
                                                     cast<CastType>(value.blue),
                                                     cast<CastType>(value.alpha));
}

template <typename CastType, typename Type>
static std::vector<typename Cast<Type, CastType>::Type> cast(const std::vector<Type> & elements)
{
    std::vector<typename Cast<Type, CastType>::Type> castElements(elements.size());
    for (std::size_t i = 0; i < elements.size(); ++i)
        castElements[i] = cast<CastType>(elements[i]);
    return castElements;
}

} // namespace sonar

#endif // SONAR_CAST_IMPL_HPP
