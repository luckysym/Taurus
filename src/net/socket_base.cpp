#include <mercury/net/network.h>
#include <mercury/error_info.h>

#include "socket_impl.h"
#include "socket_opt_impl.h"

#include <fcntl.h>

namespace mercury {
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

bool SocketBase::setBlockMode(bool bBlocked, RuntimeError &e) {
    int r = ::fcntl(m_pImpl->Fd(), F_GETFL);
	if ( r < 0 ) {
		std::ostringstream oss;
        oss<<"fcntl(F_GETFL) error, "<<syserr<<", fd: "<<m_pImpl->Fd();
        e.set(-1, oss.str().c_str(), "SocketBase::setBlockMode");
		return false;
	}

	if ( (r & O_NONBLOCK) && bBlocked )  // non-block to block
		r = fcntl(m_pImpl->Fd(), F_SETFL, r & (~O_NONBLOCK));
	else if ( !(r & O_NONBLOCK) && !bBlocked ) // block to non-block
		r = fcntl(m_pImpl->Fd(), F_SETFL, r | O_NONBLOCK);

	if ( r < 0 ) {
		std::ostringstream oss;
        oss<<"fcntl(F_SETFL) error, "<<syserr<<", fd: "<<m_pImpl->Fd();
        e.set(-1, oss.str().c_str(), "SocketBase::setBlockMode");
		return false;
	}
	return true;
}

int  SocketBase::getBlockMode(RuntimeError &e) const {
    int r = ::fcntl(m_pImpl->Fd(), F_GETFL);
	if ( r < 0 ) {
        std::ostringstream oss;
        oss<<"fcntl(F_GETFL) error, "<<syserr<<", fd: "<<m_pImpl->Fd();
        e.set(-1, oss.str().c_str(), "SocketBase::getBlockMode");
		return -1;
	}

	if ( r & O_NONBLOCK ) return 0;  // non-block
    else return 1;  // block i/o
}

int  SocketBase::getBlockMode() const {
    int r = ::fcntl(m_pImpl->Fd(), F_GETFL);
	if ( r < 0 ) return -1;
	if ( r & O_NONBLOCK ) return 0;  // non-block
    else return 1;  // block i/o
}


ServerSocket::ServerSocket() : SocketBase("ServerSocket") {}
ServerSocket::~ServerSocket() {}

bool ServerSocket::create(int domain, RuntimeError &e ) {
    Protocol proto(domain, Protocol::TypeStream, 0);
    return getImpl().Create( proto, e );
}

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

bool ServerSocket::setSoTimeout(int timeout, RuntimeError &e) {
    SocketOptRecvTimeout opt(this->fd());
    std::string errstr;
    bool isok = opt.Set(timeout, errstr);
    if ( isok ) return true;
    e.set(-1, errstr.c_str(), "ServerSocket::setSoTimeout");
    return false;
}

int ServerSocket::getSoTimeout(RuntimeError &e) const {
    SocketOptRecvTimeout opt(this->fd());
    int ms = 0;
    std::string errstr;
    bool isok = opt.Get(&ms, errstr);
    if ( isok ) return ms;
    e.set(-1,errstr.c_str(), "ServerSocket::getSoTimeout");
    return -1;
}

int ServerSocket::getSoTimeout() const {
    std::string errstr;
    SocketOptRecvTimeout opt(this->fd());
    int ms = 0;
    bool isok = opt.Get(&ms, errstr);
    if ( isok ) return ms;
    return -1;
}



StreamSocket::StreamSocket() : SocketBase("StreamSocket") {}
StreamSocket::~StreamSocket() {}



} // end namespace net
} // end namespace mercury
