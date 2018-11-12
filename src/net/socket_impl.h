#pragma once 
#include <taurus/net/network.h>
#include <taurus/error_info.h>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <unistd.h>

#include "socket_utils.h"

namespace taurus {
namespace net {

    /**
     * @brief Socket实现基础类
     */
    class SocketImpl {
    public:
        /**
         * @brief Socket状态枚举。
         */ 
        enum EnumSocketState {
            SOCK_STATE_CLOSED,    // 已关闭，或者未打开，fd=-1。
            SOCK_STATE_CREATED,   // 已创建，但未打开。
            SOCK_STATE_OPEN,      // 已打开，fd>=0且完成listen或者connect
            SOCK_STATE_OPENING    // 打开中，fd>=0但连接尚未完成，通常用于非阻塞连接INPROGRESS状态。
        };

        using Ptr = std::unique_ptr<SocketImpl>;
        
    protected:
        int                             m_fd;
        int                             m_state;
        int                             m_domain;       // address family, AF_INET/AF_INET4
        int                             m_socktype;     // socket type, SOCK_STREAM/SOCK_DGRAM 
        mutable InetSocketAddress::Ptr  m_ptrLocalAddr;
        mutable InetSocketAddress::Ptr  m_ptrRemoteAddr;
        
    public:
        SocketImpl() 
            : m_fd(INVALID_SOCKET), m_state(SOCK_STATE_CLOSED)
            , m_domain(0), m_socktype(0) {}

        SocketImpl(const SocketImpl &other) = delete; 
        SocketImpl(SocketImpl && other);
        virtual ~SocketImpl() { ErrorInfo err; this->Close(err); }

        SocketImpl & operator=(SocketImpl &&other) ;
        SocketImpl & operator=(const SocketImpl& other) = delete;

    public:
        int   Domain() const { return m_domain; }
        int   SocketType() const { return m_socktype; }
        int   Fd() const { return m_fd; }
        
        bool  Bind(const char *host, int port, ErrorInfo & errinfo);
        bool  Close(ErrorInfo &e);
        bool  Connect(const InetAddress &address, int port, std::string & errinfo);
        bool  Create(const Protocol &proto, ErrorInfo & errinfo);
        std::string GetLocalAddress(ErrorInfo &error) const;
        const InetSocketAddress * GetRemoteAddress(std::string &errinfo) const;
        bool  Listen(int backlog, ErrorInfo &errinfo);
        bool  ShutdownInput(std::string &errinfo);
        bool  ShutdownOutput(std::string &errinfo);
        int   State() const { return m_state; }
        std::string ToString() const;
    }; // end class Socket

    inline SocketImpl::SocketImpl(SocketImpl && other) {
        m_fd = other.m_fd;
        other.m_fd = INVALID_SOCKET;
        
        m_state = other.m_state; 
        other.m_state = SOCK_STATE_CLOSED;

        m_domain = other.m_domain;
        m_socktype = other.m_socktype;
        
        m_ptrLocalAddr = std::move(other.m_ptrLocalAddr);
        m_ptrRemoteAddr = std::move(other.m_ptrRemoteAddr);
    }

    inline SocketImpl & SocketImpl::operator=(SocketImpl &&other) {
        if ( m_fd != INVALID_SOCKET ) throw std::runtime_error("can't move socket to another valid socket");

        m_fd = other.m_fd;   
        other.m_fd = INVALID_SOCKET;
        
        m_state = other.m_state; 
        other.m_state = SOCK_STATE_CLOSED;

        m_domain = other.m_domain;
        m_socktype = other.m_socktype;
        
        m_ptrLocalAddr = std::move(other.m_ptrLocalAddr);
        m_ptrRemoteAddr = std::move(other.m_ptrRemoteAddr);
        return *this;
    }

    inline bool SocketImpl::Close(ErrorInfo & e) {
        if ( m_fd != INVALID_SOCKET ) {
            m_state = SOCK_STATE_CLOSED;
            int r = ::close(m_fd);
            if ( r == -1 ) {
                std::ostringstream oss;
                oss<<"close() socket error, fd: "<<m_fd<<sockerr; 
                e.set(-1, oss.str().c_str(), "SocketImpl::Close");
                return false;
            }
            m_fd = INVALID_SOCKET;
        } 
        return true;
    }

    inline bool SocketImpl::Create(const Protocol &proto, ErrorInfo & errinfo) {
        // 是否已经创建判断，避免重复创建。
        if ( m_fd != INVALID_SOCKET ) throw std::runtime_error("socket already created, cannot create again");

        // 创建socket
        int fd = ::socket(proto.Domain(), proto.Type(), proto.Proto());
        if ( fd == INVALID_SOCKET ) {
            std::ostringstream oss;
            oss<<"socket() error, "<<sockerr<<" protocol: "<<proto.ToString();
            errinfo.set(-1, oss.str().c_str(), "SocketImpl::Create");
            return false;
        }

        m_fd = fd;
        m_state = SOCK_STATE_CREATED;
        m_domain = proto.Domain();
        m_socktype = proto.Type();
        return true;
    }

    inline bool SocketImpl::Bind(const char *host, int port, ErrorInfo &errinfo) {
        InetAddress::Ptr ptrAddr( NewInetAddress(m_domain, host, errinfo) );
        if ( !ptrAddr) {
            std::ostringstream oss;
            oss<<"SocketImpl::Bind() failed, fd: "<<m_fd<<". ";
            errinfo.push(oss.str().c_str());
            return false;
        }

        InetSocketAddress::Ptr ptrSockAddr( new InetSocketAddress(*ptrAddr, port) );
        int r = ::bind(m_fd, ptrSockAddr->CAddress(), ptrSockAddr->CAddressSize());
        if ( r == -1 ) {
            std::ostringstream oss;
            oss<<"bind() error, fd: "<<m_fd<<". "<<"local address: "<<ptrSockAddr->ToString()<<", "<<sockerr;
            errinfo.set(-1, oss.str().c_str(), "SocketImpl::Bind");
            return false;
        }
        return true;
    }

    inline bool SocketImpl::Connect(const InetAddress &address, int port, std::string & errinfo) {
        InetSocketAddress::Ptr ptrRemoteAddr(new InetSocketAddress(address, port));
        int r = ::connect(m_fd, ptrRemoteAddr->CAddress(), ptrRemoteAddr->CAddressSize());
        if ( r == -1) {
            int e = errno;
            // 非阻塞连接，修改状态，可以根据状态值判断连接是否完成。
            if ( e == EINPROGRESS ) m_state = SOCK_STATE_OPENING; 
            std::ostringstream oss;
            oss<<"connect() error, fd: "<<m_fd<<", remote: "<<ptrRemoteAddr->ToString();
            MakeSocketErrorInfo(errinfo, oss);
            return false;
        }

        // 设置远程地址
        this->m_state = SOCK_STATE_OPEN;
        m_ptrRemoteAddr = ptrRemoteAddr;
        return true;
    }

    inline std::string SocketImpl::GetLocalAddress(ErrorInfo &error) const {
        char addrbuf[32];
        socklen_t addrlen = 32;
        int r = ::getsockname(m_fd, (struct sockaddr*)addrbuf, &addrlen);
        if ( r == -1 ) {
            std::ostringstream oss;
            oss<<"getsockname() error, fd: "<<m_fd;
            error.set(-1, MakeSocketErrorInfo(oss).c_str(), "SocketImpl::GetLocalAddress");
            return std::string();
        }
        if ( m_domain == Protocol::DomainInet4) {
            struct sockaddr_in * paddr = (struct sockaddr_in*)addrbuf;
            Inet4Address addr(paddr->sin_addr.s_addr);
            return addr.ToString();
        } else if ( m_domain == Protocol::DomainInet6) {
            struct sockaddr_in6 * paddr = (struct sockaddr_in6*)addrbuf;
            Inet6Address addr(paddr->sin6_addr.s6_addr, 16);
            return addr.ToString();
        } else  {
            std::ostringstream oss;
            oss<<"bad domain, fd: "<<m_fd<<", domain: "<<m_domain;
            error.set(-1, MakeSocketErrorInfo(oss).c_str(), "SocketImpl::GetLocalAddress");
            return std::string();
        }
    }

    inline const InetSocketAddress * SocketImpl::GetRemoteAddress(std::string &errinfo) const {
        if ( !m_ptrRemoteAddr ) {
            char addrbuf[32];
            socklen_t addrlen = 32;
            int r = ::getpeername(m_fd, (struct sockaddr*)addrbuf, &addrlen);
            if ( r == -1 ) {
                std::ostringstream oss;
                oss<<"getpeername() error, fd: "<<m_fd;
                MakeSocketErrorInfo(errinfo, oss);
                return nullptr;
            }
            m_ptrRemoteAddr = std::make_shared<InetSocketAddress>((sockaddr*)addrbuf, addrlen);
        }
        return m_ptrRemoteAddr.get();
    }

    inline bool SocketImpl::Listen(int backlog, ErrorInfo &errinfo) {
        int r = ::listen(m_fd, backlog);
        if ( r == -1 ) {
            std::ostringstream oss;
            oss<<"bind() error, fd: "<<m_fd<<", addr: "<<m_ptrLocalAddr->ToString()<<", "<<sockerr;
            errinfo.set(-1, oss.str().c_str(), "SocketImpl::Listen");
            return false;
        }
        m_state = SOCK_STATE_OPEN;  // server sock listen ok 
        return true;
    }

    inline bool SocketImpl::ShutdownInput(std::string &errinfo) {
        if ( m_fd == INVALID_SOCKET) return true;  // 已经关了
        int r = ::shutdown(m_fd, SHUT_RD);
        if ( r == -1 ) {
            std::ostringstream oss;
            oss<<"shutdown(RD) error, fd: "<<m_fd<<", ";
            MakeSocketErrorInfo(errinfo, oss);
            return false;
        }
        return true;
    }

    inline bool SocketImpl::ShutdownOutput(std::string &errinfo) {
        if ( m_fd == INVALID_SOCKET) return true;  // 已经关了
        int r = ::shutdown(m_fd, SHUT_WR);
        if ( r == -1 ) {
            std::ostringstream oss;
            oss<<"shutdown(WR) error, fd: "<<m_fd<<", ";
            MakeSocketErrorInfo(errinfo, oss);
            return false;
        }
        return true;
    }

}} // end namespace taurus::net