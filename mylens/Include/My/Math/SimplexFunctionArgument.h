#pragma once

#include <algorithm>
#include <memory>

namespace My::Math
{

/**
 * @brief   Class describing the basic interface for the Simplex Solver.
 * 
 * The class works as an interface for the simplex relaxation operations such as
 * addition and substraction. This allows to specifiy the search room of the arguments, 
 * for example to operate on a sphere.
 *
 * @ingroup Math
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
template <typename value_t> class SimplexFunctionArgument
{
    // Data
private:
    size_t _N{0};

    // Constructors
public:
    SimplexFunctionArgument(size_t N) : _N{N} {}

    // Properties
public:
    /**
     * @brief   Returns the maximum available argument width.
     */
    virtual size_t N() { return _N; }

    virtual value_t get(size_t i) = 0;

    virtual void set(size_t i, value_t v) = 0;

    // Methods
public:
    /**
     * @brief   Adds two arguments together.
     */
    virtual std::shared_ptr<SimplexFunctionArgument<value_t>>
    add(std::shared_ptr<SimplexFunctionArgument<value_t>> other) = 0;

    /**
     * @brief   Substracts two Arguments.
     */
    virtual std::shared_ptr<SimplexFunctionArgument<value_t>>
    sub(std::shared_ptr<SimplexFunctionArgument<value_t>> other) = 0;

    /**
     * @brief   Divides this argument by other.
     */
    virtual std::shared_ptr<SimplexFunctionArgument<value_t>> div(value_t other) = 0;

    /**
     * @brief   Multiplies this argument with other.
     */
    virtual std::shared_ptr<SimplexFunctionArgument<value_t>> mul(value_t other) = 0;

    /**
     * @brief   Returns a copy of this.
     */
    virtual std::shared_ptr<SimplexFunctionArgument<value_t>> copy() = 0;
};

template <typename value_t>
std::shared_ptr<SimplexFunctionArgument<value_t>>
operator+(std::shared_ptr<SimplexFunctionArgument<value_t>> t,
          std::shared_ptr<SimplexFunctionArgument<value_t>> o)
{
    return t->add(o);
}

template <typename value_t>
std::shared_ptr<SimplexFunctionArgument<value_t>>
operator-(std::shared_ptr<SimplexFunctionArgument<value_t>> t,
          std::shared_ptr<SimplexFunctionArgument<value_t>> o)
{
    return t->sub(o);
}

template <typename value_t>
std::shared_ptr<SimplexFunctionArgument<value_t>>
operator/(std::shared_ptr<SimplexFunctionArgument<value_t>> t, value_t o)
{
    return t->div(o);
}

template <typename value_t>
std::shared_ptr<SimplexFunctionArgument<value_t>>
operator*(std::shared_ptr<SimplexFunctionArgument<value_t>> t, value_t o)
{
    return t->mul(o);
}

template <typename value_t>
std::ostream & operator<<(std::ostream & os,
                          const std::shared_ptr<SimplexFunctionArgument<value_t>> & p)
{
    for (size_t i = 0; i < p->N(); ++i)
    {
        os << p->get(i) << ((i != p->N() - 1) ? ", " : "\n");
    }
    return os;
}

} // namespace My