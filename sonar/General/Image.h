/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_IMAGE_H
#define SONAR_IMAGE_H

#include <atomic>
#include <iostream>
#include <cassert>
#include <memory>
#include <cstring>
#include <functional>
#include <cmath>
#include <algorithm>

#include <sonar/General/Point2.h>
#include <sonar/General/cast.h>

#if defined(QT_CORE_LIB)
#include <qglobal.h>
#endif

using uchar = unsigned char;
using ushort = unsigned short;

namespace sonar {

template < typename Type >
class Rgb
{
public:
    using TypeValue = Type;

    Type red;
    Type green;
    Type blue;

    Rgb();
    inline Rgb(Type red, Type green, Type blue);
    inline void set(Type red, Type green, Type blue);

    template < typename MType >
    inline Rgb operator * (MType a) const;

    template < typename MType >
    inline Rgb operator / (MType a) const;

    inline Rgb operator + (const Rgb & a) const;
    inline void operator += (const Rgb & a);

    inline Rgb operator - (const Rgb & a) const;
    inline void operator -= (const Rgb & a);
};

template < typename Type >
class Rgba
{
public:
    using TypeValue = Type;

    Type red;
    Type green;
    Type blue;
    Type alpha;

    Rgba();
    inline Rgba(Type red, Type green, Type blue, Type alpha = 255);
    inline void set(Type red, Type green, Type blue, Type alpha = 255);

    template < typename MType >
    inline Rgba operator * (MType a) const;

    template < typename MType >
    inline Rgba operator / (MType a) const;

    inline Rgba operator + (const Rgba & a) const;
    inline void operator += (const Rgba & a);

    inline Rgba operator - (const Rgba & a) const;
    inline void operator -= (const Rgba & a);
};

using Rgb_u = Rgb<uchar>;
using Rgb_i = Rgb<int>;
using Rgb_f = Rgb<float>;

using Rgba_u = Rgba<uchar>;
using Rgba_i = Rgba<int>;
using Rgba_f = Rgba<float>;

template <typename Type>
class ImageRef;

template <typename Type>
class ConstImage;

template <typename Type>
class Image;

/// Базовый класс для Image и ConstImage, созданный чтобы можно было передавать по ссылке оба объекта
/// Оба класса наследуются от них
template <typename Type>
class ImageRef
{
public:
    typedef Type TypeValue;

public:
    inline bool equalsSources(const ImageRef<Type> & image) const;
    inline bool equals(const ImageRef<Type> & image) const;
    inline bool isNull() const;
    inline bool isContinuous() const;
    inline const Type * data() const;
    inline const Type & operator () (int x, int y) const;
    inline const Type & operator () (const Point2i & point) const;
    inline const Type * pointer(int x, int y) const;
    inline const Type * pointer(const Point2i & point) const;
    inline int area() const;
    inline int countBytes() const;
    inline Point2i size() const;
    inline int width() const;
    inline int height() const;
    inline int widthStep() const;
    inline bool autoDeleting() const;
    inline int numberReferences() const;

    inline Image<Type> copy() const;

    void for_each(const std::function<void(const Point2i & pos, const Type & val)> & func) const;

    template <typename ConvertType>
    void convertTo(const Image<ConvertType> & out,
                   const std::function<ConvertType(const Point2i & pos, const Type & a)> & convertFunction) const;

    template<typename ConvertType>
    Image<ConvertType> convert(const std::function<ConvertType(const Point2i & pos,
                                                               const Type & a)> & convertFunction) const;

    inline bool pointInImageWithBorder(const Point2i & point, int border) const;
    inline bool pointInImageWithBorder(int x, int y, int border) const;
    inline bool pointInImage(const Point2i & point) const;
    inline bool pointInImage(int x, int y) const;

    inline void swap(ImageRef<Type> & image);

private:
    friend class ConstImage<Type>;
    friend class Image<Type>;

    Type * m_sourceData;
    Type * m_data;
    volatile std::atomic_int * m_count_references;
    Point2i m_size;
    int m_widthStep;

    ImageRef();
    ~ImageRef();

    inline void _remove();
    inline void _allocData();
    inline void _copyRef(const ImageRef<Type> & image);
    inline void _moveRef(ImageRef<Type> && image);
    inline void _copyRef(const ImageRef<Type> & image, const Point2i & offset, const Point2i & size);
    inline void _moveRef(ImageRef<Type> && image, const Point2i & offset, const Point2i & size);
};

// То же, что Image, но изменять содержимое нельзя
template <typename Type>
class ConstImage: public ImageRef<Type>
{
public:
    ConstImage();
    ConstImage(const ConstImage<Type> & image);
    ConstImage(ConstImage<Type> && image);
    ConstImage(const ImageRef<Type> & image);
    ConstImage(ImageRef<Type> && image);
    ConstImage(const ImageRef<Type> & image, const Point2i & offset, const Point2i & size);
    ConstImage(ImageRef<Type> && image, const Point2i & offset, const Point2i & size);
    ConstImage(const Point2i & size, const Type * data, bool autoDeleting = true);
    ConstImage(int width, int height, const Type * data, bool autoDeleting = true);
    ConstImage(const Point2i & size, const Type * data, int widthStep, bool autoDeleting = true);
    ConstImage(int width, int height, const Type * data, int widthStep, bool autoDeleting = true);
    ~ConstImage();
    ConstImage & operator = (const ConstImage<Type> & image);
    ConstImage & operator = (ConstImage<Type> && image);
    ConstImage & operator = (const ImageRef<Type> & image);
    ConstImage & operator = (ImageRef<Type> && image);

    void release();
};

template <typename Type>
class Image: public ImageRef<Type>
{
public:
    Image();
    Image(const Image<Type> & image);
    Image(const Image<Type> & image, const Point2i & offset, const Point2i & size);
    Image(Image<Type> && image);
    Image(Image<Type> && image, const Point2i & offset, const Point2i & size);
    Image(const Point2i & size);
    Image(int width, int height);
    Image(const Point2i & size, Type * data, bool autoDeleting = true);
    Image(int width, int height, Type * data, bool autoDeleting = true);
    Image(const Point2i & size, Type * data, int widthStep, bool autoDeleting = true);
    Image(int width, int height, Type * data, int widthStep, bool autoDeleting = true);
    ~Image();

    void for_each(const std::function<void(const Point2i & pos, Type & val)> & func) const;

    operator ConstImage<Type>();
    Image<Type> & operator = (const Image<Type> & image);
    Image<Type> & operator = (Image<Type> && image);

    inline Type * data() const;
    inline Type & operator () (int x, int y) const;
    inline Type & operator () (const Point2i & point) const;
    inline Type * pointer(int x, int y) const;
    inline Type * pointer(const Point2i & point) const;
    void fill(const Type & val) const;

    inline void copyData(const ImageRef<Type> & imageSrc);

    void release();
};

} // namespace sonar

#include "impl/Image_impl.hpp"
#endif // SONAR_IMAGE_H
