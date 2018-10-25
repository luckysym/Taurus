#pragma once 
#include <taurus/net/network.h>
#include <memory>
#include <sstream>
#include <cassert>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define INVALID_SOCKET   -1

namespace taurus {
namespace net {

    /**
     * @brief 生成Socket系统错误消息。
     * @param str  错误消息的前缀。
     */ 
    inline std::string MakeSocketErrorInfo(const char * str) {
        std::ostringstream oss;
        int e = errno;
        oss<<str<<" error: "<<e<<", "<<strerror(e);
        return oss.str();
    }

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
            SOCK_STATE_OPEN,      // 已打开，fd>=0且完成listen或者connect
            SOCK_STATE_OPENING    // 打开中，fd>=0但连接尚未完成，通常用于非阻塞连接INPROGRESS状态。
        };

        using Ptr = std::shared_ptr<SocketImpl>;
        
    protected:
        int                     m_fd;
        int                     m_state;
        InetSocketAddress::Ptr  m_ptrLocalAddr;
        InetSocketAddress::Ptr  m_ptrRemoteAddr;
        
    public:
        SocketImpl() : m_fd(INVALID_SOCKET), m_state(SOCK_STATE_CLOSED) {}
        SocketImpl(const SocketImpl &other) = delete; 
        SocketImpl(SocketImpl && other);
        virtual ~SocketImpl() { std::string err; this->Close(err); }

        SocketImpl & operator=(SocketImpl &&other) ;
        SocketImpl & operator=(const SocketImpl& other) = delete;

    public:
        int  Fd() const { return m_fd; }
        bool Bind(const InetAddress &address, int port, std::string & errinfo);
        bool Close(std::string & errinfo);
        bool Connect(const InetAddress &address, int port, std::string & errinfo);
        bool Connect(const InetAddress &address, int port, int timeout, std::string & errinfo);
        bool Create(const Protocol &proto std::string & errinfo);
        const InetAddress * GetLocalAddress() const;
        int  GetLocalPort() const;
        bool Listen(int backlog);
        bool ShutdownInput();
        bool ShutdownOutput();
        int  State() const { return m_state; }
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
                errinfo = MakeSocketErrorInfo("SocketImpl::Close");
                return false;
            }
        } 
        return true;
    }

}} // end namespace taurus::net