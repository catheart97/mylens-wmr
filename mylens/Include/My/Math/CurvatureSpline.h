#pragma once

#include <algorithm>
#include <numeric>
#include <vector>

#include "Math/Intervall.h"
#include "Math/QuadraticSpline.h"

namespace My::Math
{

/**
 * @brief   Class describes a Curvature Spline which is a QuadraticSpline.
 * 
 * <b>Note</b> It does not have any kind of knots. So you cannot use the regular
 * SplineSimulationArgument class for the SimplexAlgorithm.
 * 
 * @tparam  value_t     The floating point type to operate on.
 * 
 * @ingroup Math
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
template <typename value_t> class CurvatureSpline : public QuadraticSpline<value_t>
{
    // Data
private:
    std::vector<value_t> _a;
    value_t _y_0, _y_n;

    // Constructors
public:
    CurvatureSpline(size_t num_a, //
                    value_t y_0,  //
                    value_t y_n,  //
                    Intervall<value_t> intervall)
        : QuadraticSpline<value_t>(num_a + 1, intervall), _a(num_a), _y_0{y_0}, _y_n{y_n}
    {}

    CurvatureSpline(const CurvatureSpline<value_t> & o) : QuadraticSpline<value_t>(o)
    {
        _a = std::vector<value_t>(o._a);
        _y_0 = o._y_0;
        _y_n = o._y_n;
    }

    // Properties
public:
    size_t numCurvatures() { return _a.size(); }

    void curvature(size_t a, value_t v) { _a[a] = v; }

    value_t curvature(size_t a) const { return _a[a]; }

    // Methods
private:
    value_t calpha()
    {
        value_t A = 0;
        for (size_t i = 0; i < _a.size() - 1; ++i) A += _a[i];

        value_t B = 0;
        for (size_t i = 0; i < _a.size() - 1; ++i) B += _a[i] * (_a.size() - 2 - i);

        return (_y_n - _y_0) /
               ((this->_delta * this->_delta) * (_a[_a.size() - 1] + 3 * A + 2 * B));
    }

private:
    value_t polynomial(size_t i, value_t x)
    {
        if ((i * 3) < this->_polynom.size())
        {
            return this->_polynom[i * 3] * x * x + //
                   this->_polynom[i * 3 + 1] * x + //
                   this->_polynom[i * 3 + 2];      //
        }
        else
            return _y_n;
    }

public:
    void specify(size_t, value_t) override {}

    void specifyX(size_t, value_t) override {} // Don't allow un-uniform

    void generate() override
    {
        value_t alpha{calpha()}, delta{this->_delta};
        auto & polynom = this->_polynom;

        // Initialize Recursion
        polynom[0] = alpha * _a[0];
        polynom[1] = 0;    // initial "tangent"
        polynom[2] = _y_0; // initial "y-axis intersection"

        // Compute regular coefficients
        for (size_t i = 1; i < _a.size(); ++i)
        {
            // a
            polynom[3 * i] = alpha * _a[i];
            // b
            polynom[3 * i + 1] = 2 * alpha * _a[i - 1] * delta + polynom[3 * (i - 1) + 1];
            // c
            polynom[3 * i + 2] = alpha * _a[i - 1] * delta * delta +
                                 polynom[3 * (i - 1) + 1] * delta + polynom[3 * (i - 1) + 2];
        }

        // Move into their corresponding area
        for (size_t i = 1; i < _a.size(); ++i)
        {
            value_t p = (delta * i) - this->intervall()._start;
            // a is correct
            // c
            polynom[3 * i + 2] += polynom[3 * i] * p * p - polynom[3 * i + 1] * p;
            // b
            polynom[3 * i + 1] -= 2 * polynom[3 * i] * p;
        }
    }

    value_t compute(value_t value) override
    {
        if (this->_uniform)
        {
            size_t i{size_t((value - this->_intervall._start) / this->_delta)};
            return polynomial(i, value);
        }
        else
        {
            throw std::runtime_error("Curvature Spline is not uniform.");
        }
    }

    std::shared_ptr<Spline<value_t>> copy() override
    {
        return std::make_shared<CurvatureSpline<value_t>>(*this);
    }
};

} // namespace My
