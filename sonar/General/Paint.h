/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_PAINT_H
#define SONAR_PAINT_H

#include <cassert>
#include <utility>

#include <sonar/General/Point2.h>
#include <sonar/General/Image.h>

namespace sonar {

namespace paint {

template < typename Type >
static void drawLine(const Image<Type>& image, float x1, float y1, float x2, float y2, const Type & color);

template < typename Type >
static void drawLine(const Image<Type> & image, const Point2i & p1, const Point2i & p2, const Type & color);

template < typename Type >
static void drawLine(const Image<Type> & image, const Point2f & p1, const Point2f & p2, const Type & color);

template < typename Type >
static void drawBox(const Image<Type> & image, float upperleftX, float upperleftY, float lowerrightX, float lowerrightY,
                    const Type & color);

template < typename Type >
static void drawBox(const Image<Type> & image, const Point2i & upperleft, const Point2i & lowerright, const Type & color);

template < typename Type >
static void drawBox(const Image<Type> & image, const Point2f & upperleft, const Point2f & lowerright, const Type & color);

template < typename Type >
static void drawFillBox(const Image<Type> & image, Point2i upperleft, Point2i lowerright, const Type & color);

template < typename Type >
static void drawCross(const Image<Type> & image, float x, float y, float len, const Type & color);

template < typename Type >
static void drawCross(const Image<Type> & image, const Point2i & p, float len, const Type & color);

template < typename Type >
static void drawCross(const Image<Type> & image, const Point2f & p, float len, const Type & color);

template < typename Type >
static void drawImage(const Image<Type> & canvasImage, const ImageRef<Type> & image, const Point2f & pos, const Point2f & size);

} // namespace painter

} // namespace sonar

#include "impl/Paint_impl.hpp"
#endif // SONAR_PAINT_H
