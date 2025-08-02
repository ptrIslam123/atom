#ifndef ATOM_POSIX_API_SYS_ERROR_H
#define ATOM_POSIX_API_SYS_ERROR_H

#include <string>

namespace atom::platform::posix {

int GetLastErrorCode();
std::string GetLastSysErrorMessage();

} //! namespace atom::platform::posix

#endif //! ATOM_POSIX_API_SYS_ERROR_H
