#include <taurus/net/network.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

namespace taurus  {
namespace net {

InetAddress & InetAddress::operator=(const InetAddress& other) {
    if ( this != &other ) this->m_domain = other.m_domain;
    return *this;
}

bool InetAddress::operator==(const InetAddress& other) const {
    if ( this->m_domain == other.m_domain ) return true;
    else return false;
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


Inet4Address::Inet4Address() : InetAddress(Protocol::DomainInet4), m_addr(0) {}

Inet4Address::Inet4Address(uint32_t addr) : InetAddress(Protocol::DomainInet4), m_addr(addr) {}

Inet4Address::~Inet4Address() {}


bool Inet4Address::operator==(const Inet4Address &other) const {
    if ( m_addr != other.m_addr ) return false;
    return InetAddress::operator==(other);
}

Inet6Address::Inet6Address(const unsigned char addr[16]) 
    : InetAddress(Protocol::DomainInet6) 
{
    for(int i = 0; i < 16; ++i) m_addr[i] = addr[i];
}

Inet6Address::~Inet6Address() {}

bool Inet6Address::operator==(const Inet6Address &other) const {
    for(int i = 0; i < 16; ++i) {
        if ( m_addr[i] != other.m_addr[i] ) return false;
    }
    return InetAddress::operator==(other);
}

}} // end namespace taurus::net