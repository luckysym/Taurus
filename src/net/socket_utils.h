#pragma once 

#include <taurus/net/network.h>

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
     * @brief 将Socket错误信息输出到传入的输出流。
     * @param oss       输出流，Socket错误消息将被输出此流对象中。
     */
    inline std::ostringstream & sockerr(std::ostringstream &oss) {
        int e = errno;
        oss<<" errno: "<<e<<", errmsg: "<<strerror(e)<<". ";
        return oss;
    }

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

    inline InetAddress * NewInetAddress(int domain, const char * host, ErrorInfo & e) {
        if ( domain == Protocol::DomainInet4 ) { 
            return new Inet4Address(host, e);
        } else if ( domain == Protocol::DomainInet6 ) {
            return new Inet6Address(host, e);
        } else {
            std::ostringstream oss;
            oss<<"Unknown inet address: "<<domain<<", "<<host;
            e.set(-1, oss.str().c_str(), "NewInetAddress");
            return nullptr;
        }
        
    }

} // end namespace net
} // end namespace taurus
