#include "include/platform_api/posix_api/sys_error.h"

#include <array>
#include <cstring>
#include <cerrno>

namespace atom::platform::posix {

int GetLastErrorCode() {
    return errno;
}

std::string GetLastSysErrorMessage() {
    return std::string{strerror(GetLastErrorCode())};
}

} //! namespace atom::platform::posix
