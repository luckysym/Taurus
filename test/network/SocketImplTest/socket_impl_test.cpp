#include "../../../src/net/socket_impl.h"
#include "../../../src/net/socket_opt_impl.h"

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <unistd.h>

#include <iostream>

using namespace taurus;
using namespace taurus::net;
using namespace std;

class SocketImpTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE( SocketImpTest );
    CPPUNIT_TEST( testServerSocket );
    CPPUNIT_TEST( testClientSocketInvalid );
    CPPUNIT_TEST( testClientSocketValid );
    CPPUNIT_TEST_SUITE_END();

private:
    SocketImpl::Ptr m_ptrSocketImpl;
    std::string     m_err;
    
public:
    SocketImpTest() {
        cout<<"construct"<<endl;
    }

    void setUp () { 
        CreateSocket(m_ptrSocketImpl);
        // CreateClientSocket();
    }

    void tearDown() {
        // CloseClientSocket();
        CloseSocket(m_ptrSocketImpl);
    }

    // 测试创建一个服务端socket并执行监听
    void testServerSocket() {
        this->BindSocket(m_ptrSocketImpl);
        this->ListenSocket(m_ptrSocketImpl);
        this->GetLocalAddress(m_ptrSocketImpl);
    }

    // 测试创建一个客户端socket并连接一个无效地址
    void testClientSocketInvalid() {
        this->ConnectSocketInvalid(m_ptrSocketImpl);
    }

    void testClientSocketValid() {
        this->ConnectSocketLocal22(m_ptrSocketImpl);
        this->GetRemoteAddress22(m_ptrSocketImpl);
        this->GetClientLocalAddress(m_ptrSocketImpl);
        this->ShutdownSocket(m_ptrSocketImpl);
    }

protected:
    void CreateSocket(SocketImpl::Ptr &ptrSocket) {
        ptrSocket.reset(new SocketImpl());
        printf("Server Socket Ptr: %p\n", ptrSocket.get());
        CPPUNIT_ASSERT( ptrSocket != nullptr );

        ErrorInfo errinfo;
        CPPUNIT_ASSERT( ptrSocket->Create(Protocol::Tcp4, errinfo) );
        CPPUNIT_ASSERT( ptrSocket->Fd() > 0 );
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_CREATED );
        cout<<"socket created, fd: "<<ptrSocket->Fd()<<endl;

        // 设置socket reuse addr
        bool bopt;
        SocketOptReuseAddr optreuseaddr( ptrSocket->Fd() );
        CPPUNIT_ASSERT( optreuseaddr.Get(&bopt, m_err) );
        CPPUNIT_ASSERT( !bopt );
        CPPUNIT_ASSERT( optreuseaddr.Set(true, m_err) );
        CPPUNIT_ASSERT( optreuseaddr.Get(&bopt, m_err) );
        CPPUNIT_ASSERT( bopt );
        cout<<"socket opt reuseaddr: "<<bopt<<endl;
    }

    void SetSocketBuffer(SocketImpl::Ptr &ptrSocket) {

        // 获取和设置收发缓存
        int rcvbuf, sndbuf; 
        SocketOptRecvBuffer optrcvbuf(ptrSocket->Fd());
        SocketOptSendBuffer optsndbuf(ptrSocket->Fd());
        CPPUNIT_ASSERT( optrcvbuf.Get(&rcvbuf, m_err));
        CPPUNIT_ASSERT( optsndbuf.Get(&sndbuf, m_err));
        cout<<"default socket rcvbuf: "<<rcvbuf<<endl;
        cout<<"default socket sndbuf: "<<sndbuf<<endl;

        CPPUNIT_ASSERT( optrcvbuf.Set(4096, m_err));
        CPPUNIT_ASSERT( optsndbuf.Set(16384 * 2, m_err));

        CPPUNIT_ASSERT( optrcvbuf.Get(&rcvbuf, m_err));
        CPPUNIT_ASSERT( optsndbuf.Get(&sndbuf, m_err));
        CPPUNIT_ASSERT( rcvbuf == 4096);
        CPPUNIT_ASSERT( sndbuf == 16384 * 2);
    }

    void CloseSocket(SocketImpl::Ptr &ptrSocket) {
        ErrorInfo errinfo;
        cout<<"Socket Closed: "<<ptrSocket->Fd()<<endl;
        CPPUNIT_ASSERT( ptrSocket->Close(errinfo));
        CPPUNIT_ASSERT( ptrSocket->Fd() == INVALID_SOCKET );
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_CLOSED );
    }

    void BindSocket (SocketImpl::Ptr &ptrSocket) {
        ErrorInfo errinfo;
        CPPUNIT_ASSERT( ptrSocket->Bind("0.0.0.0", 10024, errinfo));
    }

    void ListenSocket(SocketImpl::Ptr &ptrSocket) {
        std::string errinfo;
        CPPUNIT_ASSERT( ptrSocket->Listen(SOMAXCONN, errinfo));
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_OPEN );
    }
    void GetLocalAddress(SocketImpl::Ptr &ptrSocket) {
        ErrorInfo errinfo;
        string addr = ptrSocket->GetLocalAddress(errinfo);
        CPPUNIT_ASSERT(errinfo.Code() == 0);
        CPPUNIT_ASSERT(!addr.empty());
        cout<<addr<<endl;
    }

    void ConnectSocketInvalid(SocketImpl::Ptr &ptrSocket) {
        ErrorInfo errinfo;
        Inet4Address address("127.0.0.1", errinfo);
        CPPUNIT_ASSERT( errinfo.Code() == 0 );

        std::string errstr;
        CPPUNIT_ASSERT(!ptrSocket->Connect(address, 10024, errstr));
        cout<<"Connect error: "<<errstr<<endl;
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_CREATED);
    }

    void ConnectSocketLocal22(SocketImpl::Ptr &ptrSocket) {
        ErrorInfo errinfo;
        Inet4Address address("127.0.0.1", errinfo);
        CPPUNIT_ASSERT( errinfo.Code() == 0 );

        std::string errstr;
        CPPUNIT_ASSERT( ptrSocket->Connect(address, 22, errstr));
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_OPEN);
    }

    void GetRemoteAddress22(SocketImpl::Ptr &ptrSocket) {
        std::string errinfo;
        const InetSocketAddress * paddr = ptrSocket->GetRemoteAddress(errinfo);
        CPPUNIT_ASSERT(paddr);
        CPPUNIT_ASSERT(paddr->GetPort() == 22);
        cout<<paddr->ToString()<<endl;
    }

    void GetClientLocalAddress(SocketImpl::Ptr &ptrSocket) {
        ErrorInfo errinfo;
        string addr = ptrSocket->GetLocalAddress(errinfo);
        CPPUNIT_ASSERT(errinfo.Code() == 0);
        CPPUNIT_ASSERT(!addr.empty());
        cout<<addr<<endl;
    }

    void ShutdownSocket(SocketImpl::Ptr &ptrSocket) {
        std::string errinfo;
        CPPUNIT_ASSERT( ptrSocket->ShutdownInput(errinfo) );
        CPPUNIT_ASSERT( ptrSocket->ShutdownOutput(errinfo) );
    }
}; // end class SocketImpTest



// CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SocketImpTest, "alltest" );
CPPUNIT_TEST_SUITE_REGISTRATION( SocketImpTest );

int main(int argc, char **argv) 
{
    CppUnit::TextUi::TestRunner runner;
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
    runner.addTest(suite);
    runner.run();
    return 0;
}
