#pragma once 
#include <mercury/error.h>

#include <cstdint>
#include <cassert>
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
        /// \brief 地址构造。
        /// \param domain  ip地址域，取值为AF_INET, AF_INET6。
        /// \param paddr   地址字节数组，其内容和长度由地址域决定。
        /// \param addrlen paddr地址中字节数，如IPV4长度为4，IPV6长度为32.
        InetAddress(int domain, void *paddr, size_t addrlen);

        InetAddress(const InetAddress& other) : m_domain(other.m_domain) {}
        InetAddress & operator=(const InetAddress& other);

    public:
        virtual ~InetAddress();
        virtual bool operator==(const InetAddress& addr) const;

        int  domain() const { return m_domain; }

        /// 获取IP地址转化为可读的文本格式。
        std::string  hostaddr() const;

        /// 获取IP地址对应的主机名，通常需要查询系统hosts文件或者域名服务。
        std::string hostname() const;

        /// 获取IP地址的字节数组。
        size_t address(void * buffer, size_t n) const; 

    public:
        /// 是否本地任意地址。
        virtual bool is_anylocal() const = 0;

        /// 是否回环地址。
        virtual bool is_loopback() const = 0;

        /// 转为字符串，格式 domain://hostaddr
        virtual std::string str() const = 0;

    public:
        /// 获取指定主机名的所有关联IP地址，返回实际获取的地址数
        static ssize_t get_all_by_name(Vector & vecAddr, const char *hostname);

        /// 获取所有本地地址（不包括回环地址），返回实际获取的地址数。
        static ssize_t get_localhost(Vector & vecAddr);
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
        virtual bool is_anylocal() const;

        /// 是否回环地址。
        virtual bool is_loopback() const;

        /// 转为字符串，格式: inet4:xxx.xxx.xxx.xxx
        virtual std::string str() const;
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
        virtual bool is_anylocal() const ;

        /// 是否回环地址。
        virtual bool is_loopback() const;

        /// 转为字符串，格式: inet4:[xxxx:xxxx::xxxx]
        virtual std::string str() const;

    }; // end class Inet6Address

    /**
     * @brief Socket地址类
     */
    class SocketAddress {
    public:
        using Ptr = std::shared_ptr<SocketAddress>;

    public:
        virtual ~SocketAddress() { }

        virtual int domain() const = 0;
        virtual struct sockaddr * caddr() = 0;
        virtual const struct sockaddr * caddr() const = 0;
        virtual socklen_t caddrsize() const = 0;
        virtual std::string str() const = 0;
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
        
        int port() const;
        const InetAddress * address() const;
        InetAddress * address();

    public:
        virtual int domain() const ;
        virtual struct sockaddr * caddr() ;
        virtual const struct sockaddr * caddr() const;
        virtual socklen_t caddrsize() const ;
        virtual std::string str() const;
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
        SocketImpl & impl() { return *m_pImpl; }
        const SocketImpl & impl() const { return *m_pImpl; }

    public:
        virtual ~SocketBase();

        int  fd() const;
        bool bind(const char *addr, int port, RuntimeError &errinfo);
        bool close(RuntimeError &e);

        bool is_closed() const;

        /*
         * 获取本地地址端口。
         */
        std::string local_addr(RuntimeError & e) const;
        std::string local_addr() const;
        int         local_port(RuntimeError & e) const;
        int         local_port() const;
        std::string local(RuntimeError &e) const;
        std::string local() const;

        /*
         * 设置和获取SO_REUSEADDR参数。
         */
        bool set_reuse_addr(int on, RuntimeError &e);
        int  get_reuse_addr(RuntimeError &e) const;
        int  get_reuse_addr() const;

        /*
         * 设置和获取Socket阻塞模式。get_block_mode返回0表示非阻塞，返回1表示阻塞，-1表示操作失败。
         */
        bool set_block_mode(bool bBlocked, RuntimeError &e);
        int  get_block_mode(RuntimeError &e) const;
        int  get_block_mode() const;
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
        
        bool  set_so_timeout(int timeout, RuntimeError &e);
        int   get_so_timeout(RuntimeError &e) const;
        int   get_so_timeout() const;
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
        bool    shutdown_input(RuntimeError &e);
        bool    shutdown_input();
        bool    shutdown_Output(RuntimeError &e);
        bool    shutdown_Output();

        bool    is_connected() const;  // 是否已完成连接
        bool    is_connecting() const; // 是否正在连接，连接方法执行和连接完成之间的状态
        bool    is_input_shutdown() const;
        bool    is_output_shutdown() const;

        std::string remote(RuntimeError &e) const;
        std::string remote() const;
        std::string remote_addr(RuntimeError &e) const;
        std::string remote_addr() const;
        int         remote_port(RuntimeError &e) const;
        int         remote_port() const;

        bool    set_so_keepalive(int enable,  RuntimeError &e);
        int     get_so_keepalive(RuntimeError &e) const;
        int     get_so_keepalive() const;

        bool    set_tcp_nodelay(int nodelay, RuntimeError &e);
        int     get_tcp_nodelay(RuntimeError &e) const;
        int     get_tcp_nodelay() const;
    }; // end class StreamSocket

    /** 
     * 数据报文对象，用于无连接的Socket的数据发送。
     */ 
    class DatagramPacket {
    private:
        char *       m_buf;
        size_t       m_len;
        size_t       m_cap;
        InetSocketAddress * m_endp;

    public:
        DatagramPacket() : m_buf(nullptr), m_len(0), m_cap(0), m_endp(nullptr) {}

        DatagramPacket(char * buf, size_t len) : m_buf(buf), m_len(len), m_cap(len), m_endp(nullptr) {}
        
        DatagramPacket(char * buf, size_t len, InetSocketAddress *endp)
            : m_buf(buf), m_len(len), m_cap(len), m_endp(endp) {}
        
        DatagramPacket(char * buf, size_t len, size_t cap)
            : m_buf(buf), m_len(len), m_cap(cap), m_endp(nullptr) {}

        DatagramPacket(char * buf, size_t len, size_t cap, InetSocketAddress *endp)
            : m_buf(buf), m_len(len), m_cap(cap), m_endp(nullptr) {}

        DatagramPacket(const DatagramPacket &other) 
            : m_buf(other.m_buf), m_len(other.m_len), m_cap(other.m_cap), m_endp(other.m_endp) {}

        DatagramPacket(DatagramPacket &&other)
            : m_buf(other.m_buf), m_len(other.m_len), m_cap(other.m_cap), m_endp(other.m_endp) 
        {
            other.m_buf = nullptr;
            other.m_endp = nullptr;
            other.m_len = other.m_cap  = 0;
        }

        ~DatagramPacket() {
            m_buf = nullptr;
            m_endp = nullptr;
            m_len = m_cap  = 0;
        }

        DatagramPacket & operator=(const DatagramPacket &other) {
            if ( this != &other ) {
                m_buf = other.m_buf;
                m_len = other.m_len;
                m_cap = other.m_cap;
                m_endp = other.m_endp;
            }
            return *this;
        }

        DatagramPacket & operator=(DatagramPacket &&other) {
            if ( this != &other ) {
                m_buf = other.m_buf;
                m_len = other.m_len;
                m_cap = other.m_cap;
                m_endp = other.m_endp;

                other.m_buf = nullptr;
                other.m_endp = nullptr;
                other.m_len = other.m_cap  = 0;
            }
            return *this;
        }

        InetSocketAddress * endpoint() const { return m_endp; }
        void setEndpoint(InetSocketAddress *ep) { m_endp = ep; }

        char * buffer() const { return m_buf; }
        size_t length() const { return m_len; }
        size_t capacity() const { return m_cap; }

        void setbuf(char * buf, size_t len, size_t cap) {
            m_buf = buf;
            m_len = len;
            m_cap = cap;
        }

        void setbuflen(size_t len) { assert(len <= m_cap);  m_len = len; }

    }; // end class DatagramPacket

    /**
     * @brief 面向数据报文的Socket.
     */
    class DatagramSocket : public SocketBase  {
    public:
        DatagramSocket();
        virtual ~DatagramSocket();

        bool    create(int domain, RuntimeError &e);
        ssize_t send(const DatagramPacket &data, RuntimeError &e);
        ssize_t receive(DatagramPacket *data, RuntimeError &e);
        bool    is_closed();
    }; // end class DatagramSocket

    class URL final {
    public:
        struct URL_Impl;
    private:
        struct URL_Impl * m_pImpl;

    public:
        URL();
        URL(const char * str, RuntimeError &e);
        URL(const char *schema, const char *host, int port);
        URL(const URL &other);
        URL(URL && other);
        ~URL();

        URL & operator=(const URL &other);
        URL & operator=(URL &&other);

        const char * schema() const;
        const char * host() const;
        int          port() const;

        std::string str() const;
    }; // end class URL

}} // end namespace mercury::net
