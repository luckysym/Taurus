#pragma once 
#include <mercury/error_info.h>

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <memory>
#include <stdexcept>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>

namespace mercury  {
namespace net {

    class Protocol;
    class SocketBase;
    class ServerSocket;
    class StreamSocket;

    /**
     * @brief Socket协议对象。
     */
    class Protocol final {
    private:
    	int m_domain;    ///< address family
	    int m_type;      ///< socket type, SOCK_STREAM, SOCK_DGRAM
	    int m_proto;     ///< socket protocol

    public:

        Protocol() : m_domain(-1), m_type(-1), m_proto(-1) {}
        /**
         * @brief Protocol 协议类构造函数
         * @param domain address family
         * @param type socket type, SOCK_STREAM, SOCK_DGRAM
         * @param proto socket protocol, 默认0
         */
        Protocol(int domain, int type, int value = 0)
            : m_domain(domain), m_type(type), m_proto(value) {}
            
        ~Protocol() {}

        int Domain() const { return m_domain; }    ///< 获取并返回address family
        int Type() const { return m_type; }      ///< 获取并返回socket type
        int Proto() const { return m_proto; }     ///< 获取并返回socket protocol value

        std::string ToString() const;
    public:
        static const Protocol  Tcp4;   ///< Tcp v4协议类
        static const Protocol  Udp4;   ///< Upd v4协议类
        static const Protocol  Tcp6;   ///< Tcp v6协议类
        static const Protocol  Udp6;   ///< Udp v6协议类
        static const Protocol  Unix;   ///< Unix管道协议类

        static const int DomainInet4;  ///< AF_INET
        static const int DomainInet6;  ///< AF_INT6
        static const int DomainUnix;   ///< AF_UNIX

        static const int TypeStream;   ///< SOCK_STREAM
        static const int TypeDatagram; ///< SOCK_DATADGRAM
    }; // end class Protocol

    /**
     * @brief IP地址基类。
     */
    class InetAddress {
    public:
        using Ptr = std::shared_ptr<InetAddress>;
        using Vector = std::vector<Ptr>;
    private:
        int      m_domain;
        void   * m_paddr;
        size_t   m_addrlen;
        mutable std::string m_hostname;

    protected:
        InetAddress(int domain, void *paddr, size_t addrlen);
        InetAddress(const InetAddress& other) : m_domain(other.m_domain) {}
        InetAddress & operator=(const InetAddress& other);

    public:
        virtual ~InetAddress();
        virtual bool operator==(const InetAddress& addr) const;

        int  Domain() const { return m_domain; }

        /// 获取IP地址转化为可读的文本格式。
        std::string  GetHostAddress() const;

        /// 获取IP地址对应的主机名，通常需要查询系统hosts文件或者域名服务。
        std::string GetHostName() const;

        /// 获取IP地址的字节数组。
        size_t GetAddress(void * buffer, size_t n) const; 

    public:
        /// 是否本地任意地址。
        virtual bool IsAnyLocalAddress() const = 0;

        /// 是否回环地址。
        virtual bool IsLoopbackAddress() const = 0;

        /// 转为字符串，格式 domain:address
        virtual std::string ToString() const = 0;

    public:
        /// 获取指定主机名的所有关联IP地址，返回实际获取的地址数
        static ssize_t GetAllByName(Vector & vecAddr, const char *hostname);

        /// 获取所有本地地址（不包括回环地址），返回实际获取的地址数。
        static ssize_t GetLocalHost(Vector & vecAddr);
    }; // end class InetAddress

    /**
     * @brief IPv4地址类。
     */
    class Inet4Address : public InetAddress {
    public:
        using Ptr = std::unique_ptr<Inet4Address>;

    private:
        uint32_t            m_addr;

    public:
        Inet4Address();  // 默认构造，初始化为一个ANY地址。
        Inet4Address(uint32_t addr);     // 构造指定的地址。
        Inet4Address(const char *addrstr, RuntimeError &errinfo);  // 文本地址构造。xxx.xxx.xxx.xxx
        Inet4Address(const Inet4Address &other);
        virtual ~Inet4Address();

        Inet4Address& operator=(const Inet4Address &other);
        virtual bool operator==(const Inet4Address &other) const;

    public:  // 继承自InetAddress
        /// 是否本地任意地址。
        virtual bool IsAnyLocalAddress() const;

        /// 是否回环地址。
        virtual bool IsLoopbackAddress() const;

        /// 转为字符串，格式: inet4:xxx.xxx.xxx.xxx
        virtual std::string ToString() const;
    }; // end class Inet4Address

    /**
     * @brief IPv6地址类。
     */
    class Inet6Address : public InetAddress {
    private:
        unsigned char       m_addr[16];
        mutable std::string m_hostname;

    public:
        Inet6Address();  // 默认构造，初始化为一个ANY地址。
        Inet6Address(const unsigned char addr[16], size_t n);     // 构造指定的地址。 n=16
        Inet6Address(const char *pszAddress, RuntimeError & errinfo);  // 文本地址构造, xxxx:xxxx:xxxx...。
        Inet6Address(const Inet6Address &other);
        virtual ~Inet6Address();

        Inet6Address& operator=(const Inet6Address &other);

        virtual bool operator==(const Inet6Address &other) const;

    public:  // 继承自InetAddress
        /// 是否本地任意地址。
        virtual bool IsAnyLocalAddress() const ;

        /// 是否回环地址。
        virtual bool IsLoopbackAddress() const;

        /// 转为字符串，格式: inet4:[xxxx:xxxx::xxxx]
        virtual std::string ToString() const;

    }; // end class Inet6Address

    /**
     * @brief Socket地址类
     */
    class SocketAddress {
    public:
        using Ptr = std::shared_ptr<SocketAddress>;

    public:
        virtual ~SocketAddress() { }

        virtual int Domain() const = 0;
        virtual struct sockaddr * CAddress() = 0;
        virtual const struct sockaddr * CAddress() const = 0;
        virtual socklen_t CAddressSize() const = 0;
    }; // end class SocketAddress

    /**
     * @brief InetSocketAddress类。
     */
    class InetSocketAddress : public SocketAddress {
    public:
        using Ptr = std::shared_ptr<InetSocketAddress>;
        
    private:
        struct InetSocketAddressImpl;
        InetSocketAddressImpl * m_pImpl;
    public:
        InetSocketAddress();
        InetSocketAddress(const struct sockaddr *paddr, socklen_t addrlen);
        InetSocketAddress(const InetAddress &rAddr, int port);
        InetSocketAddress(const InetSocketAddress &other);
        InetSocketAddress(InetSocketAddress &&other);
        virtual ~InetSocketAddress();

        InetSocketAddress & operator=(const InetSocketAddress &other);
        InetSocketAddress & operator=(InetSocketAddress &&other);
        
        int GetPort() const;
        const InetAddress * GetAddress() const;
        InetAddress * GetAddress();

        std::string ToString() const;

    public:
        virtual int Domain() const ;
        virtual struct sockaddr * CAddress() ;
        virtual const struct sockaddr * CAddress() const;
        virtual socklen_t CAddressSize() const ;
    }; // end class InetSocketAddress

    class SocketImpl;

    /**
     * @brief Socket基础类。
     */ 
    class SocketBase {
    public:
        using Ptr = std::unique_ptr<SocketBase>; 
    private:
        SocketImpl *m_pImpl;
        const char *m_pszTypeName;  // Socket类型名称

    protected:
        SocketBase(const char * pszTypeName);

        SocketBase(const SocketBase & other) = delete;
        SocketBase& operator = (const SocketBase & other) = delete;
        
    public:
        SocketImpl & getImpl() { return *m_pImpl; }
        const SocketImpl & getImpl() const { return *m_pImpl; }

    public:
        virtual ~SocketBase();

        int  fd() const;
        bool bind(const char *addr, int port, RuntimeError &errinfo);
        bool close(RuntimeError &e);
        
        bool isClosed() const;

        /*
         * 获取本地地址端口。
         */
        std::string getLocalAddress(RuntimeError & e) const;
        std::string getLocalAddress() const;
        int         getLocalPort(RuntimeError & e) const;
        int         getLocalPort() const;
        std::string getLocalEndpoint(RuntimeError &e) const;
        std::string getLocalEndpoint() const;

        /*
         * 设置和获取SO_REUSEADDR参数。
         */
        bool setReuseAddress(bool on, RuntimeError &e);
        int  getReuseAddress(RuntimeError &e) const;
        int  getReuseAddress() const;

        /*
         * 设置和获取Socket阻塞模式。getBlockMode返回0表示非阻塞，返回1表示阻塞，-1表示操作失败。
         */
        bool setBlockMode(bool bBlocked, RuntimeError &e);
        int  getBlockMode(RuntimeError &e) const;
        int  getBlockMode() const;
    }; // class SocketBase

    /**
     * @brief 负责服务端监听的Socket
     */
    class ServerSocket : public SocketBase {
    public:
        ServerSocket();
        virtual ~ServerSocket();

        StreamSocket * accept(RuntimeError &e);
        bool  accept(StreamSocket &sock, RuntimeError &e);

        bool  create(int domain, RuntimeError &e);
        bool  listen(int backlog, RuntimeError &errinfo);
        
        bool  setSoTimeout(int timeout, RuntimeError &e);
        int   getSoTimeout(RuntimeError &e) const;
        int   getSoTimeout() const;
    }; // end class ServerSocket

    /**
     * @brief StreamSocket类，面向数据流的Socket.
     */
    class StreamSocket : public SocketBase  {
    public:
        StreamSocket();
        virtual ~StreamSocket();

        bool    create(int domain, RuntimeError &e);
        bool    connect(const char *ip, int port, RuntimeError &e);
        ssize_t send(const char *buf, size_t len, RuntimeError &e);
        ssize_t receive(char * buf, size_t len, RuntimeError &e);
        bool    shutdownInput(RuntimeError &e);
        bool    shutdownInput();
        bool    shutdownOutput(RuntimeError &e);
        bool    shutdownOutput();

        bool    isConnected() const;  // 是否已完成连接
        bool    isConnecting() const; // 是否正在连接，连接方法执行和连接完成之间的状态
        bool    isInputShutdown() const;
        bool    isOutputShutdown() const;

        std::string getRemoteEndpoint(RuntimeError &e) const;
        std::string getRemoteEndpoint() const;
        std::string getRemoteAddres(RuntimeError &e) const;
        std::string getRemoteAddres() const;
        int         getRemotePort(RuntimeError &e) const;
        int         getRemotePort() const;

        bool    setSoKeepAlive(bool enable,  RuntimeError &e);
        int     getSoKeepAlive(RuntimeError &e) const;
        int     getSoKeepAlive() const;

        bool    setTcpNoDelay(int nodelay, RuntimeError &e);
        int     getTcpNodelay(RuntimeError &e) const;
        int     getTcpNodelay() const;
    }; // end class StreamSocket

    /**
     * @brief 面向数据报文的Socket.
     */
    class DatagramSocket : public SocketBase  {
    }; // end class DatagramSocket

}} // end namespace mercury::net
