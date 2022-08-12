#pragma once

#include "SimplexFunctionArgument.h"

namespace My::Math
{

/**
 * @brief   Interface to use with the @ref SimplexSolver.
 *
 * @tparam  value_t     The floating point type to operate on.
 *
 * @ingroup Math
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
template <typename value_t> class SimplexFunction
{
    // METHODS:
public:
    /**
     * @brief   Computes the function on struct_t as function argument
     *
     * @param   t the function argument
     *
     * @return the result value
     */
    virtual value_t compute(const std::shared_ptr<SimplexFunctionArgument<value_t>> & t) = 0;

    /**
     * @brief Same as compute(). Do not override this method.
     *
     * @param   t   The function argument.
     *
     * @return the result value
     */
    virtual value_t operator()(const std::shared_ptr<SimplexFunctionArgument<value_t>> & t)
    {
        return compute(t);
    }

    /**
     * @brief   Optional interface function which should precompute some values within struct_t
     *          before performing final compute. (E.g solving equation which are required to
     *          compute.) Which are costly but not required after each addition.
     *
     * @param   t   The argument.
     */
    virtual void preCompute(std::shared_ptr<SimplexFunctionArgument<value_t>> & t) {}
};

} // namespace My