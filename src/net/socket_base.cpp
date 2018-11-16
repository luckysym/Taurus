#include <mercury/net/network.h>
#include <mercury/error_info.h>

#include "socket_impl.h"
#include "socket_opt_impl.h"
#include "socket_io_impl.h"

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

bool SocketBase::bind(const char * host, int port, RuntimeError & errinfo) {
    return m_pImpl->Bind(host, port, errinfo);
}

bool SocketBase::isClosed() const { return m_pImpl->State() == SocketImpl::SOCK_STATE_CLOSED; }

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

bool SocketBase::setReuseAddress(int on, RuntimeError &e) {
    SocketOptReuseAddr opt( m_pImpl->Fd() );
    return opt.Set(on, e);
}

int SocketBase::getReuseAddress(RuntimeError &e) const {
    SocketOptReuseAddr opt(m_pImpl->Fd());
    int value;
    if ( opt.Get(&value, e) ) return value?1:0;
    else return -1;
}

int SocketBase::getReuseAddress() const {
    SocketOptReuseAddr opt(m_pImpl->Fd());
    RuntimeError e;
    int value;
    if ( opt.Get(&value, e) ) return value?1:0;
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
    if ( opt.Set(timeout, e) ) return true;
    else return false;
}

int ServerSocket::getSoTimeout(RuntimeError &e) const {
    SocketOptRecvTimeout opt(this->fd());
    int ms;
    if ( opt.Get(&ms, e) ) return ms;
    else return -1;
}

int ServerSocket::getSoTimeout() const {
    RuntimeError e;
    SocketOptRecvTimeout opt(this->fd());
    int ms = 0;
    if ( opt.Get(&ms, e) ) return ms;
    else return -1;
}

StreamSocket::StreamSocket() : SocketBase("StreamSocket") {}
StreamSocket::~StreamSocket() {}

bool StreamSocket::create(int domain, RuntimeError &e) {
    Protocol proto(domain, Protocol::TypeStream, 0);
    return getImpl().Create(proto, e);
}

bool StreamSocket::connect(const char *ip, int port, RuntimeError &e) {
    return getImpl().Connect(ip, port, e);
}

ssize_t StreamSocket::send(const char *buf, size_t len, RuntimeError &e) {
    SocketWriterImpl writer(getImpl().Fd(), buf, len);
    return writer(e);
}

ssize_t StreamSocket::receive(char *buf, size_t len, RuntimeError &e) {
    SocketReaderImpl reader(getImpl().Fd(), buf, len);
    return reader(e);
}

bool StreamSocket::shutdownInput(RuntimeError &e) {
    return getImpl().ShutdownInput( e);
}

bool StreamSocket::shutdownOutput(RuntimeError &e) {
    return getImpl().ShutdownOutput( e);
}

bool StreamSocket::shutdownInput() {
    RuntimeError e;
    return getImpl().ShutdownInput( e);
}

bool StreamSocket::shutdownOutput() {
    RuntimeError e;
    return getImpl().ShutdownOutput( e);
}

bool StreamSocket::isConnected() const {
    return getImpl().State() == SocketImpl::SOCK_STATE_OPEN;
}

bool StreamSocket::isConnecting() const {
    return getImpl().State() == SocketImpl::SOCK_STATE_OPENING;
}

bool StreamSocket::isInputShutdown() const { 
    return getImpl().ShutdownState() & SocketImpl::SOCK_SHUT_READ; 
}

bool StreamSocket::isOutputShutdown() const {
    return getImpl().ShutdownState() & SocketImpl::SOCK_SHUT_WRITE;
}

std::string StreamSocket::getRemoteEndpoint(RuntimeError &e) const {
    return getImpl().GetRemoteEndpoint(e);
}

std::string StreamSocket::getRemoteEndpoint() const {
    RuntimeError e;
    return getImpl().GetRemoteEndpoint(e);
}

std::string StreamSocket::getRemoteAddress(RuntimeError &e) const {
    return getImpl().GetRemoteAddress(e);
}

std::string StreamSocket::getRemoteAddress() const {
    RuntimeError e;
    return getImpl().GetRemoteAddress(e);
}

int StreamSocket::getRemotePort(RuntimeError &e) const {
    return getImpl().GetRemotePort(e);
}

int StreamSocket::getRemotePort() const {
    RuntimeError e;
    return getImpl().GetRemotePort(e);
}

bool StreamSocket::setSoKeepAlive(int enable, RuntimeError &e) {
    SocketOptKeepAlive opt(getImpl().Fd());
    return opt.Set(enable, e);
}

int StreamSocket::getSoKeepAlive(RuntimeError &e) const {
    SocketOptKeepAlive opt(getImpl().Fd());
    int enable;
    if ( opt.Get(&enable, e) ) return enable?1:0;
    else return -1;
}

int StreamSocket::getSoKeepAlive() const {
    SocketOptKeepAlive opt(getImpl().Fd());
    int enable;
    RuntimeError e;
    if ( opt.Get(&enable, e) ) return enable?1:0;
    return -1;  // error
}

bool StreamSocket::setTcpNoDelay(int nodelay, RuntimeError &e) {
    SocketOptTcpNoDelay opt( getImpl().Fd());
    return opt.Set(nodelay, e);
}

int StreamSocket::getTcpNodelay(RuntimeError &e) const {
    SocketOptTcpNoDelay opt( getImpl().Fd() ) ;
    int nodelay;
    if ( opt.Get(&nodelay, e) ) return nodelay;
    else return -1;
}

int StreamSocket::getTcpNodelay() const {
    RuntimeError e;
    SocketOptTcpNoDelay opt( getImpl().Fd() ) ;
    int nodelay;
    if ( opt.Get(&nodelay, e) ) return nodelay;
    else return -1;
}

DatagramSocket::DatagramSocket() : SocketBase("DatagramSocket") {}

DatagramSocket::~DatagramSocket() {}

bool DatagramSocket::create(int domain, RuntimeError &e) {
    Protocol proto(domain, Protocol::TypeDatagram, 0);
    return getImpl().Create(proto, e);
}

ssize_t DatagramSocket::send(const DatagramPacket &data, RuntimeError &e) {
    SocketWriterImpl writer(getImpl().Fd(), data.buffer(), data.length());
    return writer.Write(*data.endpoint(), e);
}

ssize_t DatagramSocket::receive(DatagramPacket *data, RuntimeError &e) {
    char * buffer = data->buffer() + data->length();
    size_t length = data->capacity() - data->length();
    SocketReaderImpl reader(getImpl().Fd(), buffer, length);
    ssize_t r = reader.Read(data->endpoint(), e);
    if ( r > 0 ) data->setLength(length + r);
    return r;
}

} // end namespace net
} // end namespace mercury
