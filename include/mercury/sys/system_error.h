#pragma once 

#include <string>

namespace mercury {
namespace system {

    class SystemError {
    private:
        int errno_;
        std::string message_;
        
    public:
        SystemError() : errno_(0) {}
        SystemError(int e, const char *msg) : errno_(e), message_(msg) {}

        int Code() const { return errno_; }
        const char * Message() const { return message_.c_str(); }

    public:
        static SystemError last_error();
    }; // end class SystemError

} // end namespace system
} // end namespace mercury
