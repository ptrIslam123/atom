#ifndef ATOM_UTILS_TODO_H
#define ATOM_UTILS_TODO_H

#include "include/utils/assertion.h"

#include <stdexcept>

#define ATOM_TODO ASSERTION(false, std::runtime_error, "TODO: Not impl yet")

#endif //! ATOM_UTILS_TODO_H
