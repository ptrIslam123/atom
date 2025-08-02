#include "include/platform_api/posix_api/sys_error.h"

#include <array>
#include <cstring>
#include <cerrno>

namespace atom::platform::posix {

int GetLastErrorCode() {
    return errno;
}

std::string GetLastSysErrorMessage() {
    std::array<char, 256> buffer = {0};
    // Using the XSI-compliant version of strerror_r
    if (strerror_r(GetLastErrorCode()), buffer.data(), buffer.size()) {
        // If strerror_r fails, return a generic message
        return "Unknown error (" + std::to_string(GetLastErrorCode()) + ")";
    }
    return std::string(buffer.data());
}

} //! namespace atom::platform::posix
