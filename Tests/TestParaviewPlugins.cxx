#include <iostream>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/XmlOutputter.h>

#include "../ShowCVs/showCVs.h"
#include "../GMSHreader/GMSHreader.h"
#include "../GMSHwriter/GMSHwriter.h"

using namespace CppUnit;

class TestParaviewPlugins : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestParaviewPlugins);
  CPPUNIT_TEST(testNothing);
  CPPUNIT_TEST(testShowCVs);
  CPPUNIT_TEST(testGMSHreader);
  CPPUNIT_TEST(testGMSHwriter);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp(void);
  void tearDown(void);

protected:
  void testNothing(void);
  void testShowCVs(void);
  void testGMSHreader(void);
  void testGMSHwriter(void);
  
private:

};

void TestParaviewPlugins::testNothing(void)
{
  CPPUNIT_ASSERT(1==1);
}

void TestParaviewPlugins::testShowCVs(void)
{
  showCVs::showCVs* instance= showCVs::New();
  CPPUNIT_ASSERT(instance);
}

void TestParaviewPlugins::testGMSHreader(void)
{
  GMSHreader::GMSHreader* instance= GMSHreader::New();
  CPPUNIT_ASSERT(instance);
}

void TestParaviewPlugins::testGMSHwriter(void)
{
  GMSHwriter::GMSHwriter* instance= GMSHwriter::New();
  CPPUNIT_ASSERT(instance);
}


void TestParaviewPlugins::setUp(void) {};
void TestParaviewPlugins::tearDown(void) {};



CPPUNIT_TEST_SUITE_REGISTRATION( TestParaviewPlugins );

int main(int argc, char* argv[])
{    
  // informs test-listener about testresults
  CPPUNIT_NS::TestResult testresult;
  
  // register listener for collecting the test-results
  CPPUNIT_NS::TestResultCollector collectedresults;
  testresult.addListener (&collectedresults);
  
  // register listener for per-test progress output
  CPPUNIT_NS::BriefTestProgressListener progress;
  testresult.addListener (&progress);

  // insert test-suite at test-runner by registry

  CPPUNIT_NS::TestRunner testrunner;
  testrunner.addTest (CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest
		      ());
  testrunner.run(testresult);
  
  // output results in compiler-format
  CPPUNIT_NS::CompilerOutputter compileroutputter(&collectedresults, std::cerr
						  );
  compileroutputter.write ();

  // Output XML for Jenkins CPPunit plugin
  std::ofstream xmlFileOut("cxx_test_results.xml");
  XmlOutputter xmlOut(&collectedresults, xmlFileOut);
  xmlOut.write();

  // return 0 if tests were successful
  return collectedresults.wasSuccessful() ? 0 : 1;

 }
