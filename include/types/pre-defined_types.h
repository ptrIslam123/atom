#ifndef ATOM_PRE_DEFINED_TYPES_H
#define ATOM_PRE_DEFINED_TYPES_H

#include "include/types/number.h"
#include "include/types/floating-number.h"

namespace atom::types {

using I8 = Number<std::int8_t>;
using I16 = Number<std::int16_t>;
using I32 = Number<std::int32_t>;
using I64 = Number<std::int64_t>;

using U8 = Number<std::uint8_t>;
using U16 = Number<std::uint16_t>;
using U32 = Number<std::uint32_t>;
using U64 = Number<std::uint64_t>;

using Digit = Number<std::uint8_t, 0, 9>;
using NaturalNumber = Number<std::size_t, 1>;
using Index = Number<std::size_t>;
using Size = Index;

using F32 = FloatingNumber<float, 1e-6f>;
using F64 = FloatingNumber<double, 1e-9>;

} //! namespace atom::types

#endif //! ATOM_PRE_DEFINED_TYPES_H
