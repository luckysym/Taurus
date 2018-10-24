#pragma once 
#include <taurus/net/network.h>
#include <memory>

namespace taurus {
namespace network {

    /**
     * @brief Socket基础类
     */
    class SocketImpl {
    protected:
        int           m_fd;
        InetSocketAddressPtr  m_ptrLocalAddr;
        InetSocketAddressPtr  m_ptrRemoteAddr;
        
    protected:
        SocketImpl();
        SocketImpl(const SocketImpl &other) = delete; 
        SocketImpl(SocketImpl && other);
        virtual ~SocketImpl();

        SocketImpl & operator=(SocketImpl &&other) ;
        SocketImpl & operator=(const SocketImpl& other) = delete;

    public:
        int  Fd() const;
        bool Bind(const InetAddress &address, int port);
        bool Close();
        bool Connect(const InetAddress &address, int port);
        bool Connect(const InetAddress &address, int port, int timeout);
        bool Create(bool stream);
        const InetAddress * GetLocalAddress() const;
        int  GetLocalPort() const;
        bool Listen(int backlog);
        bool ShutdownInput();
        bool ShutdownOutput();
        std::string ToString() const;
        std::string GetErrorInfo() const;
    }; // end class Socket

}} // end namespace taurus::network