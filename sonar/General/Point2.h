/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_POINT2_H
#define SONAR_POINT2_H

#include <limits>
#include <climits>
#include <vector>
#include <cmath>

#include <sonar/General/cast.h>

namespace sonar {

template < typename Type >
class Point2
{
public:
    using TypeValue = Type;

    Type x, y;

    Point2();
    Point2(Type value);

    inline Point2(Type x, Type y);
    inline Point2 operator + (const Point2 & p) const;
    inline Point2 operator - (const Point2 & p) const;
    inline void set(Type x, Type y);
    inline void setZero();
    inline void operator += (const Point2 & p);
    inline void operator -= (const Point2 & p);
    inline Point2 operator - () const;

    template <typename TypeB>
    inline Point2 operator * (TypeB a) const;

    template <typename TypeB>
    inline Point2 operator / (TypeB a) const;

    template <typename TypeB>
    inline void operator *= (TypeB a);

    template <typename TypeB>
    inline void operator /= (TypeB a);

    inline bool operator == (const Point2 & p) const;
    inline bool operator != (const Point2 & p) const;
    inline Type lengthSquared() const;
    inline Type length() const;
    inline Type normalize();

    Type dot(const Point2 & b) const;
};

template <typename TypeA, typename TypeB>
static Point2<TypeA> operator * (TypeB a, const Point2<TypeA> & b);

using Point2i = Point2<int>;
using Point2f = Point2<float>;
using Point2d = Point2<double>;

} // namespace sonar

#include "impl/Point2_impl.hpp"
#endif // SONAR_POINT2_H
