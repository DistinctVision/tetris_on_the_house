/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_IMAGE_PYRAMID_IMPL_HPP
#define SONAR_IMAGE_PYRAMID_IMPL_HPP

namespace sonar {

template < typename Type, typename SamplerType >
ImagePyramid<Type, SamplerType>::ImagePyramid()
{}

template < typename Type, typename SamplerType >
ImagePyramid<Type, SamplerType>::ImagePyramid(const ImageRef<Type> & image, int numberLevels)
{
    if ((numberLevels <= 0) || (image.isNull()))
        return;
    this->m_level0 = image;
    --numberLevels;
    if (numberLevels > 0) {
        this->m_levels.resize(numberLevels);
        ConstImage<Type> prev = image;
        for (int i = 0; i < static_cast<int>(m_levels.size()); ++i) {
            this->m_levels[i] = image_utils::halfSample<SamplerType, Type>(prev);
            prev = this->m_levels[i];
        }
    }
}

template < typename Type, typename SamplerType >
ImagePyramid<Type, SamplerType>::ImagePyramid(const ImagePyramid<Type, SamplerType> & pyramid, int numberLevels)
{
    if ((numberLevels <= 0) || (pyramid.isNull()))
        return;
    int i;
    this->m_level0 = pyramid.m_level0;
    --numberLevels;
    if (numberLevels <= 0)
        return;
    this->m_levels.resize(numberLevels);
    if (this->m_levels.size() > pyramid.m_levels.size()) {
        ConstImage<Type> prev = this->m_level0;
        for (i = 0; i < static_cast<int>(pyramid.m_levels.size()); ++i) {
            this->m_levels[i] = pyramid.m_levels[i];
            prev = this->m_levels[i];
        }
        for (i = static_cast<int>(pyramid.m_levels.size());
             i < static_cast<int>(this->m_levels.size());
             ++i) {
            this->m_levels[i] = image_utils::halfSample<SamplerType, Type>(prev);
            prev = this->m_levels[i];
        }
    } else {
        for (i = 0; i < static_cast<int>(this->m_levels.size()); ++i) {
            this->m_levels[i] = pyramid.m_levels[i];
        }
    }
}

template < typename Type, typename SamplerType >
int ImagePyramid<Type, SamplerType>::numberLevels() const
{
    if (this->m_level0.isNull())
        return 0;
    return 1 + static_cast<int>(this->m_levels.size());
}

template < typename Type, typename SamplerType >
bool ImagePyramid<Type, SamplerType>::isNull() const
{
    return this->m_level0.isNull();
}

template < typename Type, typename SamplerType >
void ImagePyramid<Type, SamplerType>::clear()
{
    this->m_level0 = Image<Type>();
    this->m_levels.clear();
}

template < typename Type, typename SamplerType >
ConstImage<Type> ImagePyramid<Type, SamplerType>::get(int level) const
{
    if (level == 0)
        return this->m_level0;
    return this->m_levels.at(level - 1);
}

template < typename Type, typename SamplerType >
void ImagePyramid<Type, SamplerType>::rebuild(const ImageRef<Type> & image)
{
    if (this->isNull()) {
        return;
    }
    if (image.isNull()) {
        this->clear();
        return;
    }
    ConstImage<Type> prev = image;
    if (image.size() == this->m_level0.size()) {
        this->m_level0 = image;
        for (int i = 0; i < static_cast<int>(this->m_levels.size()); ++i) {
            if (this->m_levels[i].numberReferences() == 1)
                image_utils::halfSample<SamplerType, Type>(this->m_levels[i], prev);
            else
                this->m_levels[i] = image_utils::halfSample<SamplerType, Type>(prev);
            prev = this->m_levels[i];
        }
    } else {
        this->m_level0 = image;
        for (int i = 0; i < static_cast<int>(this->m_levels.size()); ++i) {
            this->m_levels[i] = image_utils::halfSample<SamplerType, Type>(prev);
            prev = this->m_levels[i];
        }
    }
}

template < typename Type, typename SamplerType >
void ImagePyramid<Type, SamplerType>::rebuild(const ImageRef<Type> & image, int numberLevels)
{
    if ((numberLevels <= 0) || (image.isNull())) {
        this->clear();
        return;
    }
    --numberLevels;
    ConstImage<Type> prev = image;
    int i;
    if (image.size() == this->m_level0.size()) {
        this->m_level0 = image;
        if (numberLevels > (int)this->m_levels.size()) {
            int oldNumberLevels = (int)this->m_levels.size();
            this->m_levels.resize(numberLevels);
            for (i = 0; i < oldNumberLevels; ++i) {
                if (this->m_levels[i].numberReferences() == 1)
                    image_utils::halfSample<SamplerType, Type>(this->m_levels[i], prev);
                else
                    this->m_levels[i] = image_utils::halfSample<SamplerType, Type>(prev);
                prev = this->m_levels[i];
            }
            for (i = oldNumberLevels; i < numberLevels; ++i) {
                this->m_levels[i] = image_utils::halfSample<SamplerType, Type>(prev);
                prev = this->m_levels[i];
            }
        } else {
            this->m_levels.resize(numberLevels);
            for (i = 0; i < numberLevels; ++i) {
                if (this->m_levels[i].numberReferences() == 1)
                    image_utils::halfSample<SamplerType, Type>(this->m_levels[i], prev);
                else
                    this->m_levels[i] = image_utils::halfSample<SamplerType, Type>(prev);
                prev = this->m_levels[i];
            }
        }
    } else {
        this->m_level0 = image;
        this->m_levels.resize(numberLevels);
        for (i = 0; i < numberLevels; ++i) {
            this->m_levels[i] = image_utils::halfSample<SamplerType, Type>(prev);
            prev = this->m_levels[i];
        }
    }
}

template < typename Type, typename SamplerType >
void ImagePyramid<Type, SamplerType>::rebuild(int numberLevels)
{
    if (this->isNull()) {
        return;
    }
    if (numberLevels == 0) {
        this->clear();
        return;
    }
    --numberLevels;
    int oldNumberLevels = static_cast<int>(this->m_levels.size());
    this->m_levels.resize(numberLevels);
    if (numberLevels > oldNumberLevels) {
        ConstImage<Type> prev = (oldNumberLevels == 0) ? this->m_level0 :
                                                         this->m_levels[oldNumberLevels - 1];
        for (int i = oldNumberLevels; i < numberLevels; ++i) {
            this->m_levels[i] = image_utils::halfSample<SamplerType, Type>(prev);
            prev = this->m_levels[i];
        }
    }
}

template < typename Type, typename SamplerType >
ImagePyramid<Type, SamplerType> ImagePyramid<Type, SamplerType>::copy() const
{
    ImagePyramid<Type, SamplerType> r;
    r.m_level0 = this->m_level0.copy();
    r.m_levels.resize(m_levels.size());
    for (int i = 0; i < (int)this->m_levels.size(); ++i) {
        r.m_levels[i] = this->m_levels[i].copy();
    }
    return r;
}

template < typename Type, typename SamplerType >
ImagePyramid<Type, SamplerType> ImagePyramid<Type, SamplerType>::copy(int numberLevels) const
{
    ImagePyramid<Type, SamplerType> r;
    if ((this->isNull()) || (numberLevels == 0))
        return r;
    r.m_level0 = this->m_level0.copy();
    --numberLevels;
    r.m_levels.resize(numberLevels);
    int i;
    if (r.m_levels.size() > this->m_levels.size()) {
        ConstImage<Type> prev = this->m_level0;
        for (i = 0; i < (int)this->m_levels.size(); ++i) {
            r.m_levels[i] = this->m_levels[i].copy();
            prev = r.m_levels[i];
        }
        for (i = (int)this->m_levels.size(); i < (int)r.m_levels.size(); ++i) {
            r.m_levels[i] = image_utils::halfSample<SamplerType, Type>(prev);
            prev = r.m_levels[i];
        }
    } else {
        for (i = 0; i < (int)r.m_levels.size(); ++i) {
            r.m_levels[i] = this->m_levels[i].copy();
        }
    }
    return r;
}

} // namespace sonar

#endif // SONAR_IMAGE_PYRAMID_IMPL_HPP;
