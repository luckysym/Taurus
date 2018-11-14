#pragma once 

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/tcp.h>

#include "socket_utils.h"

namespace mercury  {
namespace net {

class SocketOptImpl;
class SocketOptReuseAddr;
class SocketOptRecvBuffer;
class SocketOptSendBuffer;
class SocketOptRecvTimeout;
class SocketOptSendTimeout;
class SocketOptKeepAlive;
class SocketOptLinger;
class SocketOptTcpNoDelay;

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
        errinfo = MakeSocketRuntimeError(oss);
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
        errinfo = MakeSocketRuntimeError(oss);
        return false;
    }
    return true;
}

/**
 * @brief SOL_SOCKET/SO_REUSEADDR选项操作类
 */ 
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

class SocketOptRecvBuffer : protected SocketOptImpl {
public:
    SocketOptRecvBuffer(int fd) : SocketOptImpl(fd, SOL_SOCKET, SO_RCVBUF) {}
    bool Get(int * value, std::string &e) {
        socklen_t len = sizeof(int);
        return SocketOptImpl::Get(value, &len, e);
    }
    bool Set(int value, std::string &e) {
        return SocketOptImpl::Set(&value, sizeof(value), e);
    }
}; // end class SocketOptRecvBuffer

class SocketOptSendBuffer : protected SocketOptImpl {
public:
    SocketOptSendBuffer(int fd) : SocketOptImpl(fd, SOL_SOCKET, SO_SNDBUF) {}
    bool Get(int * value, std::string &e) {
        socklen_t len = sizeof(int);
        return SocketOptImpl::Get(value, &len, e);
    }
    bool Set(int value, std::string &e) {
        return SocketOptImpl::Set(&value, sizeof(value), e);
    }
}; // end class SocketOptSendBuffer

class SocketOptSendTimeout : protected SocketOptImpl {
public:
    SocketOptSendTimeout(int fd) : SocketOptImpl(fd, SOL_SOCKET, SO_SNDTIMEO) {}
    bool Get(int *ms, std::string &e) {
        struct timeval t;
        socklen_t len = sizeof(t);
        bool isok = SocketOptImpl::Get(&t, &len, e);
        if ( isok ) *ms = (int)(t.tv_sec * 1000 + t.tv_usec / 1000);
        return isok;
    }
    bool Set(int ms, std::string &e) {
        struct timeval t;
        t.tv_sec = (time_t)(ms / 1000);
        t.tv_usec = ( ms % 1000 ) * 1000;
        return SocketOptImpl::Set(&t, sizeof(t), e);
    }
}; // end class SocketOptSendTimeout

class SocketOptRecvTimeout : protected SocketOptImpl {
public:
    SocketOptRecvTimeout(int fd) : SocketOptImpl(fd, SOL_SOCKET, SO_RCVTIMEO) {}
    bool Get(int *ms, std::string &e) {
        struct timeval t;
        socklen_t len = sizeof(t);
        bool isok = SocketOptImpl::Get(&t, &len, e);
        if ( isok ) *ms = (int)(t.tv_sec * 1000 + t.tv_usec / 1000);
        return isok;
    }
    bool Set(int ms, std::string &e) {
        struct timeval t;
        t.tv_sec = (time_t)(ms / 1000);
        t.tv_usec = ( ms % 1000 ) * 1000;
        return SocketOptImpl::Set(&t, sizeof(t), e);
    }
}; // end class SocketOptRecvTimeout


class SocketOptKeepAlive : protected SocketOptImpl {
public:
    SocketOptKeepAlive(int fd) : SocketOptImpl(fd, SOL_SOCKET, SO_KEEPALIVE) {}
    bool Get(bool *value, std::string &e) {
        int t;
        socklen_t len = sizeof(t);
        bool isok = SocketOptImpl::Get(&t, &len, e);
        if ( isok ) *value = (t != 0);
        return isok;
    }
    bool Set(bool value, std::string &e) {
        int t = value?1:0;
        return SocketOptImpl::Set(&t, sizeof(t), e);
    }
}; // end class SocketOptKeepAlive

class SocketOptLinger : protected SocketOptImpl {
public:
    SocketOptLinger(int fd) : SocketOptImpl(fd, SOL_SOCKET, SO_LINGER) {}
    bool Get(int *secs, std::string &e) {
        struct linger t;
        socklen_t len = sizeof(t);
        bool isok = SocketOptImpl::Get(&t, &len, e);
        if ( isok ) *secs = t.l_onoff==0?0:t.l_linger;  // 0表示关闭
        return isok;
    }
    bool Set(int secs, std::string &e) {
        struct linger t;
        t.l_onoff = secs==0?0:1;   // 0表示关闭
        t.l_linger = secs;
        return SocketOptImpl::Set(&t, sizeof(t), e);
    }
}; // end class SocketOptLinger

class SocketOptTcpNoDelay : protected SocketOptImpl {
public:
    SocketOptTcpNoDelay(int fd) : SocketOptImpl(fd, SOL_SOCKET, TCP_NODELAY) {}
    bool Get(bool *value, std::string &e) {
        int t;
        socklen_t len = sizeof(t);
        bool isok = SocketOptImpl::Get(&t, &len, e);
        if ( isok ) *value = (t != 0);
        return isok;
    }
    bool Set(bool value, std::string &e) {
        int t = value?1:0;
        return SocketOptImpl::Set(&t, sizeof(t), e);
    }
}; // end class SocketOptTcpNoDelay



} // end namespace net
} // end namespace mercury