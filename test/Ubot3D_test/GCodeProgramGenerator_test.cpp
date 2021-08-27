//win32:CONFIG(release, debug|release): LIBS += -LD:/Projects/googletest/build/lib/Release -lgmock -lgtest -lgmock_main -lgtest_main
#include <GCodeProgramGenerator.h>

#include <gtest/gtest.h>

class GCodeProgramGeneratorTest : public ::testing::Test {
 protected:
  void SetUp() override {
  }

  // void TearDown() override {}

//  GCodeProgramGenerator g();
};

namespace Slicer
{

TEST_F(GCodeProgramGeneratorTest, GoogleTestSuiteWorks)
{
	ASSERT_EQ(2+2, 4);
}

}
