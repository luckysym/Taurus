#pragma once 
#include <taurus/net/network.h>
#include <memory>
#include <cassert>
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
        mutable InetSocketAddress::Ptr  m_ptrLocalAddr;
        mutable InetSocketAddress::Ptr  m_ptrRemoteAddr;
        
    public:
        SocketImpl() : m_fd(INVALID_SOCKET), m_state(SOCK_STATE_CLOSED) {}
        SocketImpl(const SocketImpl &other) = delete; 
        SocketImpl(SocketImpl && other);
        virtual ~SocketImpl() { std::string err; this->Close(err); }

        SocketImpl & operator=(SocketImpl &&other) ;
        SocketImpl & operator=(const SocketImpl& other) = delete;

    public:
        int   Fd() const { return m_fd; }
        bool  Bind(const InetAddress &address, int port, std::string & errinfo);
        bool  Close(std::string & errinfo);
        bool  Connect(const InetAddress &address, int port, std::string & errinfo);
        bool  Create(const Protocol &proto, std::string & errinfo);
        const InetSocketAddress * GetLocalAddress(std::string &errinfo) const;
        const InetSocketAddress * GetRemoteAddress(std::string &errinfo) const;
        bool  Listen(int backlog, std::string &errinfo);
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
        
        m_ptrLocalAddr = std::move(other.m_ptrLocalAddr);
        m_ptrRemoteAddr = std::move(other.m_ptrRemoteAddr);
    }

    inline SocketImpl & SocketImpl::operator=(SocketImpl &&other) {
        if ( m_fd != INVALID_SOCKET ) throw std::runtime_error("can't move socket to another valid socket");

        m_fd = other.m_fd;   
        other.m_fd = INVALID_SOCKET;
        
        m_state = other.m_state; 
        other.m_state = SOCK_STATE_CLOSED;
        
        m_ptrLocalAddr = std::move(other.m_ptrLocalAddr);
        m_ptrRemoteAddr = std::move(other.m_ptrRemoteAddr);
        return *this;
    }

    inline bool SocketImpl::Close(std::string & errinfo) {
        if ( m_fd != INVALID_SOCKET ) {
            m_state = SOCK_STATE_CLOSED;
            int r = ::close(m_fd);
            if ( r == -1 ) {
                errinfo = MakeSocketErrorInfo("close() error. ");
                return false;
            }
            m_fd = INVALID_SOCKET;
        } 
        return true;
    }

    inline bool SocketImpl::Create(const Protocol &proto, std::string & errinfo) {
        // 是否已经创建判断，避免重复创建。
        if ( m_fd != INVALID_SOCKET ) throw std::runtime_error("socket already created, cannot create again");

        // 创建socket
        int fd = ::socket(proto.Domain(), proto.Type(), proto.Proto());
        if ( fd == INVALID_SOCKET ) {
            MakeSocketErrorInfo(errinfo, "socket() error,");
            return false;
        }
        m_fd = fd;
        m_state = SOCK_STATE_CREATED;
        return true;
    }

    inline bool SocketImpl::Bind(const InetAddress & address, int port, std::string &errinfo) {
        InetSocketAddress::Ptr ptrSockAddr( new InetSocketAddress(address, port) );
        int r = ::bind(m_fd, ptrSockAddr->CAddress(), ptrSockAddr->CAddressSize());
        if ( r == -1 ) {
            std::ostringstream oss;
            oss<<"bind() error, "<<ptrSockAddr->ToString();
            MakeSocketErrorInfo(errinfo, oss);
            return false;
        }
        m_ptrLocalAddr = ptrSockAddr;
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

    inline const InetSocketAddress * SocketImpl::GetLocalAddress(std::string &errinfo) const {
        if ( !m_ptrLocalAddr ) {
            char addrbuf[32];
            socklen_t addrlen = 32;
            int r = ::getsockname(m_fd, (struct sockaddr*)addrbuf, &addrlen);
            if ( r == -1 ) {
                std::ostringstream oss;
                oss<<"getsockname() error, fd: "<<m_fd;
                MakeSocketErrorInfo(errinfo, oss);
                return nullptr;
            }
            m_ptrLocalAddr = std::make_shared<InetSocketAddress>((sockaddr*)addrbuf, addrlen);
        }
        return m_ptrLocalAddr.get();
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

    inline bool SocketImpl::Listen(int backlog, std::string &errinfo) {
        if ( !m_ptrLocalAddr ) throw std::runtime_error("bind socket before listen");

        int r = ::listen(m_fd, backlog);
        if ( r == -1 ) {
            std::ostringstream oss;
            oss<<"bind() error, fd: "<<m_fd<<", addr: "<<m_ptrLocalAddr->ToString();
            MakeSocketErrorInfo(errinfo, oss);
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