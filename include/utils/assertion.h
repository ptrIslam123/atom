#ifndef ASSERTION_H
#define ASSERTION_H

#include <string_view>
#include <sstream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstdlib>

#define ASSERTION(expr, except_type, msg)                              \
    do {                                                               \
        if (!static_cast<bool>(expr)) {                                \
            std::stringstream ss;                                      \
            ss << "Assertion failed: " << __FILE__ << ":" << __LINE__  \
               << ": " << msg;                                         \
            throw except_type(ss.str());                               \
        }                                                              \
    } while (0);

#define PANIC(expr)                                                    \
    do {                                                               \
        if (static_cast<bool>(expr)) {                                 \
            std::stringstream ss;                                      \
            ss << "PANIC: " << __FILE__ << ":" << __LINE__;            \
            std::cerr << ss.str() << std::endl;                        \
            std::abort();                                              \
        }                                                              \
    } while (0);

#endif // ASSERTION_H
