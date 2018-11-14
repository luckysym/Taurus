#include <mars/matrix.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <unistd.h>

#include <iostream>

using namespace mars;
using namespace std;

class MatrixTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE( MatrixTest );
    CPPUNIT_TEST( testMatrixInit );
    CPPUNIT_TEST( testMatrixAdd );
    CPPUNIT_TEST( testMatrixMultiply );
    CPPUNIT_TEST_SUITE_END();
    
public:
    MatrixTest() {
        cout<<"construct"<<endl;
    }

    void setUp () {
    }

    void tearDown() {
    }

    // 测试创建一个服务端socket并执行监听
    void testMatrixInit() {
        DoubleMatrix mat0;
        CPPUNIT_ASSERT(mat0.rows() == 0);
        CPPUNIT_ASSERT(mat0.cols() == 0);

        DoubleMatrix mat1(1, 1, 2);
        CPPUNIT_ASSERT(mat1.rows() == 1);
        CPPUNIT_ASSERT(mat1.cols() == 1);
        CPPUNIT_ASSERT(mat1(0, 0) == 2);

        IntMatrix mat2(100, 200, 30);
        CPPUNIT_ASSERT(mat2.rows() == 100);
        CPPUNIT_ASSERT(mat2.cols() == 200);
        CPPUNIT_ASSERT(mat2(0, 0) == 30);
        CPPUNIT_ASSERT(mat2(99, 199) == 30);
        mat2(45, 45) = 68;
        CPPUNIT_ASSERT(mat2(45, 45) == 68);
    }

    void testMatrixAdd() {
        IntMatrix m1(4, 5, 9);
        IntMatrix m2(4, 5, 1);
        IntMatrix mr1 = m1 + m2;
        CPPUNIT_ASSERT(mr1(0, 0) == 10);
        mr1.print(std::cout);

        IntMatrix mr2 = m1 - m2;
        CPPUNIT_ASSERT(mr2(0, 0) == 8);
        mr2.print(std::cout);

        mr2 = m1 + 2;
        mr2.print(std::cout);
        mr2 = m2 - 1;
        mr2.print(std::cout);
    }

    void testMatrixMultiply() {
        int a1[2][3] = {{1, 2, 3}, {3, 4, 5} };
        int a2[3][4] = {{3, 2, 1, 4}, {4, 7, 5, 6}, {9, 3, 6, 8}};
        IntMatrix m1((int *)a1, 2, 3);
        IntMatrix m2((int *)a2, 3, 4);
        IntMatrix mr1 = m1 * m2;
        CPPUNIT_ASSERT(2 == mr1.rows());
        CPPUNIT_ASSERT(4 == mr1.cols());
        mr1.print(std::cout);

        mr1 = m1 * 2;
        mr1.print(std::cout);
    }
}; // end class MatrixTest

// CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SocketImpTest, "alltest" );
CPPUNIT_TEST_SUITE_REGISTRATION( MatrixTest );

int main(int argc, char **argv) 
{
    CppUnit::TextUi::TestRunner runner;
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
    runner.addTest(suite);
    runner.run();
    return 0;
}
