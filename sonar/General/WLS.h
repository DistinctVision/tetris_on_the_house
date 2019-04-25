/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_WLS_H
#define SONAR_WLS_H

#include <cassert>

#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Eigen>

namespace sonar {

/// Performs weighted least squares computation.
/// A * x = b
template < typename Type, int _size,
           class DecompositionType = Eigen::ColPivHouseholderQR<Eigen::Matrix<Type, _size, _size>>>
class WLS
{
public:
    WLS();

    WLS(int size);

    int size() const;

    /// Clear all the measurements and apply a constant regularisation term.
    void clear();

    /// Add a single measurement
    /// @param m The value of the measurement
    /// @param J The Jacobian for the measurement \f$\frac{\partial\text{m}}{\partial\text{param}_i}\f$
    void addMeasurement(Type m, const Eigen::Matrix<Type, _size, 1> & J);

    /// Add a single measurement
    /// @param m The value of the measurement
    /// @param J The Jacobian for the measurement \f$\frac{\partial\text{m}}{\partial\text{param}_i}\f$
    /// @param weight The inverse variance of the measurement
    void addMeasurement(Type m, const Eigen::Matrix<Type, _size, 1> & J, Type weight);

    /// Process all the measurements and compute the weighted least squares set of parameter values
    Eigen::Matrix<Type, _size, 1> compute();

    /// For Levenberg-Marquardt algorithm
    void addPrior(Type lambda);

private:
    Eigen::Matrix<Type, _size, _size> m_AtA;
    Eigen::Matrix<Type, _size, 1> m_B;
};

} // namespace sonar

#include "impl/WLS_impl.hpp"
#endif // SONAR_WLS_H
