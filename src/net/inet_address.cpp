#include <mercury/net/network.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

#include <cassert>
#include <sstream>

namespace mercury  {
namespace net {

InetAddress::InetAddress(int domain, void * paddr, size_t addrlen)
    : m_domain(domain), m_paddr(paddr), m_addrlen(addrlen) {}

InetAddress::~InetAddress() {
    m_domain = AF_UNSPEC;
    m_paddr = nullptr;
    m_addrlen = 0;
}

InetAddress & InetAddress::operator=(const InetAddress& other) {
    if ( this != &other ) this->m_domain = other.m_domain;
    return *this;
}

bool InetAddress::operator==(const InetAddress& other) const {
    if ( this->m_domain == other.m_domain ) return true;
    else return false;
}

std::string InetAddress::hostaddr() const {
    char addrbuf[128] = {0};
    const char * p = ::inet_ntop(m_domain, m_paddr, addrbuf, 128);
    if ( p != nullptr ) {
        return std::string(addrbuf);
    } else {
        return std::string();
    }
}

std::string InetAddress::hostname() const {
    if ( !m_hostname.empty() ) return m_hostname;

    // 通过名称服务反向查找文件名
    std::string str;
    struct addrinfo *res = nullptr;
	struct addrinfo hints;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = m_domain;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_addr = nullptr;
	hints.ai_canonname = nullptr;
	hints.ai_next = nullptr;
    std::string addrname = this->hostaddr();
    int r = ::getaddrinfo(addrname.c_str(), nullptr, &hints, &res);
    if ( r > 0 ) str.assign(res->ai_canonname);
    freeaddrinfo(res);
    m_hostname = str;
    return std::move(str);
}

size_t InetAddress::address(void * buffer, size_t n) const {
    char * buf = (char *)buffer;
    n = n < m_addrlen?n:m_addrlen;
    for( size_t i = 0; i < n; ++i) buf[i] = ((const char *)m_paddr)[i];
    return m_addrlen;
}

ssize_t InetAddress::get_localhost(InetAddress::Vector & vecAddr) {
    // 获取网卡地址
	struct ifaddrs * ifaddr;
	int ret = getifaddrs(&ifaddr);
	if ( ret == -1 ) return -1;   // 错误

	// 根据地址类型，过滤出后，创建对应的SocketAddress对象实例并输出。
	ssize_t count = 0;
	struct ifaddrs * ifaddr1 = ifaddr;
	for(; ifaddr1 != nullptr; ifaddr1 = ifaddr1->ifa_next) {
		if ( ifaddr1->ifa_addr == nullptr ) continue;
		switch( ifaddr1->ifa_addr->sa_family ) {
		case AF_INET: {
			struct sockaddr_in* in4addr = (struct sockaddr_in*)ifaddr1->ifa_addr;
			if (  in4addr->sin_addr.s_addr != INADDR_ANY  && 
                  in4addr->sin_addr.s_addr != htonl(INADDR_LOOPBACK) )
            {
                Inet4Address * pAddr = new Inet4Address(in4addr->sin_addr.s_addr);
				vecAddr.push_back(InetAddress::Ptr(pAddr));
				count += 1;
			}
			break;
		}
		case AF_INET6: {
			struct sockaddr_in6* in6addr = (struct sockaddr_in6*)ifaddr1->ifa_addr;
			unsigned char * addr_bytes = in6addr->sin6_addr.s6_addr;

            if ( !IN6_IS_ADDR_UNSPECIFIED(in6addr->sin6_addr.s6_addr) &&
                 !IN6_IS_ADDR_LOOPBACK(in6addr->sin6_addr.s6_addr))
            {
                Inet6Address * pAddr = new Inet6Address(addr_bytes, 16);
				vecAddr.push_back(InetAddress::Ptr(pAddr));
				count += 1;
            }
			break;
		}
		} // end of switch
	}
	return count;
}

ssize_t InetAddress::get_all_by_name(InetAddress::Vector & vecAddr, const char *hostname) {
    struct addrinfo *res = nullptr;
	struct addrinfo hints;
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_addr = nullptr;
	hints.ai_canonname = nullptr;
	hints.ai_next = nullptr;

    int r = ::getaddrinfo(hostname, nullptr, &hints, &res);
    if ( r != 0 ) return -1;

    int count = 0;
	struct addrinfo * p = res;
	for( ; p != nullptr; p = p->ai_next) {
		if ( p->ai_addr == nullptr || p->ai_socktype != SOCK_STREAM ) continue;
		InetAddress *pAddr = nullptr;
        if ( p->ai_addr->sa_family == AF_INET) {
            pAddr = new Inet4Address(((struct sockaddr_in *)p->ai_addr)->sin_addr.s_addr);
		} else if ( p->ai_addr->sa_family == AF_INET6 ) {
            unsigned char * addr_bytes = ((struct sockaddr_in6 *)p->ai_addr)->sin6_addr.s6_addr;
            pAddr = new Inet6Address(addr_bytes, 16);
		}
        if ( pAddr ) {
            InetAddress::Ptr ptrAddr(pAddr);
            vecAddr.push_back(ptrAddr);
            ++count;
        }
	}
    return count;
} // end InetAddress::GetAllByName


Inet4Address::Inet4Address() 
    : InetAddress(Protocol::DomainInet4, &m_addr, sizeof(m_addr))
    , m_addr(INADDR_ANY) {}

Inet4Address::Inet4Address(uint32_t addr) 
    : InetAddress(Protocol::DomainInet4, &m_addr, sizeof(m_addr))
    , m_addr(addr) {}

Inet4Address::Inet4Address(const char * pszAddress, RuntimeError &errinfo) 
    : InetAddress(Protocol::DomainInet4, &m_addr, sizeof(m_addr))
    , m_addr(0) 
{
    int r = inet_pton(AF_INET, pszAddress, &m_addr);
    if ( r == 0 ) {
        std::string str;
        str.reserve(128);
        str.assign("NOT_A_VALID_INET4_ADDR").append(": ").append(pszAddress);
        errinfo.set(-1, str.c_str(), "Inet4Address::Inet4Address" );
    } else if ( r == -1 ) {
        assert(false); // r==-1表示af not support, 这不应该出现。
    }
}

Inet4Address::Inet4Address(const Inet4Address &other) 
    : InetAddress(Protocol::DomainInet4, &m_addr, sizeof(uint32_t)) 
    , m_addr(other.m_addr) {}

Inet4Address::~Inet4Address() {}

bool Inet4Address::operator==(const Inet4Address &other) const {
    if ( m_addr != other.m_addr ) return false;
    return InetAddress::operator==(other);
}

bool Inet4Address::is_loopback() const {
    return m_addr == htonl(INADDR_LOOPBACK);
}

bool Inet4Address::is_anylocal() const {
    return m_addr == INADDR_ANY;
}

std::string Inet4Address::str() const {
    std::string str;
    str.reserve(64);
    str.append("inet4://").append(this->hostaddr());
    return std::move(str);
}

Inet6Address::Inet6Address(const unsigned char addr[16], size_t n) 
    : InetAddress(Protocol::DomainInet6, m_addr, sizeof(m_addr))
{
    assert(n == 16);
    struct in6_addr * p1 = (struct in6_addr *)addr;
    struct in6_addr * p0 = (struct in6_addr *)m_addr;
    *p0 = *p1;
}

Inet6Address::Inet6Address(const char *pszAddress, RuntimeError &errinfo) 
    : InetAddress(Protocol::DomainInet6, m_addr, sizeof(m_addr)) {
    int r = inet_pton(AF_INET6, pszAddress, &m_addr);
    if ( r == 0 ) {
        std::string str;
        str.reserve(128);
        str.assign("NOT_A_VALID_INET6_ADDR").append(": ").append(pszAddress);
        errinfo.set(-1, str.c_str(), "Inet6Address::Inet6Address" );
    } else if ( r == -1 ) {
        assert(false); // r==-1表示af not support, 这不应该出现。
    }
}

Inet6Address::Inet6Address(const mercury::net::Inet6Address & other )
    : InetAddress(Protocol::DomainInet6, m_addr, sizeof(m_addr)) 
{
    struct in6_addr * p1 = (struct in6_addr *)other.m_addr;
    struct in6_addr * p0 = (struct in6_addr *)m_addr;
    *p0 = *p1;
}

Inet6Address::~Inet6Address() {}

bool Inet6Address::operator==(const Inet6Address &other) const {
    return IN6_ARE_ADDR_EQUAL(m_addr, other.m_addr); // netinet/in.h
}

bool Inet6Address::is_anylocal() const {
    return IN6_IS_ADDR_UNSPECIFIED(m_addr);  // netinet/in.h
}

bool Inet6Address::is_loopback() const {
    return IN6_IS_ADDR_LOOPBACK(m_addr);   // netinet/in.h
}

std::string Inet6Address::str() const {
    std::string str; str.reserve(64);
    str.append("inet6://[").append(this->hostaddr()).append(1, ']');
    return std::move(str);
}

/**
 * @brief InetSocketAddress内部实现类
 */
class InetSocketAddress::InetSocketAddressImpl {
    static const size_t ADDR_BUFFER_SIZE = 32;
private:
    InetAddress::Ptr m_ptrAddress;
    int              m_port;
    mutable char     m_addrbuf[ADDR_BUFFER_SIZE];

public:
    InetSocketAddressImpl(const struct sockaddr *paddr, socklen_t addrlen) {
        int af = paddr->sa_family;
        if ( af == AF_INET ) {
            auto pinaddr = (const struct sockaddr_in *)paddr;
            m_ptrAddress.reset(new Inet4Address(pinaddr->sin_addr.s_addr));
            m_port = ntohs(pinaddr->sin_port);
        } else if ( af == AF_INET6 ) {
            auto pin6addr = (const struct sockaddr_in6 *)paddr;
            m_ptrAddress.reset(new Inet6Address(pin6addr->sin6_addr.s6_addr, 16));
            m_port = ntohs(pin6addr->sin6_port);
        } else {
            throw std::runtime_error("InetSocketAddressImpl, invalid inet address domain");
        }
    }

    InetSocketAddressImpl(const InetAddress &rAddr, int port) 
    {
        m_port = port;
        if ( rAddr.domain() == Protocol::DomainInet4) {
            m_ptrAddress.reset(new Inet4Address((const Inet4Address&)rAddr));
        } else if ( rAddr.domain() == Protocol::DomainInet6) {
            m_ptrAddress.reset(new Inet6Address((const Inet6Address&)rAddr));
        } else {
            throw std::runtime_error("InetSocketAddressImpl, invalid inet address domain");
        }
    }

    InetSocketAddressImpl(const InetSocketAddressImpl &other) {
        m_port = other.m_port;
        if ( !other.m_ptrAddress ) return;   // 源地址为空，就不做复制了

        // 复制地址类
        if ( other.Domain() == Protocol::DomainInet4) {
            m_ptrAddress.reset(new Inet4Address((const Inet4Address&)*other.m_ptrAddress));
        } else if ( other.Domain() == Protocol::DomainInet6) {
            m_ptrAddress.reset(new Inet6Address((const Inet6Address&)*other.m_ptrAddress));
        } else {
            throw std::runtime_error("InetSocketAddressImpl, invalid inet address domain");
        }
    }

    int Domain() const { 
        if ( m_ptrAddress ) return m_ptrAddress->domain();
        else return 0;
    }

    int GetPort() const { return m_port; }

    InetAddress * GetAddress() { return m_ptrAddress.get(); }
    const InetAddress * GetAddress() const { return m_ptrAddress.get();}

    const struct sockaddr *GetCAddress() const {
        // 地址为空时，默认返回IPv4 ANY地址
        if ( !m_ptrAddress ) {
            struct sockaddr_in * pinaddr = (struct sockaddr_in *)m_addrbuf;
            pinaddr->sin_family = Protocol::DomainInet4;
            pinaddr->sin_port = htons(m_port);
            pinaddr->sin_addr.s_addr = INADDR_ANY;
            return (struct sockaddr *)pinaddr;
        }

        int af = m_ptrAddress->domain();
        if ( af == Protocol::DomainInet4) {
            struct sockaddr_in * pinaddr = (struct sockaddr_in *)m_addrbuf;
            pinaddr->sin_family = af;
            pinaddr->sin_port = htons(m_port);
            m_ptrAddress->address(&pinaddr->sin_addr.s_addr, sizeof(pinaddr->sin_addr.s_addr));
            return (struct sockaddr *)pinaddr;
        } else if ( af == Protocol::DomainInet6 ) {
            struct sockaddr_in6 *pinaddr6 = (struct sockaddr_in6 *)m_addrbuf;
            pinaddr6->sin6_family = af;
            pinaddr6->sin6_port = htons(m_port);
            m_ptrAddress->address(&pinaddr6->sin6_addr.s6_addr, sizeof(pinaddr6->sin6_addr.s6_addr));
            return (struct sockaddr *)pinaddr6;
        } else {
            throw std::runtime_error("InetSocketAddressImpl::GetCAddress, invalid inet address domain");
        }
    }

    socklen_t GetCAddressSize() const {
        if ( !m_ptrAddress ) return sizeof(struct sockaddr_in);
        int af = m_ptrAddress->domain();
        if ( af == Protocol::DomainInet4 ) return sizeof(struct sockaddr_in);
        else if ( af == Protocol::DomainInet6 ) return sizeof(struct sockaddr_in6);
        else throw std::runtime_error("InetSocketAddressImpl::GetCAddressSize, invalid inet address domain");
    }

    void str(std::string &str) const {
        std::ostringstream oss;
        if ( !m_ptrAddress ) oss<<"inet4:*:";
        else oss<<m_ptrAddress->str();
        oss<<':'<<m_port;
        str = oss.str();
    }
}; // end InetSocketAddress::InetSocketAddressImpl

InetSocketAddress::InetSocketAddress() : m_pImpl(nullptr) {}

InetSocketAddress::InetSocketAddress(const InetAddress &rAddr, int port) 
    : m_pImpl(new InetSocketAddressImpl(rAddr, port) ) 
{}

InetSocketAddress::InetSocketAddress(const struct sockaddr *paddr, socklen_t addrlen)
    : m_pImpl(new InetSocketAddressImpl(paddr, addrlen))
{}

InetSocketAddress::InetSocketAddress(const InetSocketAddress &other) 
    : m_pImpl(new InetSocketAddressImpl(*other.m_pImpl)) {}

InetSocketAddress::InetSocketAddress(InetSocketAddress &&other) 
    : m_pImpl(other.m_pImpl) { other.m_pImpl = nullptr; }

InetSocketAddress::~InetSocketAddress() {
    if ( m_pImpl ) { 
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

InetSocketAddress & InetSocketAddress::operator=(const InetSocketAddress &other) {
    if ( this == &other ) return *this;
    *m_pImpl = *other.m_pImpl;
    return *this;
}

InetSocketAddress & InetSocketAddress::operator=(InetSocketAddress &&other) {
    if ( m_pImpl ) delete m_pImpl;
    m_pImpl = other.m_pImpl;
    other.m_pImpl = nullptr;
    return *this;
}

int InetSocketAddress::port() const {
    if ( m_pImpl ) return m_pImpl->GetPort();
    else return 0;
}

const InetAddress * InetSocketAddress::address() const {
    if ( m_pImpl ) return m_pImpl->GetAddress();
    else return nullptr;
}

InetAddress * InetSocketAddress::address() {
    if ( m_pImpl ) return m_pImpl->GetAddress();
    else return nullptr;
}

std::string InetSocketAddress::str() const {
    std::string str;
    if ( m_pImpl ) m_pImpl->str(str);
    return str;
}

int InetSocketAddress::domain() const {
    if ( m_pImpl ) m_pImpl->Domain();
    else return Protocol::DomainInet4;   // 默认ipv4
}

struct sockaddr * InetSocketAddress::caddr()  {
    if ( m_pImpl ) m_pImpl->GetCAddress();
    else return nullptr;
}
const struct sockaddr * InetSocketAddress::caddr() const {
    if ( m_pImpl ) m_pImpl->GetCAddress();
    else return nullptr;
}
socklen_t InetSocketAddress::caddrsize() const {
    if ( m_pImpl ) m_pImpl->GetCAddressSize();
    else return 0;
}

}} // end namespace mercury::net
