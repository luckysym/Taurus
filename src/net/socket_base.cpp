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

bool SocketBase::is_closed() const { return m_pImpl->State() == SocketImpl::SOCK_STATE_CLOSED; }

std::string SocketBase::local_addr(RuntimeError & e) const {
    return m_pImpl->GetLocalAddress(e);
}

std::string SocketBase::local_addr() const {
    RuntimeError e;
    std::string str = m_pImpl->GetLocalAddress(e);
    if ( e ) throw e;
    return str;
}

int SocketBase::local_port(RuntimeError & e) const {
    return m_pImpl->GetLocalPort(e);
}

int SocketBase::local_port() const {
    RuntimeError e;
    int port = m_pImpl->GetLocalPort(e);
    if ( e ) throw e;
}

std::string SocketBase::local(RuntimeError &e) const  {
    return m_pImpl->GetLocalEndpoint(e);
}

std::string SocketBase::local() const {
    RuntimeError e;
    std::string str = m_pImpl->GetLocalEndpoint(e);
    if ( !e ) return str;
    throw e;
}

bool SocketBase::set_reuse_addr(int on, RuntimeError &e) {
    SocketOptReuseAddr opt( m_pImpl->Fd() );
    return opt.Set(on, e);
}

int SocketBase::get_reuse_addr(RuntimeError &e) const {
    SocketOptReuseAddr opt(m_pImpl->Fd());
    int value;
    if ( opt.Get(&value, e) ) return value?1:0;
    else return -1;
}

int SocketBase::get_reuse_addr() const {
    SocketOptReuseAddr opt(m_pImpl->Fd());
    RuntimeError e;
    int value;
    if ( opt.Get(&value, e) ) return value?1:0;
    return -1;
}

bool SocketBase::set_block_mode(bool bBlocked, RuntimeError &e) {
    int r = ::fcntl(m_pImpl->Fd(), F_GETFL);
	if ( r < 0 ) {
		std::ostringstream oss;
        oss<<"fcntl(F_GETFL) error, "<<syserr<<", fd: "<<m_pImpl->Fd();
        e.set(-1, oss.str().c_str(), "SocketBase::set_block_mode");
		return false;
	}

	if ( (r & O_NONBLOCK) && bBlocked )  // non-block to block
		r = fcntl(m_pImpl->Fd(), F_SETFL, r & (~O_NONBLOCK));
	else if ( !(r & O_NONBLOCK) && !bBlocked ) // block to non-block
		r = fcntl(m_pImpl->Fd(), F_SETFL, r | O_NONBLOCK);

	if ( r < 0 ) {
		std::ostringstream oss;
        oss<<"fcntl(F_SETFL) error, "<<syserr<<", fd: "<<m_pImpl->Fd();
        e.set(-1, oss.str().c_str(), "SocketBase::set_block_mode");
		return false;
	}
	return true;
}

int  SocketBase::get_block_mode(RuntimeError &e) const {
    int r = ::fcntl(m_pImpl->Fd(), F_GETFL);
	if ( r < 0 ) {
        std::ostringstream oss;
        oss<<"fcntl(F_GETFL) error, "<<syserr<<", fd: "<<m_pImpl->Fd();
        e.set(-1, oss.str().c_str(), "SocketBase::get_block_mode");
		return -1;
	}

	if ( r & O_NONBLOCK ) return 0;  // non-block
    else return 1;  // block i/o
}

int  SocketBase::get_block_mode() const {
    int r = ::fcntl(m_pImpl->Fd(), F_GETFL);
	if ( r < 0 ) return -1;
	if ( r & O_NONBLOCK ) return 0;  // non-block
    else return 1;  // block i/o
}


ServerSocket::ServerSocket() : SocketBase("ServerSocket") {}
ServerSocket::~ServerSocket() {}

bool ServerSocket::create(int domain, RuntimeError &e ) {
    Protocol proto(domain, Protocol::TypeStream, 0);
    return impl().Create( proto, e );
}

bool ServerSocket::listen(int backlog, RuntimeError &errinfo) {
    return impl().Listen(backlog, errinfo);
}

StreamSocket * ServerSocket::accept(RuntimeError &e) {
    SocketBase::Ptr  ptr (new StreamSocket());
    bool isok = this->impl().accept(ptr->impl(), e);
    if ( isok ) return (StreamSocket *)ptr.release();
    else return nullptr;
}

bool ServerSocket::accept(StreamSocket &rSock, RuntimeError &e) {
    SocketBase * p = static_cast<SocketBase *>(&rSock);
    return this->impl().accept(p->impl(), e);
}

bool ServerSocket::set_so_timeout(int timeout, RuntimeError &e) {
    SocketOptRecvTimeout opt(this->fd());
    if ( opt.Set(timeout, e) ) return true;
    else return false;
}

int ServerSocket::get_so_timeout(RuntimeError &e) const {
    SocketOptRecvTimeout opt(this->fd());
    int ms;
    if ( opt.Get(&ms, e) ) return ms;
    else return -1;
}

int ServerSocket::get_so_timeout() const {
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
    return impl().Create(proto, e);
}

bool StreamSocket::connect(const char *ip, int port, RuntimeError &e) {
    return impl().Connect(ip, port, e);
}

ssize_t StreamSocket::send(const char *buf, size_t len, RuntimeError &e) {
    SocketWriterImpl writer(impl().Fd(), buf, len);
    return writer(e);
}

ssize_t StreamSocket::receive(char *buf, size_t len, RuntimeError &e) {
    SocketReaderImpl reader(impl().Fd(), buf, len);
    return reader(e);
}

bool StreamSocket::shutdown_input(RuntimeError &e) {
    return impl().ShutdownInput( e);
}

bool StreamSocket::shutdown_Output(RuntimeError &e) {
    return impl().ShutdownOutput( e);
}

bool StreamSocket::shutdown_input() {
    RuntimeError e;
    return impl().ShutdownInput( e);
}

bool StreamSocket::shutdown_Output() {
    RuntimeError e;
    return impl().ShutdownOutput( e);
}

bool StreamSocket::is_connected() const {
    return impl().State() == SocketImpl::SOCK_STATE_OPEN;
}

bool StreamSocket::is_connecting() const {
    return impl().State() == SocketImpl::SOCK_STATE_OPENING;
}

bool StreamSocket::is_input_shutdown() const { 
    return impl().ShutdownState() & SocketImpl::SOCK_SHUT_READ; 
}

bool StreamSocket::is_output_shutdown() const {
    return impl().ShutdownState() & SocketImpl::SOCK_SHUT_WRITE;
}

std::string StreamSocket::remote(RuntimeError &e) const {
    return impl().GetRemoteEndpoint(e);
}

std::string StreamSocket::remote() const {
    RuntimeError e;
    return impl().GetRemoteEndpoint(e);
}

std::string StreamSocket::remote_addr(RuntimeError &e) const {
    return impl().GetRemoteAddress(e);
}

std::string StreamSocket::remote_addr() const {
    RuntimeError e;
    return impl().GetRemoteAddress(e);
}

int StreamSocket::remote_port(RuntimeError &e) const {
    return impl().GetRemotePort(e);
}

int StreamSocket::remote_port() const {
    RuntimeError e;
    return impl().GetRemotePort(e);
}

bool StreamSocket::set_so_keepalive(int enable, RuntimeError &e) {
    SocketOptKeepAlive opt(impl().Fd());
    return opt.Set(enable, e);
}

int StreamSocket::get_so_keepalive(RuntimeError &e) const {
    SocketOptKeepAlive opt(impl().Fd());
    int enable;
    if ( opt.Get(&enable, e) ) return enable?1:0;
    else return -1;
}

int StreamSocket::get_so_keepalive() const {
    SocketOptKeepAlive opt(impl().Fd());
    int enable;
    RuntimeError e;
    if ( opt.Get(&enable, e) ) return enable?1:0;
    return -1;  // error
}

bool StreamSocket::set_tcp_nodelay(int nodelay, RuntimeError &e) {
    SocketOptTcpNoDelay opt( impl().Fd());
    return opt.Set(nodelay, e);
}

int StreamSocket::get_tcp_nodelay(RuntimeError &e) const {
    SocketOptTcpNoDelay opt( impl().Fd() ) ;
    int nodelay;
    if ( opt.Get(&nodelay, e) ) return nodelay;
    else return -1;
}

int StreamSocket::get_tcp_nodelay() const {
    RuntimeError e;
    SocketOptTcpNoDelay opt( impl().Fd() ) ;
    int nodelay;
    if ( opt.Get(&nodelay, e) ) return nodelay;
    else return -1;
}

DatagramSocket::DatagramSocket() : SocketBase("DatagramSocket") {}

DatagramSocket::~DatagramSocket() {}

bool DatagramSocket::create(int domain, RuntimeError &e) {
    Protocol proto(domain, Protocol::TypeDatagram, 0);
    return impl().Create(proto, e);
}

ssize_t DatagramSocket::send(const DatagramPacket &data, RuntimeError &e) {
    SocketWriterImpl writer(impl().Fd(), data.buffer(), data.length());
    return writer.Write(*data.endpoint(), e);
}

ssize_t DatagramSocket::receive(DatagramPacket *data, RuntimeError &e) {
    char * buffer = data->buffer() + data->length();
    size_t length = data->capacity() - data->length();
    SocketReaderImpl reader(impl().Fd(), buffer, length);
    ssize_t r = reader.Read(data->endpoint(), e);
    if ( r > 0 ) data->setbuflen(length + r);
    return r;
}

} // end namespace net
} // end namespace mercury
