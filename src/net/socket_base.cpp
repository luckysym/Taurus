#include <taurus/net/network.h>
#include <taurus/error_info.h>

#include "socket_impl.h"

namespace taurus {
namespace net {

SocketBase::SocketBase(const char * pszTypeName) 
    : m_pImpl(new SocketImpl), m_pszTypeName(pszTypeName) {}

SocketBase::~SocketBase() {
    delete m_pImpl;
    m_pImpl = nullptr;
    m_pszTypeName = nullptr;
}

int SocketBase::Fd() const { return m_pImpl->Fd(); }

bool SocketBase::Close(RuntimeError &errinfo) { 
    return m_pImpl->Close(errinfo);
}

bool SocketBase::Create(const Protocol &proto, RuntimeError & errinfo) {
    return m_pImpl->Create( proto, errinfo );
}

std::string SocketBase::getLocalAddress(RuntimeError & e) const {
    return m_pImpl->GetLocalAddress(e);
}

std::string SocketBase::getLocalAddress() const {
    RuntimeError e;
    return m_pImpl->GetLocalAddress(e);
}

int SocketBase::getLocalPort(RuntimeError & e) const {
    return m_pImpl->GetLocalPort(e);
}

int SocketBase::getLocalPort() const {
    RuntimeError e;
    return m_pImpl->GetLocalPort(e);
}

ServerSocket::ServerSocket() : SocketBase("ServerSocket") {}
ServerSocket::~ServerSocket() {}

bool ServerSocket::Bind(const char * host, int port, RuntimeError & errinfo) {
    return GetImpl().Bind(host, port, errinfo);
}

bool ServerSocket::listen(int backlog, RuntimeError &errinfo) {
    return GetImpl().Listen(backlog, errinfo);
}

} // end namespace net
} // end namespace taurus
