#pragma once 

#include <sstream>
#include <string>
#include <errno.h>
#include <string.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET   -1
#endif // INVALID_SOCKET

namespace taurus{
namespace net {

    /**
     * @brief 生成Socket系统错误消息。
     * @param str  错误消息的前缀。
     */ 
    inline std::string MakeSocketErrorInfo(const char * str) {
        std::ostringstream oss;
        int e = errno;
        oss<<str<<" errno: "<<e<<", "<<strerror(e);
        return oss.str();
    }
    inline void MakeSocketErrorInfo(std::string &errinfo, const char * str) {
        std::ostringstream oss;
        int e = errno;
        oss<<str<<" errno: "<<e<<", "<<strerror(e);
        errinfo = oss.str();
    }
    inline std::string MakeSocketErrorInfo(std::ostringstream &oss) {
        int e = errno;
        oss<<" errno: "<<e<<", "<<strerror(e);
        return oss.str();
    }
    inline void MakeSocketErrorInfo(std::string &errinfo, std::ostringstream &oss) {
        int e = errno;
        oss<<" errno: "<<e<<", "<<strerror(e);
        errinfo = oss.str();
    }

} // end namespace net
} // end namespace taurus
