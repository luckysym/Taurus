#include <taurus/net/network.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

namespace taurus  {
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

std::string InetAddress::GetHostAddress() const {
    char addrbuf[128] = {0};
    const char * p = ::inet_ntop(m_domain, m_paddr, addrbuf, 128);
    if ( p != nullptr ) {
        return std::string(addrbuf);
    } else {
        return std::string();
    }
}

std::string InetAddress::GetHostName() const {
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
    std::string addrname = this->GetHostAddress();
    int r = ::getaddrinfo(addrname.c_str(), nullptr, &hints, &res);
    if ( r > 0 ) str.assign(res->ai_canonname);
    freeaddrinfo(res);
    m_hostname = str;
    return std::move(str);
}

size_t InetAddress::GetAddress(char * buffer, size_t n) const {
    n = n < m_addrlen?n:m_addrlen;
    for( size_t i = 0; i < n; ++i) buffer[i] = ((const char *)m_paddr)[i];
    return m_addrlen;
}

ssize_t InetAddress::GetAllByName(InetAddress::Vector & vecAddr, const char *hostname) {
    struct addrinfo *res = nullptr;
	struct addrinfo hints;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_addr = nullptr;
	hints.ai_canonname = nullptr;
	hints.ai_next = nullptr;

    int r = ::getaddrinfo(hostname, nullptr, &hints, &res);
    if ( r < 0 ) return -1;

    int count = 0;
	struct addrinfo * p = res;
	for( ; p != nullptr; p = p->ai_next) {
		if ( p->ai_addr == nullptr || p->ai_socktype != SOCK_STREAM ) continue;
		InetAddress *pAddr = nullptr;
        if ( p->ai_addr->sa_family == AF_INET) {
            Inet4Address *pAddr = new Inet4Address(((struct sockaddr_in *)p->ai_addr)->sin_addr.s_addr);
		} else if ( p->ai_addr->sa_family == AF_INET6 ) {
            Inet6Address *pAddr = new Inet6Address(((struct sockaddr_in6 *)p->ai_addr)->sin6_addr.s6_addr);
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

Inet4Address::~Inet4Address() {}

bool Inet4Address::operator==(const Inet4Address &other) const {
    if ( m_addr != other.m_addr ) return false;
    return InetAddress::operator==(other);
}

bool Inet4Address::IsLoopbackAddress() const {
    return m_addr == INADDR_LOOPBACK;
}

bool Inet4Address::IsAnyLocalAddress() const {
    return m_addr == INADDR_ANY;
}

std::string Inet4Address::ToString() const {
    std::string str;
    str.reserve(64);
    str.append("inet4:").append(this->GetHostAddress());
    return std::move(str);
}

Inet6Address::Inet6Address(const unsigned char addr[16]) 
    : InetAddress(Protocol::DomainInet6, m_addr, sizeof(m_addr))
{
    struct in6_addr * p1 = (struct in6_addr *)addr;
    struct in6_addr * p0 = (struct in6_addr *)m_addr;
    *p0 = *p1;
}

Inet6Address::~Inet6Address() {}

bool Inet6Address::operator==(const Inet6Address &other) const {
    return IN6_ARE_ADDR_EQUAL(m_addr, other.m_addr); // netinet/in.h
}

bool Inet6Address::IsAnyLocalAddress() const {
    return IN6_IS_ADDR_UNSPECIFIED(m_addr);  // netinet/in.h
}

bool Inet6Address::IsLoopbackAddress() const {
    return IN6_IS_ADDR_LOOPBACK(m_addr);   // netinet/in.h
}


std::string Inet6Address::ToString() const {
    std::string str; str.reserve(64);
    str.append("inet6:").append(this->GetHostAddress());
    return std::move(str);
}

}} // end namespace taurus::net