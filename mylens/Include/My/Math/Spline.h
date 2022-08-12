#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "Utility/Utility.h"

#include "Math/Intervall.h"

namespace My::Math
{

/**
 * @brief   Interface specifiying a spline curve.
 *
 * @tparam  value_t     The floating point type to operate on.
 *
 * @ingroup Math
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
template <typename value_t> class Spline
{
    // Data
protected:
    std::vector<value_t> _knot_x, _knot_y; // knots (x is computed, y specified)
    std::vector<value_t> _polynom;         // coefficients for each polynom
                                           // Structured as: p0[0] p0[1] p0[2], p1[0] ...n - 1
    Intervall<value_t> _intervall;
    value_t _delta;

    // Constructors
public:
    /**
     * @brief   Constructor for inheritance
     *
     * @param   num_knots   Specifies the amount of knots used.
     * @param   intervall   The intervall in which the spline lives.
     */
    Spline(size_t num_knots, Intervall<value_t> intervall)
        : _knot_x(num_knots), _knot_y(num_knots), _intervall{intervall},
          _delta((_intervall._end - _intervall._start) / (num_knots - 1))
    {
        for (value_t i = 0; i < num_knots; ++i)
            _knot_x[i] = _intervall._start + i * _delta; // equally distributed x-values
    }

    /**
     * @brief   Copies the whole spline.
     *
     * @param   o   The other spline.
     */
    Spline(const Spline<value_t> & o)
        : _knot_x(o._knot_x), _knot_y(o._knot_y),
          _polynom(o._polynom), _intervall{o._intervall}, _delta{o._delta}
    {}

    virtual ~Spline() = default;

    // Properties
public:
    /**
     * @brief   Access the intervall.
     * @return  The operating intervall.
     */
    Intervall<value_t> intervall() const noexcept { return _intervall; }

    /**
     * @brief   Access the number of knots.
     * @return  The number of knots.
     */
    size_t numKnots() const noexcept { return _knot_x.size(); }

    /**
     * @brief   Access the knot data in x direction.
     * @return  The pointer to the data.
     */
    const value_t * knotXData() const noexcept { return _knot_x.data(); }

    /**
     * @brief   Access the knot data in y direction.
     * @return  The pointer to the data.
     */
    const value_t * knotYData() const noexcept { return _knot_y.data(); }

    /**
     * @brief   Access the computed polynom coefficients.
     * @return  The pointer to the data.
     */
    const value_t * polynomData() const noexcept { return _polynom.data(); }

    /**
     * @brief   Access x-positions of the spline's knots.
     */
    const std::vector<value_t> & X() const { return _knot_x; }

    /**
     * @brief   Access y-positions of the spline's knots.
     */
    const std::vector<value_t> & Y() const { return _knot_y; }

    /**
     * @brief   Specify knot knot to value value.
     *
     * @param   knot    The ID of the knot.
     * @param   value   The new y-value.
     */
    virtual void specify(size_t knot, value_t value) = 0;

    /**
     * @brief   Override to allow spline x-value changes.
     *
     * @param   knot    The ID of the knot.
     * @param   value   The new x-value.
     */
    virtual void specifyX(size_t knot, value_t value) {}

    // Methods
public:
    /**
     * @brief   Should compute the spline at x.
     *
     * @param   value   X value to compute.
     *
     * @return The function value.
     */
    virtual value_t compute(value_t value) = 0;

    /**
     * @brief   Same as compute. Do not override this function.
     *
     * @param   value   X value to compute.
     *
     * @return The function value.
     */
    virtual value_t operator()(value_t value) { return compute(value); }

    /**
     * @brief   This function should compute the polynom spline coefficients.
     */
    virtual void generate() = 0;

    /**
     * @brief   Must return a copy of the derived spline.
     */
    virtual std::shared_ptr<Spline<value_t>> copy() = 0;
};

/**
 * @brief   Enables pairwise addition of y-KnotData
 *
 * @param   t   First spline.
 * @param   o   Second spline.
 *
 * @return t + o
 */
template <typename value_t>
const std::shared_ptr<Spline<value_t>> operator+(std::shared_ptr<Spline<value_t>> t,
                                                 std::shared_ptr<Spline<value_t>> o)
{
    auto s = t->copy();
    for (size_t i = 0; i < s->numKnots(); ++i) s->specify(i, t->knotYData()[i] + o->knotYData()[i]);
    return s;
}

/**
 * @brief   Enables pairwise substraction of y-KnotData
 *
 * @param   t   First spline.
 * @param   o   Second spline.
 *
 * @return t - o
 */
template <typename value_t>
const std::shared_ptr<Spline<value_t>> operator-(std::shared_ptr<Spline<value_t>> t,
                                                 std::shared_ptr<Spline<value_t>> o)
{
    auto s = t->copy();
    for (size_t i = 0; i < s->numKnots(); ++i) s->specify(i, t->knotYData()[i] - o->knotYData()[i]);
    return s;
}

/**
 * @brief   Enables pairwise division of y-KnotData
 *
 * @param   t   Spline
 * @param   o   value
 *
 * @return t / o
 */
template <typename value_t>
const std::shared_ptr<Spline<value_t>> operator/(std::shared_ptr<Spline<value_t>> t, value_t o)
{
    auto s = t->copy();
    for (size_t i = 0; i < s->numKnots(); ++i) s->specify(i, t->knotYData()[i] / o);
    return s;
}

/**
 * @brief   Enables pairwise multiplication of y-KnotData
 *
 * @param   t   Spline
 * @param   o   value
 *
 * @return t * o
 */
template <typename value_t>
const std::shared_ptr<Spline<value_t>> operator*(std::shared_ptr<Spline<value_t>> t, value_t o)
{
    auto s = t->copy();
    for (size_t i = 0; i < s->numKnots(); ++i) s->specify(i, t->knotYData()[i] * o);
    return s;
}

/**
 * @brief   Operator which enables printing argument to std::ostream.
 *
 * @tparam  value_t     The floating point type to operate on.
 *
 * @param   os          The output stream.
 * @param   spline      The spline.
 *
 * @return os
 */
template <typename value_t>
std::ostream & operator<<(std::ostream & os, const std::shared_ptr<Spline<value_t>> & spline)
{
    std::stringstream ss;
    std::string s;

    os << "<Spline<>\n";

    os << "    Knot X:\t " << spline->X();
    os << "    Knot Y:\t " << spline->Y();

    os << ">\n";
    return os;
}

} // namespace My
