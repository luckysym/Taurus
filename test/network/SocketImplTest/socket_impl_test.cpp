#include "../../../src/net/socket_impl.h"
#include "../../../src/net/socket_opt_impl.h"
#include <mercury/nio/buffer.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <unistd.h>

#include <iostream>

using namespace mercury;
using namespace mercury::net;
using namespace std;

class SocketImpTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE( SocketImpTest );
    CPPUNIT_TEST( testServerSocket );
    CPPUNIT_TEST( testClientSocketInvalid );
    CPPUNIT_TEST( testClientSocketValid );
    CPPUNIT_TEST_SUITE_END();

private:
    SocketImpl::Ptr m_ptrSocketImpl;
    RuntimeError    m_err;
    
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

        RuntimeError errinfo;
        CPPUNIT_ASSERT( ptrSocket->Create(Protocol::Tcp4, errinfo) );
        CPPUNIT_ASSERT( ptrSocket->Fd() > 0 );
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_CREATED );
        cout<<"socket created, fd: "<<ptrSocket->Fd()<<endl;

        // 设置socket reuse addr
        int bopt;
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
        RuntimeError errinfo;
        cout<<"Socket Closed: "<<ptrSocket->Fd()<<endl;
        CPPUNIT_ASSERT( ptrSocket->Close(errinfo));
        CPPUNIT_ASSERT( ptrSocket->Fd() == INVALID_SOCKET );
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_CLOSED );
    }

    void BindSocket (SocketImpl::Ptr &ptrSocket) {
        RuntimeError errinfo;
        CPPUNIT_ASSERT( ptrSocket->Bind("0.0.0.0", 10024, errinfo));
    }

    void ListenSocket(SocketImpl::Ptr &ptrSocket) {
        RuntimeError errinfo;
        CPPUNIT_ASSERT( ptrSocket->Listen(SOMAXCONN, errinfo));
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_OPEN );
    }
    void GetLocalAddress(SocketImpl::Ptr &ptrSocket) {
        RuntimeError errinfo;
        string addr = ptrSocket->GetLocalAddress(errinfo);
        CPPUNIT_ASSERT(errinfo.code() == 0);
        CPPUNIT_ASSERT(!addr.empty());
        cout<<addr<<endl;
    }

    void ConnectSocketInvalid(SocketImpl::Ptr &ptrSocket) {
        RuntimeError errinfo;
        CPPUNIT_ASSERT(!ptrSocket->Connect("127.0.0.1", 10024, errinfo));
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_CREATED);
    }

    void ConnectSocketLocal22(SocketImpl::Ptr &ptrSocket) {
        RuntimeError errinfo;

        CPPUNIT_ASSERT( ptrSocket->Connect("127.0.0.1", 22, errinfo));
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_OPEN);
    }

    void GetRemoteAddress22(SocketImpl::Ptr &ptrSocket) {
        RuntimeError e1;
        std::string addr = ptrSocket->GetRemoteAddress(e1);
        CPPUNIT_ASSERT(!addr.empty());

        RuntimeError e2;
        int port = ptrSocket->GetRemotePort(e2);
        CPPUNIT_ASSERT(port == 22);
    }

    void GetClientLocalAddress(SocketImpl::Ptr &ptrSocket) {
        RuntimeError errinfo;
        string addr = ptrSocket->GetLocalAddress(errinfo);
        CPPUNIT_ASSERT(errinfo.code() == 0);
        CPPUNIT_ASSERT(!addr.empty());
        cout<<addr<<endl;
    }

    void ShutdownSocket(SocketImpl::Ptr &ptrSocket) {
        RuntimeError errinfo;
        CPPUNIT_ASSERT( ptrSocket->ShutdownInput(errinfo) );
        CPPUNIT_ASSERT( ptrSocket->ShutdownOutput(errinfo) );
    }
}; // end class SocketImpTest

class URL_Test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE( URL_Test );
    CPPUNIT_TEST( testURL );
    CPPUNIT_TEST_SUITE_END(); 
    
public:
    void setUp () { }
    void tearDown() { }

    void testURL() {
        URL url;
        CPPUNIT_ASSERT(url.schema()[0] == '\0');
        CPPUNIT_ASSERT(url.host()[0] == '\0');
        CPPUNIT_ASSERT(url.port() == -1);

        URL url1("http", "192.168.1.1", 8080);
        CPPUNIT_ASSERT(0 == strcmp("http", url1.schema()));
        CPPUNIT_ASSERT(0 == strcmp("192.168.1.1", url1.host()));
        CPPUNIT_ASSERT(url1.port() == 8080);
        cout<<url1.str()<<endl;

        RuntimeError e;
        URL url2("http://192.168.1.2:8080", e);
        CPPUNIT_ASSERT(0 == e.code());
        CPPUNIT_ASSERT(0 == strcmp("http", url2.schema()));
        CPPUNIT_ASSERT(0 == strcmp("192.168.1.2", url2.host()));
        CPPUNIT_ASSERT(url2.port() == 8080);
        cout<<url2.str()<<endl;

        URL url3("http", "fe80::9069:38e5:2d7:8ed3", -1);
        CPPUNIT_ASSERT(0 == strcmp("http", url3.schema()));
        CPPUNIT_ASSERT(0 == strcmp("fe80::9069:38e5:2d7:8ed3", url3.host()));
        CPPUNIT_ASSERT(url3.port() == -1);
        cout<<url3.str()<<endl;

        URL url4("http", "[fe80::9069:38e5:2d7:8ed4]", 0);
        CPPUNIT_ASSERT(0 == strcmp("http", url4.schema()));
        CPPUNIT_ASSERT(0 == strcmp("[fe80::9069:38e5:2d7:8ed4]", url4.host()));
        CPPUNIT_ASSERT(url4.port() == 0);
        cout<<url4.str()<<endl;

        e.clear();
        URL url5("http://[fe80::9069:38e5:2d7:8ed4]:9090", e);
        CPPUNIT_ASSERT(0 == strcmp("http", url5.schema()));
        CPPUNIT_ASSERT(0 == strcmp("[fe80::9069:38e5:2d7:8ed4]", url5.host()));
        CPPUNIT_ASSERT(url5.port() == 9090);
        cout<<url5.str()<<endl;
    }
};

// CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SocketImpTest, "alltest" );
// CPPUNIT_TEST_SUITE_REGISTRATION( SocketImpTest );
CPPUNIT_TEST_SUITE_REGISTRATION( URL_Test );

int main(int argc, char **argv) 
{
    CppUnit::TextUi::TestRunner runner;
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
    runner.addTest(suite);
    runner.run();
    return 0;
}
