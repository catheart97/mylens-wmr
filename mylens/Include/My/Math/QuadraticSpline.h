#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "Math/Intervall.h"
#include "Math/Spline.h"
#include "Utility/Utility.h"

namespace My::Math
{

/**
 * @brief   Class representing a Quadratic Spline with equally distributed knots in x-direction
 *          (y is free for specification)
 *
 * @tparam  value_t     The floating point value to operate on.
 *
 * @ingroup Math
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
template <typename value_t> class QuadraticSpline : public Spline<value_t>
{
    // Data
protected:
    bool _uniform{true};

    // CONSTRUCTORS
public:
    /**
     * @brief   Create new instance.
     *
     * @param   num_knots   Number of knots.
     * @param   intervall   Intervall in which the spline curve lives.
     */
    QuadraticSpline(size_t num_knots, Intervall<value_t> intervall)
        : Spline<value_t>(num_knots, std::move(intervall))
    {
        this->_polynom =
            std::vector<value_t>((num_knots - 1) * 3); // this is required due to inheritance
    }

    /**
     * @brief   Create new instance.
     *
     * @param   knot_x   x-Knot Values
     * @param   knot_y   y-Knot Values
     */
    QuadraticSpline(std::initializer_list<value_t> knot_x, std::initializer_list<value_t> knot_y)
        : Spline<value_t>(knot_x.size(), Intervall<value_t>{*knot_x.begin(), *knot_x.end()}),
          _uniform{false}
    {
        this->_polynom =
            std::vector<value_t>((this->numKnots() - 1) * 3); // this is required due to inheritance
        std::copy(knot_x.begin(), knot_x.end(), this->_knot_x.begin());
        std::copy(knot_y.begin(), knot_y.end(), this->_knot_y.begin());
    }

    /**
     * @brief   Create new instance.
     *
     * @param   knot_x   x-Knot Values
     * @param   knot_y   y-Knot Values
     */
    QuadraticSpline(std::vector<value_t> knot_x, std::vector<value_t> knot_y)
        : Spline<value_t>(knot_x.size(), Intervall<value_t>{*knot_x.begin(), *knot_x.end()}),
          _uniform{false}
    {
        this->_polynom =
            std::vector<value_t>((this->numKnots() - 1) * 3); // this is required due to inheritance
        std::copy(knot_x.begin(), knot_x.end(), this->_knot_x.begin());
        std::copy(knot_y.begin(), knot_y.end(), this->_knot_y.begin());
    }

    /**
     * @brief   Create new instance.
     *
     * @param   knot_x   x-Knot Values
     * @param   knot_y   y-Knot Values
     * @param   polynom  Polynom coefficients: a_1, b_1, c_1, a_2, ...
     */
    QuadraticSpline(std::initializer_list<value_t> knot_x, std::initializer_list<value_t> knot_y,
                    std::initializer_list<value_t> polynom)
        : Spline<value_t>(polynom.size() / 3, Intervall<value_t>{0, 0}), _uniform{false}
    {
        this->_polynom = std::vector<value_t>(polynom.size());
        std::copy(knot_x.begin(), knot_x.end(), this->_knot_x.begin());
        std::copy(knot_y.begin(), knot_y.end(), this->_knot_y.begin());
        std::copy(polynom.begin(), polynom.end(), this->_polynom.begin());
    }

    /**
     * @brief   Create new instance.
     *
     * @param   knot_x   x-Knot Values
     * @param   knot_y   y-Knot Values
     * @param   polynom  Polynom coefficients: a_1, b_1, c_1, a_2, ...
     */
    QuadraticSpline(std::vector<value_t> knot_x, std::vector<value_t> knot_y,
                    std::vector<value_t> polynom)
        : Spline<value_t>(polynom.size() / 3, Intervall<value_t>{0, 0}), _uniform{false}
    {
        this->_polynom = std::vector<value_t>(polynom.size());
        std::copy(knot_x.begin(), knot_x.end(), this->_knot_x.begin());
        std::copy(knot_y.begin(), knot_y.end(), this->_knot_y.begin());
        std::copy(polynom.begin(), polynom.end(), this->_polynom.begin());
    }

    QuadraticSpline(const QuadraticSpline<value_t> & o) : Spline<value_t>(o) {}

    // METHODS
private:
    /**
     * @brief   Evaluates i-th-spline-polynom within the spline.
     *          Execute QuadraticSpline::generate before usage.
     *
     * @param   i   Which polynom.
     * @param   x   Function argument.
     */
    value_t polynomial(size_t i, value_t x)
    {
        if ((i * 3) < this->_polynom.size())
        {
            return this->_polynom[i * 3] * x * x + //
                   this->_polynom[i * 3 + 1] * x + //
                   this->_polynom[i * 3 + 2];      //
        }
        else
            return this->_knot_y[this->_knot_y.size() - 1];
    }

    value_t polynomial_derivative(size_t i, value_t x)
    {
        if ((i * 3) < this->_polynom.size())
        {
            return this->_polynom[i * 3] * x + //
                   this->_polynom[i * 3 + 1];
        }
        else
        {
            return this->_polynom[(this->_knot_x.size() - 1) * 3] * x + //
                   this->_polynom[(this->_knot_x.size() - 1) * 3 + 1];
        }
    }

public:
    /**
     * @brief   Specify knot y-value
     *
     * @param   knot    ID which knot is being changed.
     * @param   value   The new y-value for the knot.
     */
    void specify(size_t knot, value_t value) override { this->_knot_y[knot] = value; }

    void specifyX(size_t knot, value_t value) override
    {
        _uniform = false;
        this->_knot_x[knot] = value;
        if (knot == 0) this->_intervall._start = value;
        if (knot == this->_knot_x.size() - 1) this->_intervall._end = value;
    }

    void generate() override
    {
        std::vector<value_t> d(this->_knot_x.size()); // size N
        std::vector<value_t> w(this->_knot_x.size()); // size N

        for (size_t i = 1; i < d.size(); ++i) // d[0] = 0
        {
            d[i] = 2 * (this->_knot_y[i] - this->_knot_y[i - 1]) /
                   (this->_knot_x[i] - this->_knot_x[i - 1]);
        }

        // solve linear equation system
        for (size_t i = 1; i < d.size(); i++) // z[0] = d[0] = 0
        {
            w[i] = d[i];
            w[i] -= d[i - 1];
            w[i] += (i > 1) ? w[i - 2] : 0;
        }

        // compute coefficients
        for (size_t i = 1; i < this->_knot_x.size(); i++)
        {
            auto id = (i - 1) * 3;
            auto &x{this->_knot_x}, &y{this->_knot_y}, &p{this->_polynom};

            // quadratic coefficient
            p[id] = 0.5 * (w[i] - w[i - 1]) / (x[i] - x[i - 1]);
            // linear coefficient
            p[id + 1] = w[i - 1] - 2 * p[id] * x[i - 1];
            // constant coefficient
            p[id + 2] = p[id] * x[i - 1] * x[i - 1] - w[i - 1] * x[i - 1] + y[i - 1];
        }
    }

    value_t compute(value_t value) override
    {
        if (_uniform)
        {
            size_t i{size_t((value - this->_intervall._start) / this->_delta)};
            return polynomial(i, value);
        }
        else
        {
            size_t id = 0;
            while (!(this->_knot_x[id] <= value && value < this->_knot_x[id + 1]) &&
                   id < this->_knot_x.size())
            {
                id++;
            }

            return polynomial(id, value);
        }
    }

    value_t derivative(value_t value)
    {
        if (_uniform)
        {
            size_t i{size_t((value - this->_intervall._start) / this->_delta)};
            return polynomial_derivative(i, value);
        }
        else
        {
            size_t id = 0;
            while (!(this->_knot_x[id] <= value && value < this->_knot_x[id + 1]) &&
                   id < this->_knot_x.size())
            {
                id++;
            }

            return polynomial_derivative(id, value);
        }
    }

    std::shared_ptr<Spline<value_t>> copy() override
    {
        return std::make_shared<QuadraticSpline<value_t>>(*this);
    }
};

} // namespace My
