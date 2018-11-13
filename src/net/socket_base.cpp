#include <taurus/net/network.h>
#include <taurus/error_info.h>

#include "socket_impl.h"
#include "socket_opt_impl.h"

namespace taurus {
namespace net {

SocketBase::SocketBase(const char * pszTypeName) 
    : m_pImpl(new SocketImpl), m_pszTypeName(pszTypeName) {}

SocketBase::~SocketBase() {
    delete m_pImpl;
    m_pImpl = nullptr;
    m_pszTypeName = nullptr;
}

int SocketBase::fd() const { return m_pImpl->Fd(); }

bool SocketBase::close(RuntimeError &e) { 
    return m_pImpl->Close(e);
}

bool SocketBase::create(const Protocol &proto, RuntimeError & e) {
    return m_pImpl->Create( proto, e );
}

std::string SocketBase::getLocalAddress(RuntimeError & e) const {
    return m_pImpl->GetLocalAddress(e);
}

std::string SocketBase::getLocalAddress() const {
    RuntimeError e;
    std::string str = m_pImpl->GetLocalAddress(e);
    if ( e ) throw e;
    return str;
}

int SocketBase::getLocalPort(RuntimeError & e) const {
    return m_pImpl->GetLocalPort(e);
}

int SocketBase::getLocalPort() const {
    RuntimeError e;
    int port = m_pImpl->GetLocalPort(e);
    if ( e ) throw e;
}

std::string SocketBase::getLocalEndpoint(RuntimeError &e) const  {
    return m_pImpl->GetLocalEndpoint(e);
}

std::string SocketBase::getLocalEndpoint() const {
    RuntimeError e;
    std::string str = m_pImpl->GetLocalEndpoint(e);
    if ( !e ) return str;
    throw e;
}

bool SocketBase::setReuseAddress(bool on, RuntimeError &e) {
    SocketOptReuseAddr opt( m_pImpl->Fd() );
    std::string errstr;
    bool isok = opt.Set(on, errstr);
    if ( isok ) return true;
    e.set(-1, errstr.c_str(), "SocketBase::setReuseAddress");
    return false;
}

int SocketBase::getReuseAddress(RuntimeError &e) const {
    SocketOptReuseAddr opt(m_pImpl->Fd());
    std::string errstr;
    bool value;
    bool isok = opt.Get(&value, errstr);
    if ( isok ) return value?1:0;
    e.set(-1, errstr.c_str(), "SocketBase::getReuseAddress");
    return -1;
}

int SocketBase::getReuseAddress() const {
    SocketOptReuseAddr opt(m_pImpl->Fd());
    std::string errstr;
    bool value;
    bool isok = opt.Get(&value, errstr);
    if ( isok ) return value?1:0;
    return -1;
}

ServerSocket::ServerSocket() : SocketBase("ServerSocket") {}
ServerSocket::~ServerSocket() {}

bool ServerSocket::bind(const char * host, int port, RuntimeError & errinfo) {
    return getImpl().Bind(host, port, errinfo);
}

bool ServerSocket::listen(int backlog, RuntimeError &errinfo) {
    return getImpl().Listen(backlog, errinfo);
}

StreamSocket * ServerSocket::accept(RuntimeError &e) {
    SocketBase::Ptr  ptr (new StreamSocket());
    bool isok = this->getImpl().accept(ptr->getImpl(), e);
    if ( isok ) return (StreamSocket *)ptr.release();
    else return nullptr;
}

bool ServerSocket::accept(StreamSocket &rSock, RuntimeError &e) {
    SocketBase * p = static_cast<SocketBase *>(&rSock);
    return this->getImpl().accept(p->getImpl(), e);
}

StreamSocket::StreamSocket() : SocketBase("StreamSocket") {}

} // end namespace net
} // end namespace taurus
