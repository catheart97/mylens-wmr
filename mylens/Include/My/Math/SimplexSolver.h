#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

#include "Math/SimplexFunction.h"
#include "Math/SimplexFunctionArgument.h"
#include "Math/SimplexPair.h"
#include "Utility/Utility.h"

namespace My::Math
{

/**
 * @brief   Class using the NelderMeadSimplex algorithm to solve a specific @ref SimplexFunction.
 *
 * @tparam  value_t     The floating point type to operate on.
 *
 * @ingroup Math
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
template <typename value_t> class SimplexSolver
{
    // DATA
private:
    std::shared_ptr<SimplexFunction<value_t>> _function;
    std::vector<SimplexPair<value_t>> _simplex;

    std::shared_ptr<SimplexFunctionArgument<value_t>> _init_state;
    value_t _lambda, _tolerance;

    // CONSTRUCTOR
public:
    /**
     * @brief   Construct a @ref SimplexSolver
     *
     * @param   function    Pointer to SimplexFunction instance that is to be optimized.
     * @param   init_state  Struct giving the initialization configuration to fit.
     * @param   lambda      The constant offset for initializing the simplex.
     * @param   tolerance   The tolerance value when to stop the optimization.
     */
    SimplexSolver(std::shared_ptr<SimplexFunction<value_t>> function,
                  std::shared_ptr<SimplexFunctionArgument<value_t>> init_state, value_t lambda,
                  value_t tolerance)
        : _function{function}, _init_state{init_state}, _lambda{lambda}, _tolerance{tolerance}
    {}

    // METHODS
private:
    std::shared_ptr<SimplexFunctionArgument<value_t>>
    massCenterStruct() // computes mass center of simplex
    {
        return std::accumulate(_simplex.begin() + 1, //
                               _simplex.end() - 1,   //
                               _simplex[0]._first,   //
                               [](std::shared_ptr<SimplexFunctionArgument<value_t>> a,
                                  SimplexPair<value_t> b) { //
                                   VERBOSE_OUT(a);
                                   VERBOSE_OUT(b._first);
                                   return a->add(b._first); //
                               })
            ->div(value_t((_simplex.size() - 1)));
    }

    SimplexPair<value_t> simplexPair(std::shared_ptr<SimplexFunctionArgument<value_t>> t)
    {
        _function->preCompute(t);
        return SimplexPair<value_t>(t, _function->compute(t));
    }

    SimplexPair<value_t> simplexPair(SimplexPair<value_t> & o)
    {
        return SimplexPair<value_t>(o._first->copy(), o._second);
    }

    bool sortSimplex()
    {
        std::sort(_simplex.begin(), _simplex.end());
        return true;
    }

    void initializeSimplex()
    {
        auto pair = simplexPair(_init_state->copy()); // generate init state
        for (size_t i = 0; i < _init_state->N() + 1; ++i)
        {
            auto p = simplexPair(pair); // copy init state
            if (i > 0)                  // 0 = init_state
            {
                p._first->set(i - 1, p._first->get(i - 1) + _lambda);
                p = simplexPair(p._first); // compute new state
            }
            _simplex.push_back(p); // push back state
        }
    }

public:
    /**
     * @brief   Searches for a local optimum.
     *
     * @param   print   Whether to print output process (default: true)
     *
     * @return  The found optimum.
     */
    SimplexPair<value_t> solve(bool print = true, int * num_iter = nullptr)
    {
        if (print) std::cout << "> Initializing Simplex ... " << std::flush;
        initializeSimplex();

        VERBOSE_OUT(_simplex);

        size_t n = _simplex.size();
        if (print) std::cout << "Done.\n> Run Simplex-Optimization ..." << std::flush;

        int k = 0;
        while (sortSimplex() && (std::abs(_simplex[0] - _simplex[1]) > _tolerance)
#ifdef _DEBUG
               && k < 200
#endif
        )
        //      first is always true, but to check second one all simplex must be sorted
        //      it would be possible to sort before each continue and after each loop too
        {
            VERBOSE_OUT(_simplex[0]);
            VERBOSE_OUT(_simplex[1]);

            if (print) // Loading animation
                std::cout << "\b\b\b" << (((k + 2) % 6 < 3) ? "." : " ")
                          << ((((k + 1) % 6) < 3) ? "." : " ") << ((k % 6 < 3) ? "." : " ")
                          << std::flush;
            k++;

            // 1st step: getting values
            auto & x_low = _simplex[0];
            auto & x_next_high = _simplex[n - 2];
            auto & x_high = _simplex[n - 1];

            // 2nd step: get mass center
            auto x_0 = massCenterStruct();

            VERBOSE_OUT(x_0);

            // 3rd step: Reflection
            auto x_r = simplexPair(x_0 + x_0 - x_high._first);
            if (x_low < x_r && x_r < x_next_high)
            {
                x_high = x_r;
                continue;
            } // to step 1

            if (x_r < x_low)
            {
                // 4th step: Expansion
                auto x_e = simplexPair(x_0 + (x_0 - x_high._first) * value_t(2.0));
                x_high = x_e < x_r ? x_e : x_r;
                continue;
            }

            // 5th step: Contraction // x_r > x_next_high
            auto x_c = simplexPair(x_0 + (x_high._first - x_0) * value_t(0.5));
            if (x_c < x_high)
            {
                x_high = x_c;
                continue;
            } // to step 1

            // 6th step: Shrink
            for (size_t i = 1; i < _simplex.size(); ++i)
                _simplex[i] =
                    simplexPair(x_low._first + (_simplex[i]._first - x_low._first) * value_t(0.5));
        }

#ifndef _DEBUG
        if (print)
#endif
            std::cout << "\b\b\b... Done with " << k << " iterations.\n";

        if (num_iter) *num_iter = k;

        return _simplex[0];
    }
};

} // namespace My