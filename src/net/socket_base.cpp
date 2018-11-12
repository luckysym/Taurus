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

bool SocketBase::Close(std::string &e) { 
    return m_pImpl->Close(e);
}

bool SocketBase::Create(const Protocol &proto, ErrorInfo & errinfo) {
    return m_pImpl->Create( proto, errinfo );
}

ServerSocket::ServerSocket() : SocketBase("ServerSocket") {}
ServerSocket::~ServerSocket() {}

} // end namespace net
} // end namespace taurus
