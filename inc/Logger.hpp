#ifndef LOGGER_HPP // include guard
#define LOGGER_HPP
#include <iostream>

// definition of macro to activate logger
// #define ENABLE_LOGGER
// TODO :Replace with variadic template
// Enable set at compile time in Cmake
#ifdef ENABLE_LOGGER
#define DEBUG(message) std::cout << "Log [" << level << "]" \
                                 << " (" << __FUNCTION__ << ") " << message << std::endl
#else
#define DEBUG(level, message) // ne fait rien
#endif
#endif