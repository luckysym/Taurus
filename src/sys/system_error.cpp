#include <mercury/sys/system_error.h>

#include <errno.h>
#include <string.h>

namespace mercury {
namespace system {

    SystemError SystemError::last_error() {
        int e = errno;
        return SystemError(e, strerror(e));
    }

}} // end namespace mercury::system
