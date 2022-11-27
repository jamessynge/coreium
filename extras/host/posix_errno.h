#ifndef MCUCORE_EXTRAS_HOST_POSIX_ERRNO_H_
#define MCUCORE_EXTRAS_HOST_POSIX_ERRNO_H_

// TODO(jamessynge): Describe why this file exists/what it provides.

#include <string>

namespace mcucore_host {

// Returns the name of the errno value according to the platform, or nullptr if
// not known.
const char* ErrnoToPlatformName(int error_number);

// Returns a string describing an errno.
std::string ErrnoToString(int error_number);

}  // namespace mcucore_host

#endif  // MCUCORE_EXTRAS_HOST_POSIX_ERRNO_H_
