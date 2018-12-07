#pragma once 

#include <mercury/net/network.h>

#include <sstream>
#include <string>
#include <errno.h>
#include <string.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET   -1
#endif // INVALID_SOCKET

namespace mercury{
namespace net {

    /**
     * @brief 将Socket错误信息输出到传入的输出流。
     * @param oss       输出流，Socket错误消息将被输出此流对象中。
     */
    inline std::ostringstream & sockerr(std::ostringstream &oss) {
        int e = errno;
        oss<<" socket errno: "<<e<<", errmsg: "<<strerror(e)<<". ";
        return oss;
    }
    inline std::ostringstream & syserr(std::ostringstream &oss) {
        int e = errno;
        oss<<" sys errno: "<<e<<", errmsg: "<<strerror(e)<<". ";
        return oss;
    }

    /**
     * @brief 生成Socket系统错误消息。
     * @param str  错误消息的前缀。
     */ 
    inline std::string MakeSocketRuntimeError(const char * str) {
        std::ostringstream oss;
        int e = errno;
        oss<<str<<" errno: "<<e<<", "<<strerror(e);
        return oss.str();
    }
    inline void MakeSocketRuntimeError(std::string &errinfo, const char * str) {
        std::ostringstream oss;
        int e = errno;
        oss<<str<<" errno: "<<e<<", "<<strerror(e);
        errinfo = oss.str();
    }
    inline std::string MakeSocketRuntimeError(std::ostringstream &oss) {
        int e = errno;
        oss<<" errno: "<<e<<", "<<strerror(e);
        return oss.str();
    }
    inline void MakeSocketRuntimeError(std::string &errinfo, std::ostringstream &oss) {
        int e = errno;
        oss<<" errno: "<<e<<", "<<strerror(e);
        errinfo = oss.str();
    }

    inline InetAddress * NewInetAddress(int domain, const char * host, RuntimeError & e) {
        if ( domain == AF_INET ) { 
            return new Inet4Address(host, e);
        } else if ( domain == AF_INET6 ) {
            return new Inet6Address(host, e);
        } else {
            std::ostringstream oss;
            oss<<"Unknown inet address: "<<domain<<", "<<host;
            e.set(-1, oss.str().c_str(), "NewInetAddress");
            return nullptr;
        }
    }

    inline std::string sock_proto_str(int af, int type, int proto = 0) {
        if ( af == AF_INET ) {
            if ( type == SOCK_STREAM ) return std::string("tcp");
            else if ( type == SOCK_DGRAM) return std::string("udp");
            else return std::string("inet");
        } else if (af == AF_INET6 ) {
            if ( type == SOCK_STREAM ) return std::string("tcp6");
            else if ( type == SOCK_DGRAM) return std::string("udp6");
            else return std::string("inet6");
        } else {
            return std::string("unknown");
        }
    }

} // end namespace net
} // end namespace mercury
