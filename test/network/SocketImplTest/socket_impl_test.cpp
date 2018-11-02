#include "../../../src/net/socket_impl.h"
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <unistd.h>

#include <iostream>

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

        std::string errinfo;
        CPPUNIT_ASSERT( ptrSocket->Create(Protocol::Tcp4, errinfo) );
        CPPUNIT_ASSERT( ptrSocket->Fd() > 0 );
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_CREATED );
        cout<<"socket created, fd: "<<ptrSocket->Fd()<<endl;
    }

    void CloseSocket(SocketImpl::Ptr &ptrSocket) {
        std::string errinfo;
        cout<<"Socket Closed: "<<ptrSocket->Fd()<<endl;
        CPPUNIT_ASSERT( ptrSocket->Close(errinfo));
        CPPUNIT_ASSERT( ptrSocket->Fd() == INVALID_SOCKET );
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_CLOSED );
    }

    void BindSocket (SocketImpl::Ptr &ptrSocket) {
        std::string errinfo;
        CPPUNIT_ASSERT( ptrSocket->Bind(Inet4Address(), 10024, errinfo));
    }
    void ListenSocket(SocketImpl::Ptr &ptrSocket) {
        std::string errinfo;
        CPPUNIT_ASSERT( ptrSocket->Listen(SOMAXCONN, errinfo));
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_OPEN );
    }
    void GetLocalAddress(SocketImpl::Ptr &ptrSocket) {
        std::string errinfo;
        const InetSocketAddress * paddr = ptrSocket->GetLocalAddress(errinfo);
        CPPUNIT_ASSERT(paddr);
        CPPUNIT_ASSERT(paddr->GetPort() == 10024);
        cout<<paddr->ToString()<<endl;
    }

    void ConnectSocketInvalid(SocketImpl::Ptr &ptrSocket) {
        std::string errinfo;
        Inet4Address address("10.10.10.10", errinfo);
        CPPUNIT_ASSERT( errinfo.empty() );
        CPPUNIT_ASSERT(!ptrSocket->Connect(address, 10024, errinfo));
        cout<<"Connect error: "<<errinfo<<endl;
        CPPUNIT_ASSERT( ptrSocket->State() == SocketImpl::SOCK_STATE_CREATED);
    }

    void ConnectSocketLocal22(SocketImpl::Ptr &ptrSocket) {
        std::string errinfo;
        Inet4Address address("127.0.0.1", errinfo);
        CPPUNIT_ASSERT( errinfo.empty() );
        CPPUNIT_ASSERT( ptrSocket->Connect(address, 22, errinfo));
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
        std::string errinfo;
        const InetSocketAddress * paddr = ptrSocket->GetLocalAddress(errinfo);
        CPPUNIT_ASSERT(paddr);
        cout<<paddr->ToString()<<endl;
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
