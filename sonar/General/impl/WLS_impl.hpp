/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_WLS_IMPL_HPP
#define SONAR_WLS_IMPL_HPP

namespace sonar {

template < typename Type, int _size, class DecompositionType >
WLS<Type, _size, DecompositionType>::WLS()
{
    assert(_size > 0);
    clear();
}

template < typename Type, int _size, class DecompositionType >
WLS<Type, _size, DecompositionType>::WLS(int size):
    m_AtA(size, size),
    m_B(size)
{
    assert(_size == Eigen::Dynamic);
    clear();
}

template < typename Type, int _size, class DecompositionType >
int WLS<Type, _size, DecompositionType>::size() const
{
    return (int)m_AtA.rows();
}

template < typename Type, int _size, class DecompositionType >
void WLS<Type, _size, DecompositionType>::clear()
{
    m_AtA.setZero();
    m_B.setZero();
}

template < typename Type, int _size, class DecompositionType >
void WLS<Type, _size, DecompositionType>::addMeasurement(Type m, const Eigen::Matrix<Type, _size, 1> & J)
{
    //Upper right triangle only, for speed
    for(int r = 0; r < m_AtA.rows(); ++r) {
        m_B(r) += m * J(r);
        for(int c = r; c < m_AtA.cols(); ++c)
            m_AtA(r, c) += J(r) * J(c);
    }
}

template < typename Type, int _size, class DecompositionType >
void WLS<Type, _size, DecompositionType>::addMeasurement(Type m, const Eigen::Matrix<Type, _size, 1> & J, Type weight)
{
    //Upper right triangle only, for speed
    for(int r = 0; r < m_AtA.rows(); ++r) {
        Type Jw = weight * J(r);
        m_B(r) += m * Jw;
        for(int c = r; c < m_AtA.cols(); ++c)
            m_AtA(r, c) += Jw * J(c);
    }
}

template < typename Type, int _size, class DecompositionType >
Eigen::Matrix<Type, _size, 1> WLS<Type, _size, DecompositionType>::compute()
{
    //Copy the upper right triangle to the empty lower-left.
    int r, c;
    for(r = 1; r < m_AtA.rows(); ++r)
        for(c = 0; c < r; ++c)
            m_AtA(r, c) = m_AtA(c, r);

    DecompositionType d(m_AtA);
    return d.solve(m_B);
}

template < typename Type, int _size, class DecompositionType >
void WLS<Type, _size, DecompositionType>::addPrior(Type lambda)
{
    lambda += 1.0;
    //Upper right triangle only, for speed
    for(int r = 0; r < m_AtA.rows(); ++r) {
        m_AtA(r, r) *= lambda;
    }
}

} // namespace sonar

#endif // SONAR_WLS_IMPL_HPP
