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
    // 声明一个TestSuite
    CPPUNIT_TEST_SUITE( SocketImpTest );
    // 添加测试用例到TestSuite, 定义新的测试用例需要在这儿声明一下
    CPPUNIT_TEST( testServerSocket );
    // TestSuite声明完成
    CPPUNIT_TEST_SUITE_END();

private:
    SocketImpl::Ptr m_ptrSockImpl;

public:
    SocketImpTest() {
        cout<<"construct"<<endl;
    }

    void setUp () { 
        CreateSocket();
    }
    void tearDown() {
        CloseSocket();
    }

    void testServerSocket() {
        this->BindSocket();
        this->ListenSocket();
    }

protected:
    void CreateSocket() {
        m_ptrSockImpl = std::make_shared<SocketImpl>();
        printf("%p\n", m_ptrSockImpl.get());
        CPPUNIT_ASSERT( m_ptrSockImpl != nullptr );

        std::string errinfo;
        CPPUNIT_ASSERT( m_ptrSockImpl->Create(Protocol::Tcp4, errinfo) );
        CPPUNIT_ASSERT( m_ptrSockImpl->Fd() > 0 );
        CPPUNIT_ASSERT( m_ptrSockImpl->State() == SocketImpl::SOCK_STATE_CREATED );
        cout<<"socket created, fd: "<<m_ptrSockImpl->Fd()<<endl;
    }
    void CloseSocket() {
        std::string errinfo;
        CPPUNIT_ASSERT( m_ptrSockImpl->Close(errinfo));
        CPPUNIT_ASSERT( m_ptrSockImpl->Fd() == INVALID_SOCKET );
        CPPUNIT_ASSERT( m_ptrSockImpl->State() == SocketImpl::SOCK_STATE_CLOSED );
    }
    void BindSocket () {
        std::string errinfo;
        CPPUNIT_ASSERT( m_ptrSockImpl->Bind(Inet4Address(), 10024, errinfo));
    }
    void ListenSocket() {
        std::string errinfo;
        CPPUNIT_ASSERT( m_ptrSockImpl->Listen(SOMAXCONN, errinfo));
        CPPUNIT_ASSERT( m_ptrSockImpl->State() == SocketImpl::SOCK_STATE_OPEN );
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
