#pragma once

namespace My::Math
{

/**
 * @brief   Simple intervall struct.
 * 
 * @tparam  value_t     The floating point type to operate on.
 * 
 * @ingroup Math
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
template <typename value_t> class Intervall
{
public:
    value_t _start;
    value_t _end;
};

} // namespace My
