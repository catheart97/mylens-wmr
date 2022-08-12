#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef _VERBOSE
#define _DEBUG
#define VERBOSE_OUT(VARIABLE) std::cout << "> " << #VARIABLE << ": " << (VARIABLE) << "\n"
#define VERBOSE_OUT_GPU(VARIABLE, COND)                                                            \
    if (COND) printf("> " #VARIABLE ": %f\n", VARIABLE)
#else
#define VERBOSE_OUT(VARIABLE) //
#define VERBOSE_OUT_GPU(VARIABLE, COND)
#endif

#ifdef _DEBUG
#define DEBUG(EXP) EXP
#define DEBUG_OUT(VARIABLE) std::cout << "> " << #VARIABLE << ": " << (VARIABLE) << "\n"
#define DEBUG_OUT_GPU(VARIABLE, COND)                                                              \
    if (COND) printf("> " #VARIABLE ": %f\n", VARIABLE)
#else
#define DEBUG(EXP)
#define DEBUG_OUT(VARIABLE) //
#define DEBUG_OUT_GPU(VARIABLE, COND)
#endif

namespace My
{

/**
 * @brief   Enables std::ostream for std::vector
 *
 * @tparam  value_t     The type of vectors content.
 *
 * @param   os          The output stream.
 * @param   v           The vector.
 *
 * @return  os
 */
template <typename value_t>
std::ostream & operator<<(std::ostream & os, const std::vector<value_t> & v)
{
    os << "<std::vector\t";
    for (size_t i = 0; i < v.size(); ++i) os << (v[i]) << ((i != v.size() - 1) ? ", " : "");
    return os << ">\n";
}

/**
 * @brief    Module containing various headers with utility functions depending on the platform.
 *
 * @defgroup Utility
 * @author   Ronja Schnur (rschnur@students.uni-mainz.de)
 */
namespace Utility
{

}

} // namespace My::Utility