/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_IMAGE_PYRAMID_H
#define SONAR_IMAGE_PYRAMID_H

#include <cassert>
#include <vector>

#include <sonar/General/Image.h>
#include <sonar/General/ImageUtils.h>

namespace sonar {

// Класс для удобной рабогты с пирамидой изображений
// SamplerType - структура, в которой храниться логика сэмплирования.
// по умолчанию уровни будут усредняться с типом сумирования самого типа изображения,
// но при работе с uchar, нпаример, лучше выбрать int (чтобы избежать переполнения)
template < typename Type,
           typename SamplerType = image_utils::Sampler_avg<typename BaseElement<Type>::Type>>
class ImagePyramid
{
public:
    ImagePyramid();
    ImagePyramid(const ImageRef<Type> & image, int numberLevels);
    ImagePyramid(const ImagePyramid & pyramid, int numberLevels);

    int numberLevels() const;
    bool isNull() const;

    ConstImage<Type> get(int level = 0) const;

    void clear();

    void rebuild(const ImageRef<Type> & image);
    void rebuild(const ImageRef<Type> & image, int numberLevels);
    void rebuild(int numberLevels);

    ImagePyramid<Type, SamplerType> copy() const;
    ImagePyramid<Type, SamplerType> copy(int numberLevels) const;

private:
    ConstImage<Type> m_level0;
    std::vector<Image<Type>> m_levels;
};

using ImagePyramid_u = ImagePyramid<uchar, image_utils::Sampler_avg<int>>;
using ImagePyramid_f = ImagePyramid<float, image_utils::Sampler_avg<float>>;

} // namespace sonar

#include "impl/ImagePyramid_impl.hpp"
#endif // SONAR_IMAGE_PYRAMID_H
