/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_IMAGE_IMPL_HPP
#define SONAR_IMAGE_IMPL_HPP

namespace sonar {

template < typename Type >
Rgb<Type>::Rgb()
{}

template < typename Type >
Rgb<Type>::Rgb(Type red, Type green, Type blue)
{
    set(red, green, blue);
}

template < typename Type >
void Rgb<Type>::set(Type red, Type green, Type blue)
{
    this->red = red;
    this->green = green;
    this->blue = blue;
}

template < typename Type >
template < typename MType >
Rgb<Type> Rgb<Type>::operator * (MType a) const
{
    return Rgb<Type>(cast<Type>(this->red * a),
                     cast<Type>(this->green * a),
                     cast<Type>(this->blue * a));
}

template < typename Type >
template < typename MType >
Rgb<Type> Rgb<Type>::operator / (MType a) const
{
    return Rgb<Type>(cast<Type>(this->red / a),
                     cast<Type>(this->green / a),
                     cast<Type>(this->blue / a));
}

template < typename Type >
Rgb<Type> Rgb<Type>::operator + (const Rgb & a) const
{
    return Rgb<Type>((this->red + a.red),
                     (this->green + a.green),
                     (this->blue + a.blue));
}

template < typename Type >
void Rgb<Type>::operator += (const Rgb & a)
{
    this->red += a.red;
    this->green += a.green;
    this->blue += a.blue;
}

template < typename Type >
Rgb<Type> Rgb<Type>::operator - (const Rgb & a) const
{
    return Rgb<Type>((this->red - a.red),
                     (this->green - a.green),
                     (this->blue - a.blue));
}

template < typename Type >
void Rgb<Type>::operator -= (const Rgb & a)
{
    this->red -= a.red;
    this->green -= a.green;
    this->blue -= a.blue;
}

template < typename Type >
Rgba<Type>::Rgba()
{}

template < typename Type >
void Rgba<Type>::set(Type red, Type green, Type blue, Type alpha)
{
    this->red = red;
    this->green = green;
    this->blue = blue;
    this->alpha = alpha;
}

template < typename Type >
Rgba<Type>::Rgba(Type red, Type green, Type blue, Type alpha)
{
    set(red, green, blue, alpha);
}

template < typename Type >
template < typename MType >
Rgba<Type> Rgba<Type>::operator * (MType a) const
{
    return Rgba<Type>(cast<Type>(this->red * a),
                      cast<Type>(this->green * a),
                      cast<Type>(this->blue * a),
                      cast<Type>(this->alpha * a));
}

template < typename Type >
template < typename MType >
Rgba<Type> Rgba<Type>::operator / (MType a) const
{
    return Rgba<Type>(cast<Type>(this->red / a),
                      cast<Type>(this->green / a),
                      cast<Type>(this->blue / a),
                      cast<Type>(this->alpha / a));
}

template < typename Type >
Rgba<Type> Rgba<Type>::operator + (const Rgba &  a) const
{
    return Rgba((this->red + a.red),
                (this->green + a.green),
                (this->blue + a.blue),
                (this->alpha + a.alpha));
}

template < typename Type >
void Rgba<Type>::operator += (const Rgba & a)
{
    this->red += a.red;
    this->green += a.green;
    this->blue += a.blue;
    this->alpha += a.alpha;
}

template < typename Type >
Rgba<Type> Rgba<Type>::operator - (const Rgba & a) const
{
    return Rgba<Type>((this->red - a.red),
                      (this->green - a.green),
                      (this->blue - a.blue),
                      (this->alpha - a.alpha));
}

template < typename Type >
void Rgba<Type>::operator -= (const Rgba & a)
{
    this->red -= a.red;
    this->green -= a.green;
    this->blue -= a.blue;
    this->alpha -= a.alpha;
}

template < typename Type >
bool ImageRef<Type>::equalsSources(const ImageRef<Type> & image) const
{
    return (this->m_sourceData == image.m_sourceData);
}

template < typename Type >
bool ImageRef<Type>::equals(const ImageRef<Type> & image) const
{
    if (this->m_count_references != nullptr)
        return (this->m_count_references == image.m_count_references);
    return (this->m_data == image.m_data) && (this->m_size == image.m_size);
}

template < typename Type >
bool ImageRef<Type>::isNull() const
{
    return (this->m_sourceData == nullptr);
}

template < typename Type >
bool ImageRef<Type>::isContinuous() const
{
    return ((this->m_sourceData == this->m_data) && (this->m_size.x == this->m_widthStep));
}

template < typename Type >
const Type * ImageRef<Type>::data() const
{
    return this->m_data;
}

template < typename Type >
const Type & ImageRef<Type>::operator () (int x, int y) const
{
#if defined(QT_DEBUG)
    assert(this->pointInImage(x, y));
#endif
    return this->m_data[y * this->m_widthStep + x];
}

template < typename Type >
const Type & ImageRef<Type>::operator () (const Point2i & point) const
{
#if defined(QT_DEBUG)
    assert(this->pointInImage(point));
#endif
    return this->m_data[point.y * this->m_widthStep + point.x];
}

template < typename Type >
const Type * ImageRef<Type>::pointer(int x, int y) const
{
#if defined(QT_DEBUG)
    assert(this->pointInImage(x, y));
#endif
    return &this->m_data[y * this->m_widthStep + x];
}

template < typename Type >
const Type * ImageRef<Type>::pointer(const Point2i & point) const
{
#if defined(QT_DEBUG)
    assert(this->pointInImage(point));
#endif
    return &this->m_data[point.y * this->m_widthStep + point.x];
}

template < typename Type >
int ImageRef<Type>::area() const
{
    return this->m_size.y * this->m_size.x;
}

template < typename Type >
int ImageRef<Type>::countBytes() const
{
    return this->area() * sizeof(Type);
}

template < typename Type >
Point2i ImageRef<Type>::size() const
{
    return this->m_size;
}

template < typename Type >
int ImageRef<Type>::width() const
{
    return this->m_size.x;
}

template < typename Type >
int ImageRef<Type>::height() const
{
    return this->m_size.y;
}

template < typename Type >
int ImageRef<Type>::widthStep() const
{
    return this->m_widthStep;
}

template < typename Type >
bool ImageRef<Type>::autoDeleting() const
{
    return (this->m_count_references != nullptr);
}

template < typename Type >
int ImageRef<Type>::numberReferences() const
{
    return (this->m_count_references != nullptr) ?
                this->m_count_references->load() : (-1);
}

template < typename Type >
bool ImageRef<Type>::pointInImageWithBorder(const Point2i & point, int border) const
{
    return ((point.x >= border) && (point.y >= border) &&
            (point.x < (this->m_size.x - border)) && (point.y < (this->m_size.y - border)));
}

template < typename Type >
bool ImageRef<Type>::pointInImageWithBorder(int x, int y, int border) const
{
    return ((x >= border) && (y >= border) &&
            (x < (this->m_size.x - border)) && (y < (this->m_size.y - border)));
}

template < typename Type >
bool ImageRef<Type>::pointInImage(const Point2i & point) const
{
    return ((point.x >= 0) && (point.y >= 0) &&
            (point.x < this->m_size.x) && (point.y < this->m_size.y));
}

template < typename Type >
bool ImageRef<Type>::pointInImage(int x, int y) const
{
    return ((x >= 0) && (y >= 0) && (x < this->m_size.x) && (y < this->m_size.y));
}

template < typename Type >
void ImageRef<Type>::swap(ImageRef<Type> & image)
{
    std::swap(this->m_sourceData, image.m_sourceData);
    std::swap(this->m_data, image.m_data);
    std::swap(this->m_count_references, image.m_count_references);
    std::swap(this->m_size, image.m_size);
    std::swap(this->m_widthStep, image.m_widthStep);
}

template <typename Type>
void Image<Type>::copyData(const ImageRef<Type> & imageSrc)
{
    assert(this->m_sourceData != nullptr);
    assert(this->m_size == imageSrc.size());
    if (this->isContinuous() && imageSrc.isContinuous()) {
        std::memcpy(this->m_data, imageSrc.data(), imageSrc.countBytes());
    } else {
        Type * strDst = this->m_data;
        const Type * strSrc = imageSrc.data();
        for (int i = 0; i < this->m_size.y; ++i) {
            std::memcpy(strDst, strSrc, this->m_size.x * sizeof(Type));
            strDst = &strDst[this->m_widthStep];
            strSrc = &strSrc[imageSrc.widthStep()];
        }
    }
}

template < typename Type >
ImageRef<Type>::ImageRef()
{}

template < typename Type >
ImageRef<Type>::~ImageRef()
{}

template < typename Type >
void ImageRef<Type>::_remove()
{
    //m_size.set(0, 0);
    if (this->m_count_references == nullptr) {
        //m_data = nullptr;
        return;
    }
    if (this->m_count_references->load() <= 1) {
/*#if defined(_WIN32)
        _aligned_free(this->m_data);
#elif defined(__ANDROID__)
        free(this->m_data);
#endif*/
        delete [] this->m_sourceData;
        delete this->m_count_references;
    } else {
        --(*this->m_count_references);
    }
    this->m_sourceData = nullptr;
    this->m_data = nullptr;
    this->m_count_references = nullptr;
}

template < typename Type >
void ImageRef<Type>::_allocData()
{
    //assert((this->m_size.x >= 0) && (this->m_size.y >= 0));
/*#if defined(_WIN32)
    this->m_data = this->m_sourceData = static_cast<Type*>(_aligned_malloc(this->countBytes(), 16));
#elif defined(__ANDROID__)
    this->m_data = this->m_sourceData = static_cast<Type*>(memalign(16, this->countBytes());
#endif
    this->m_data = this->m_sourceData = new (m_sourceData)Type[this->area()];*/
    this->m_data = this->m_sourceData = new Type[area()];
}

template < typename Type >
void ImageRef<Type>::_copyRef(const ImageRef<Type> & image)
{
    this->m_sourceData = image.m_sourceData;
    this->m_data = image.m_data;
    this->m_size = image.m_size;
    this->m_widthStep = image.m_widthStep;
    if (image.m_count_references != nullptr) {
        ++(*image.m_count_references);
        this->m_count_references = image.m_count_references;
    } else {
        this->m_count_references = nullptr;
    }
}

template < typename Type >
void ImageRef<Type>::_moveRef(ImageRef<Type> && image)
{
    this->m_sourceData = image.m_sourceData;
    image.m_sourceData = nullptr;
    this->m_widthStep = image.m_widthStep;
    image.m_widthStep = 0;
    this->m_data = image.m_data;
    image.m_data = nullptr;
    this->m_size = image.m_size;
    image.m_size.set(0, 0);
    this->m_count_references = image.m_count_references;
    image.m_count_references = nullptr;
}

template < typename Type >
void ImageRef<Type>::_copyRef(const ImageRef<Type> & image, const Point2i & offset, const Point2i & size)
{
    assert((offset.x >= 0) && (offset.y >= 0));
    assert((image.m_size.x >= (offset.x + size.x)) && (image.m_size.y >= (offset.y + size.y)));
    this->m_data = &image.m_data[offset.y * image.m_widthStep + offset.x];
    this->m_size = size;
    if (this == &image)
        return;
    this->m_sourceData = image.m_sourceData;
    this->m_widthStep = image.m_widthStep;
    if (image.m_count_references != nullptr) {
        ++(*image.m_count_references);
        this->m_count_references = image.m_count_references;
    } else {
        this->m_count_references = nullptr;
    }
}

template < typename Type >
void ImageRef<Type>::_moveRef(ImageRef<Type> && image, const Point2i & offset, const Point2i & size)
{
    assert((offset.x >= 0) && (offset.y >= 0));
    assert((image.m_size.x >= (offset.x + size.x)) && (image.m_size.y >= (offset.y + size.y)));
    this->m_data = &image.m_data[offset.y * image.m_widthStep + offset.x];
    this->m_size = m_size;
    if (this == &image)
        return;
    this->m_sourceData = image.m_sourceData;
    this->m_widthStep = image.m_widthStep;
    this->m_count_references = image.m_count_references;
    image.m_sourceData = nullptr;
    image.m_widthStep = 0;
    image.m_data = nullptr;
    image.m_size.set(0, 0);
    image.m_count_references = nullptr;
}

template < typename Type >
ConstImage<Type>::ConstImage()
{
    this->m_sourceData = nullptr;
    this->m_data = nullptr;
    this->m_count_references = nullptr;
    this->m_size.setZero();
    this->m_widthStep = 0;
}

template < typename Type >
ConstImage<Type>::ConstImage(const ConstImage<Type> & image)
{
    this->_copyRef(image);
}

template < typename Type >
ConstImage<Type>::ConstImage(ConstImage<Type> && image)
{
    this->_moveRef(std::move(image));
}

template < typename Type >
ConstImage<Type>::ConstImage(const ImageRef<Type> & image)
{
    this->_copyRef(image);
}

template < typename Type >
ConstImage<Type>::ConstImage(ImageRef<Type> && image)
{
    this->_moveRef(std::move(image));
}

template < typename Type >
ConstImage<Type>::ConstImage(const ImageRef<Type> & image, const Point2i & offset, const Point2i & size)
{
    this->_copyRef(image, offset, size);
}

template < typename Type >
ConstImage<Type>::ConstImage(ImageRef<Type> && image, const Point2i & offset, const Point2i & size)
{
    this->_moveRef(std::move(image), offset, size);
}

template < typename Type >
ConstImage<Type>::ConstImage(const Point2i & size, const Type * data, bool autoDeleting)
{
    this->m_size = size;
    this->m_widthStep = size.x;
    this->m_sourceData = this->m_data = const_cast<Type*>(data);
    this->m_count_references = (autoDeleting) ? new std::atomic_int(1) : nullptr;
}

template < typename Type >
ConstImage<Type>::ConstImage(int width, int height, const Type * data, bool autoDeleting)
{
    this->m_size.set(width, height);
    this->m_widthStep = width;
    this->m_sourceData = this->m_data = const_cast<Type*>(data);
    this->m_count_references = (autoDeleting) ? new std::atomic_int(1) : nullptr;
}

template < typename Type >
ConstImage<Type>::ConstImage(const Point2i & size, const Type * data, int widthStep, bool autoDeleting)
{
    this->m_size = size;
    this->m_widthStep = widthStep;
    this->m_sourceData = this->m_data = const_cast<Type*>(data);
    this->m_count_references = (autoDeleting) ? new std::atomic_int(1) : nullptr;
}

template < typename Type >
ConstImage<Type>::ConstImage(int width, int height, const Type * data, int widthStep, bool autoDeleting)
{
    this->m_size.set(width, height);
    this->m_widthStep = widthStep;
    this->m_sourceData = this->m_data = const_cast<Type*>(data);
    this->m_count_references = (autoDeleting) ? new std::atomic_int(1) : nullptr;
}

template < typename Type >
ConstImage<Type>::~ConstImage()
{
    this->_remove();
}

template < typename Type >
ConstImage<Type> & ConstImage<Type>::operator = (const ConstImage<Type> & image)
{
    if (this == &image)
        return (*this);
    this->_remove();
    this->_copyRef(image);
    return (*this);
}

template < typename Type >
ConstImage<Type> & ConstImage<Type>::operator = (ConstImage<Type> && image)
{
    if (this == &image)
        return (*this);
    this->_remove();
    this->_moveRef(std::move(image));
    return (*this);
}

template < typename Type >
ConstImage<Type> & ConstImage<Type>::operator = (const ImageRef<Type> & image)
{
    if (this == &image)
        return (*this);
    this->_remove();
    this->_copyRef(image);
    return (*this);
}

template < typename Type >
ConstImage<Type> & ConstImage<Type>::operator = (ImageRef<Type> && image)
{
    if (this == &image)
        return (*this);
    this->_remove();
    this->_moveRef(std::move(image));
    return (*this);
}

template < typename Type >
void ConstImage<Type>::release()
{
    this->_remove();
    this->m_sourceData = nullptr;
    this->m_data = nullptr;
    this->m_count_references = nullptr;
    this->m_size.set(0, 0);
    this->m_widthStep = 0;
}

template <typename Type>
Image<Type>::Image()
{
    this->m_sourceData = nullptr;
    this->m_data = nullptr;
    this->m_count_references = nullptr;
    this->m_size.setZero();
    this->m_widthStep = 0;
}

template <typename Type>
Image<Type>::Image(const Image<Type> & image)
{
    this->_copyRef(image);
}

template <typename Type>
Image<Type>::Image(const Image<Type> & image, const Point2i & offset, const Point2i & size)
{
    this->_copyRef(image, offset, size);
}

template <typename Type>
Image<Type>::Image(Image<Type> && image)
{
    this->_moveRef(std::move(image));
}

template <typename Type>
Image<Type>::Image(Image<Type> && image, const Point2i & offset, const Point2i & size)
{
    this->_moveRef(std::move(image), offset, size);
}

template <typename Type>
Image<Type>::Image(const Point2i & size)
{
    this->m_size = size;
    this->m_widthStep = size.x;
    this->_allocData();
    this->m_count_references = new std::atomic_int(1);
}

template <typename Type>
Image<Type>::Image(int width, int height)
{
    this->m_size.set(width, height);
    this->m_widthStep = width;
    this->_allocData();
    this->m_count_references = new std::atomic_int(1);
}

template <typename Type>
Image<Type>::Image(const Point2i & size, Type * data, bool autoDeleting)
{
    this->m_size = size;
    this->m_widthStep = size.x;
    this->m_sourceData = this->m_data = data;
    this->m_count_references = autoDeleting ? new std::atomic_int(1) : nullptr;
}

template <typename Type>
Image<Type>::Image(int width, int height, Type * data, bool autoDeleting)
{
    this->m_size.set(width, height);
    this->m_widthStep = width;
    this->m_sourceData = this->m_data = data;
    this->m_count_references = autoDeleting ? new std::atomic_int(1) : nullptr;
}

template <typename Type>
Image<Type>::Image(const Point2i & size, Type * data, int widthStep, bool autoDeleting)
{
    this->m_size = size;
    this->m_widthStep = widthStep;
    this->m_sourceData = this->m_data = data;
    this->m_count_references = autoDeleting ? new std::atomic_int(1) : nullptr;
}

template <typename Type>
Image<Type>::Image(int width, int height, Type * data, int widthStep, bool autoDeleting)
{
    this->m_size.set(width, height);
    this->m_widthStep = widthStep;
    this->m_sourceData = this->m_data = data;
    this->m_count_references = autoDeleting ? new std::atomic_int(1) : nullptr;
}

template <typename Type>
Image<Type>::~Image()
{
    this->_remove();
}

template <typename Type>
Image<Type>::operator ConstImage<Type>()
{
    return ConstImage<Type>(*this);
}

template <typename Type>
Image<Type> & Image<Type>::operator = (const Image<Type> & image)
{
    if (this == &image)
        return (*this);
    this->_remove();
    this->_copyRef(image);
    return (*this);
}

template <typename Type>
Image<Type> & Image<Type>::operator = (Image<Type> && image)
{
    if (this == &image)
        return (*this);
    this->_remove();
    this->_moveRef(std::move(image));
    return (*this);
}

template <typename Type>
Type * Image<Type>::data() const
{
    return this->m_data;
}

template <typename Type>
Type & Image<Type>::operator () (int x, int y) const
{
#if defined(QT_DEBUG)
    assert(this->pointInImage(x, y));
#endif
    return this->m_data[y * this->m_widthStep + x];
}

template <typename Type>
Type & Image<Type>::operator () (const Point2i & point) const
{
#if defined(QT_DEBUG)
    assert(this->pointInImage(point));
#endif
    return this->m_data[point.y * this->m_widthStep + point.x];
}

template <typename Type>
Type * Image<Type>::pointer(int x, int y) const
{
#if defined(QT_DEBUG)
    assert(this->pointInImage(x, y));
#endif
    return &this->m_data[y * this->m_widthStep + x];
}

template <typename Type>
Type * Image<Type>::pointer(const Point2i & point) const
{
#if defined(QT_DEBUG)
    assert(this->pointInImage(point));
#endif
    return &this->m_data[point.y * this->m_widthStep + point.x];
}

template <typename Type>
void Image<Type>::fill(const Type & val) const
{
    //TODO may be use memset
    if (this->isContinuous()) {
        int sz = this->m_size.y * this->m_size.x;
        for (int i = 0; i < sz; this->m_data[i] = val, ++i);
    } else {
        int i, j;
        Type * str = this->m_data;
        for (i = 0; i < this->m_size.y; ++i) {
            for (j = 0; j < this->m_size.x; str[j] = val, ++j);

            str = &str[this->m_widthStep];
        }
    }
}

template <typename Type>
Image<Type> ImageRef<Type>::copy() const
{
    if (this->isNull())
        return Image<Type>();
    Image<Type> image(m_size);
    if (this->isContinuous()) {
        std::memcpy(image.data(), this->m_data, this->countBytes());
    } else {
        Type * outStr = image.data();
        const Type * str = this->m_data;
        for (int i = 0; i < this->m_size.y; ++i) {
            std::memcpy(outStr, str, this->m_size.x * sizeof(Type));
            str = &str[this->m_widthStep];
            outStr = &outStr[image.widthStep()];
        }
    }
    return image;
}

template <typename Type>
template <typename ConvertType>
void ImageRef<Type>::convertTo(const Image<ConvertType> & out,
                               const std::function<ConvertType(const Point2i & pos, const Type & a)> &
                               convertFunction) const
{
    assert(this->m_size == out.size());
    Type * str = this->m_data;
    ConvertType * outStr = out.data();
    Point2i p;
    for (p.y = 0; p.y < this->m_size.y; ++p.y) {
        for (p.x = 0; p.x < this->m_size.x; ++p.x) {
            outStr[p.x] = convertFunction(p, str[p.x]);
        }
        str = &str[this->m_widthStep];
        outStr = &outStr[out.widthStep()];
    }
}

template <typename Type>
template <typename ConvertType>
Image<ConvertType> ImageRef<Type>::convert(const std::function<ConvertType(const Point2i & pos, const Type & a)> &
                                           convertFunction) const
{
    Image<ConvertType> image(this->m_size);
    convertTo<ConvertType>(image, convertFunction);
    return image;
}

template <typename Type>
void ImageRef<Type>::for_each(const std::function<void(const Point2i & pos, const Type & val)> & func) const
{
    const Type * str = this->m_data;
    Point2i p;
    for (p.y = 0; p.y < this->m_size.y; ++p.y) {
        for (p.x = 0; p.x < this->m_size.x; ++p.x) {
            func(p, str[p.x]);
        }
        str = &str[this->m_widthStep];
    }
}

template <typename Type>
void Image<Type>::for_each(const std::function<void(const Point2i & pos, Type & val)> & func) const
{
    Type * str = this->m_data;
    Point2i p;
    for (p.y = 0; p.y < this->m_size.y; ++p.y) {
        for (p.x = 0; p.x < this->m_size.x; ++p.x) {
            func(p, str[p.x]);
        }
        str = &str[this->m_widthStep];
    }
}

template < typename Type >
void Image<Type>::release()
{
    this->_remove();
    this->m_sourceData = nullptr;
    this->m_data = nullptr;
    this->m_count_references = nullptr;
    this->m_size.set(0, 0);
    this->m_widthStep = 0;
}

} // namespace sonar

#endif // SONAR_IMAGE_IMPL_HPP
