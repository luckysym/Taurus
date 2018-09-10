#include <taurus/net/network.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>

namespace taurus  {
namespace net {

    const Protocol Protocol::Tcp4(AF_INET, SOCK_STREAM, 0);
    const Protocol Protocol::Udp4(AF_INET, SOCK_DGRAM, 0);

    const Protocol Protocol::Tcp6(AF_INET6, SOCK_STREAM, 0);
    const Protocol Protocol::Udp6(AF_INET6, SOCK_DGRAM, 0);

    const Protocol Protocol::Unix(AF_LOCAL, SOCK_STREAM, 0);

    const int Protocol::DomainInet4 = AF_INET;
    const int Protocol::DomainInet6 = AF_INET6;
    const int Protocol::DomainUnix = AF_LOCAL;

    const int Protocol::TypeStream  = SOCK_STREAM;
    const int Protocol::TypeDatagram = SOCK_DGRAM;

}} // end namespace taurus::net