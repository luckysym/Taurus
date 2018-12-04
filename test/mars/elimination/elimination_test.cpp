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
        double a[3][4] = {{3, 2, 1, 4}, {4, 7, 5, 6}, {9, 3, 6, 8}};
        DoubleMatrix mat = DoubleMatrix((double *)a, 3, 4);
        mat.print(cout);

        Matop<double>::reduced_row_echelon(mat);
        mat.print(cout);

        Matop<double>::row_switch(mat, 0, 2);
        mat.print(cout);

        Matop<double>::row_multiply(mat, 0, 2);
        mat.print(cout);

        Matop<double>::row_multiply_add(mat, 1, 2, 2);
        mat.print(cout);

        Matop<double>::reduced_row_echelon(mat);
        mat.print(cout);

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
