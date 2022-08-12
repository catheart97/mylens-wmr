#pragma once

#include <algorithm>
#include <numeric>
#include <vector>

#include "Math/Intervall.h"
#include "Math/QuadraticSpline.h"

namespace My
{

/**
 * @brief   Class describes a GradientSpline which is a QuadraticSpline.
 * 
 * @tparam  value_t     The floating point type to operate on.
 * 
 * @ingroup Math
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
template <typename value_t> class GradientSpline : public QuadraticSpline<value_t>
{
    // Data
private:
    bool _last{true}; // wheter the last is computed differently

    // Constructors
public:
    GradientSpline(size_t num_gradients,         //
                   Intervall<value_t> intervall, //
                   value_t y_0,                  //
                   value_t y_n)
        : QuadraticSpline<value_t>(num_gradients + 2, intervall)
    {
        // first and last value of knoty_data will be y_0 and y_n all other data derivatives of the
        // spline
        this->_knot_y[0] = y_0;
        this->_knot_y[this->_knot_y.size() - 1] = y_n;
    }

    GradientSpline(size_t num_gradients,         //
                   Intervall<value_t> intervall, //
                   value_t y_0)
        : QuadraticSpline<value_t>(num_gradients + 1, intervall), _last{false}
    {
        // first of knoty_data will be y_0 all other data derivatives of the spline
        this->_knot_y[0] = y_0;
    }

    GradientSpline(const GradientSpline<value_t> & o) : QuadraticSpline<value_t>(o)
    {
        _last = o._last;
    }

public:
    void generate() override
    {
        auto &x{this->_knot_x}, &polynom{this->_polynom};
        auto &y_0{this->_knot_y[0]}, &y_n{this->_knot_y[this->_knot_y.size() - 1]};
        auto delta{this->_delta};
        auto eta = [this](size_t i) { return i ? this->_knot_y[i] : 0; };

        value_t y{y_0};
        size_t border = x.size() - (_last ? 2 : 1);
        for (size_t i = 1; i < border + 1; ++i)
        {
            delta = this->_uniform ? delta : (x[i] - x[i - 1]);

            polynom[3 * (i - 1) + 0] = (eta(i) - eta(i - 1)) / (2 * delta);
            polynom[3 * (i - 1) + 1] = (x[i - 1] * (eta(i - 1) - eta(i))) / delta + eta(i - 1);
            polynom[3 * (i - 1) + 2] = (x[i - 1] * x[i - 1] * (eta(i) - eta(i - 1))) / (2 * delta) -
                                       x[i - 1] * eta(i - 1) + y;
            y = (delta * (eta(i - 1) + eta(i))) / 2 + y;
        }

        if (_last)
        {
            size_t n = x.size() - 1;
            delta = this->_uniform ? delta : (x[n] - x[n - 1]);

            polynom[3 * (n - 1) + 0] = (-y + y_n - delta * eta(n - 1)) / (delta * delta);
            polynom[3 * (n - 1) + 1] =
                (2 * x[n - 1] * y - 2 * x[n - 1] * y_n + 2 * delta * x[n - 1] * eta(n - 1)) /
                    (delta * delta) +
                eta(n - 1);
            polynom[3 * (n - 1) + 2] =
                (y_n * x[n - 1] * x[n - 1] + y * delta * delta - y * x[n - 1] * x[n - 1] -
                 x[n - 1] * eta(n - 1) * delta * delta - delta * eta(n - 1) * x[n - 1] * x[n - 1]) /
                (delta * delta);
        }
    }

    std::shared_ptr<Spline<value_t>> copy() override
    {
        return std::make_shared<GradientSpline<value_t>>(*this);
    }
};

} // namespace My
