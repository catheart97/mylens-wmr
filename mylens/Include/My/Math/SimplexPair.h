#pragma once

#include <regex>
#include <sstream>
#include <string>

#include "Math/SimplexFunctionArgument.h"

namespace My::Math
{

/**
 * @brief   Simple Pair/tuple class with < comparison of the second value
 *
 * @tparam  value_t     The function return value.
 *
 * @ingroup Math
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
template <typename value_t> class SimplexPair
{
    // DATA
public:
    std::shared_ptr<SimplexFunctionArgument<value_t>> _first;
    value_t _second;

    // Constructors
public:
    /**
     * @brief Construct new pair.
     *
     * @param   first   The first value.
     * @param   second  The second value.
     */
    SimplexPair(std::shared_ptr<SimplexFunctionArgument<value_t>> first, value_t second)
        : _first{first}, _second{second}
    {}

#ifdef PYTHON
    SimplexPair() {}
#endif

    // Operators
public:
    /**
     * @param p other SimplexPair
     * @return true if this._second < p._second
     */
    bool operator<(const SimplexPair<value_t> & p) { return _second < p._second; }

    /**
     * @param p other SimplexPair
     * @return this._second - p._second
     */
    value_t operator-(const SimplexPair<value_t> & p) { return _second - p._second; }
};

/**
 * @brief   Operator which enables printing argument to std::ostream.
 *          it too.
 *
 * @tparam  value_t     The function return type.
 *
 * @param   os          The output stream.
 * @param   p           The SimplexPair.
 *
 * @return os
 */
template <typename value_t>
std::ostream & operator<<(std::ostream & os, const SimplexPair<value_t> & p)
{
    std::stringstream ss;
    std::string s;

    os << "<SimplexPair<>\n";
    os << "    argument:\n";

    ss << p._first;
    s = ss.str();
    s = std::regex_replace(s, std::regex("\n"), "\n\t");
    s.resize(s.size() - 1);
    os << "\t" << s;

    os << "    value: <" << typeid(p._second).name() << " " << p._second << ">\n";
    os << ">\n";
    return os;
}

} // namespace My