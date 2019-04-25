/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_POINT2_IMPL_HPP
#define SONAR_POINT2_IMPL_HPP

namespace sonar {

template < typename Type >
Point2<Type>::Point2()
{}

template < typename Type >
Point2<Type>::Point2(Type value)
{
    this->x = value;
    this->y = value;
}

template < typename Type >
void Point2<Type>::set(Type x, Type y)
{
    this->x = x;
    this->y = y;
}

template < typename Type >
void Point2<Type>::setZero()
{
    set(cast<Type>(0), cast<Type>(0));
}

template < typename Type >
Point2<Type>::Point2(Type x, Type y)
{
    set(x, y);
}

template < typename Type >
Point2<Type> Point2<Type>::operator + (const Point2<Type> & p) const
{
    return Point2(x + p.x, y + p.y);
}

template < typename Type >
Point2<Type> Point2<Type>::operator - (const Point2<Type> & p) const
{
    return Point2(x - p.x, y - p.y);
}

template < typename Type >
void Point2<Type>::operator += (const Point2<Type> & p)
{
    x += p.x;
    y += p.y;
}

template < typename Type >
void Point2<Type>::operator -= (const Point2<Type> & p)
{
    x -= p.x;
    y -= p.y;
}

template < typename Type >
Point2<Type> Point2<Type>::operator - () const
{
    return Point2(- x, - y);
}

template < typename Type >
template <typename TypeB>
Point2<Type> Point2<Type>::operator * (TypeB a) const
{
    return Point2((Type)(x * a), (Type)(y * a));
}

template < typename Type >
template <typename TypeB>
Point2<Type> Point2<Type>::operator / (TypeB a) const
{
    return Point2(cast<Type>(x / a), cast<Type>(y / a));
}

template < typename Type >
template <typename TypeB>
void Point2<Type>::operator *= (TypeB a)
{
    x *= a;
    y *= a;
}

template < typename Type >
template <typename TypeB>
void Point2<Type>::operator /= (TypeB a)
{
    x /= a;
    y /= a;
}

template < typename Type >
bool Point2<Type>::operator == (const Point2 & p) const
{
    return ((x == p.x) && (y == p.y));
}

template < typename Type >
bool Point2<Type>::operator != (const Point2 & p) const
{
    return ((x != p.x) || (y != p.y));
}

template < typename Type >
Type Point2<Type>::lengthSquared() const
{
    return (x * x + y * y);
}

template < typename Type >
Type Point2<Type>::length() const
{
    return cast<Type>(std::sqrt(lengthSquared()));
}

template < typename Type >
Type Point2<Type>::normalize()
{
    Type l = length();
    if (l > std::numeric_limits<Type>::epsilon()) {
        x /= l;
        y /= l;
    } else {
        x = y = cast<Type>(0);
        return cast<Type>(0);
    }
    return l;
}

template < typename TypeA, typename TypeB >
Point2<TypeA> operator * (TypeB a, const Point2<TypeA> & b)
{
    return Point2<TypeA>(cast<TypeA>(b.x * a), cast<TypeA>(b.y * a));
}

template <typename Type>
Type Point2<Type>::dot(const Point2 & b) const
{
    return x * b.x + y * b.y;
}

} // namespace sonar

#endif // SONAR_POINT2_IMPL_HPP
