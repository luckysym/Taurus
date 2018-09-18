#pragma once 

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace taurus  {
namespace net {

    /**
     * @brief Socket协议对象。
     */
    class Protocol final {
    private:
    	int m_domain;    ///< address family
	    int m_type;      ///< socket type, SOCK_STREAM, SOCK_DGRAM
	    int m_proto;     ///< socket protocol

    public:
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
        int Value() const { return m_proto; }     ///< 获取并返回socket protocol value

    public:
        static const Protocol  Tcp4;   ///< Tcp v4协议类
        static const Protocol  Udp4;   ///< Upd v4协议类
        static const Protocol  Tcp6;   ///< Tcp v6协议类
        static const Protocol  Udp6;   ///< Udp v6协议类
        static const Protocol  Unix;   ///< Unix管道协议类

        static const int DomainInet4;
        static const int DomainInet6;
        static const int DomainUnix;

        static const int TypeStream;
        static const int TypeDatagram;
    }; // end class Protocol

    /**
     * @brief IP地址基类。
     */
    class InetAddress {
    public:
        typedef std::shared_ptr<InetAddress> Ptr;
        typedef std::vector<Ptr> Vector;
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
        size_t GetAddress(char * buffer, size_t n) const; 

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
        uint32_t m_addr;
        mutable std::string m_hostname;
    public:
        Inet4Address();  // 默认构造，初始化为一个ANY地址。
        Inet4Address(uint32_t addr);     // 构造指定的地址。
        Inet4Address(const char *addr);  // 文本地址构造。
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
        unsigned char m_addr[16];
        mutable std::string m_hostname;
    public:
        Inet6Address();  // 默认构造，初始化为一个ANY地址。
        Inet6Address(const unsigned char addr[16], size_t n);     // 构造指定的地址。 n=16
        Inet6Address(const char *addr);  // 文本地址构造。
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

}} // end namespace taurus::net
