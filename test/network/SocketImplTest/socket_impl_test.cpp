#include "../../../src/net/socket_impl.h"
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <unistd.h>

using namespace taurus::net;

class SocketImpTest : public CppUnit::TestFixture {
    // 声明一个TestSuite
    CPPUNIT_TEST_SUITE( SocketImpTest );
    // 添加测试用例到TestSuite, 定义新的测试用例需要在这儿声明一下
    CPPUNIT_TEST( testCreate );
    // TestSuite声明完成
    CPPUNIT_TEST_SUITE_END();

private:
    SocketImpl::Ptr m_ptrSockImpl;

public:
    SocketImpTest() {}

    void setUp () {}
    void tearDown() {}
    void testCreate() { 
        m_ptrSockImpl = std::make_shared<SocketImpl>();
        printf("%p\n", m_ptrSockImpl.get());
        CPPUNIT_ASSERT( m_ptrSockImpl != nullptr );
    }
  // 可以添加新的测试函数
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

