#pragma once 

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#include "socket_utils.h"

namespace taurus  {
namespace net {

/**
 * @brief Socket Option访问基类。
 */
class SocketOptImpl {
    int m_fd;
    int m_level;
    int m_name;

public:
    SocketOptImpl(int fd, int level, int name) : m_fd(fd), m_level(level), m_name(name) {}
    bool Set(const void *pvalue, socklen_t len, std::string &errinfo);
    bool Get(void * pvalue, socklen_t * len,  std::string & errinfo);
}; // end class SocketOptImpl

inline bool SocketOptImpl::Set(const void *pvalue, socklen_t len, std::string &errinfo)
{
    int r = ::setsockopt(m_fd, m_level, m_name, pvalue, len );
    if ( r == -1 ) {
        std::ostringstream oss;
        oss<<"setsockopt failed, fd: "<<m_fd<<", level: "<<m_level<<", optname: "<<m_name;
        oss<<", valueptr: "<<pvalue<<", value_len: "<<len<<". ";
        errinfo = MakeSocketErrorInfo(oss);
        return false;
    }
    return true;
}

inline bool SocketOptImpl::Get(void * pvalue, socklen_t * len, std::string &errinfo)
{
    int r = ::getsockopt(m_fd, m_level, m_name, pvalue, len);
    if ( r == -1 ) {
        std::ostringstream oss;
        oss<<"getsockopt failed, fd: "<<m_fd<<", level: "<<m_level<<", optname: "<<m_name<<". ";
        errinfo = MakeSocketErrorInfo(oss);
        return false;
    }
    return true;
}

class SocketOptReuseAddr : protected SocketOptImpl {
public:
    SocketOptReuseAddr(int fd) : SocketOptImpl(fd, SOL_SOCKET, SO_REUSEADDR) {}
    
    bool Get(bool * pEnabled, std::string &errinfo) {
        int value; 
        socklen_t len  = sizeof(value);
        bool isok = SocketOptImpl::Get(&value, &len, errinfo);
        if ( isok ) *pEnabled = value != 0;
        return isok;
    }

    bool Set(bool bEnabled, std::string &errinfo) {
        int value = bEnabled?1:0;
        return SocketOptImpl::Set(&value, sizeof(value), errinfo);
    }
}; // end class SocketOptReuseAddr


} // end namespace net
} // end namespace taurus