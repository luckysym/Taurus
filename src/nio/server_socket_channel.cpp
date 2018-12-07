#include <mercury/nio/channel.h>

namespace mercury {
namespace nio {

SelectableChannel::SelectableChannel() {
    // not impl
}

SelectableChannel::~SelectableChannel() {
    // not impl
}

class Selector::SelectorImpl{
private:
public:
    bool open(RuntimeError &e);
    bool close(RuntimeError &e);
    bool is_open() const;
}; // end class Selector::SelectorImpl

Selector::Selector() : m_pImpl(nullptr) {}

Selector::~Selector() {
    if ( m_pImpl ) {
        delete m_pImpl; 
        m_pImpl = nullptr; 
    }
}

bool Selector::open(RuntimeError &e) {
    if ( m_pImpl == nullptr ) m_pImpl = new SelectorImpl();

    if ( !m_pImpl->is_open( ))  {
        return m_pImpl->open(e);
    } else {
        return true;
    }
}

bool Selector::close(RuntimeError &e) {
    if ( m_pImpl ) return m_pImpl->close(e);
    else return true;
}

class ServerSocketChannel::ServerSocketChannelImpl {
public:
    mercury::net::ServerSocket m_socket;
}; // end class ServerSocketChannel::ServerSocketChannelImpl 

ServerSocketChannel::ServerSocketChannel() : m_pSockImpl(nullptr) {}

ServerSocketChannel::~ServerSocketChannel() {
    if ( m_pSockImpl ) {
        delete m_pSockImpl;
        m_pSockImpl = nullptr;
    }
}

bool ServerSocketChannel::create(int domain, RuntimeError &e) {
    if ( m_pSockImpl == nullptr) m_pSockImpl = new ServerSocketChannelImpl();
    assert( m_pSockImpl->m_socket.is_closed() );
    return m_pSockImpl->m_socket.create(domain, e); 
}

bool ServerSocketChannel::close(RuntimeError &e ) {
    if ( m_pSockImpl != nullptr && !m_pSockImpl->m_socket.is_closed()) {
        return m_pSockImpl->m_socket.close(e);
    } else {
        return true;
    }
}

int ServerSocketChannel::valid_ops() const {
    return SelectionKey::OpAccept;
}


}} // end namespace mercury::nio
