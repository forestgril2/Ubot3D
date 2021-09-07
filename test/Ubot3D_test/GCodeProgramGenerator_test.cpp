//win32:CONFIG(release, debug|release): LIBS += -LD:/Projects/googletest/build/lib/Release -lgmock -lgtest -lgmock_main -lgtest_main
#include <GCodeProgramGenerator.h>

#include <gtest/gtest.h>
#include <gcode_program.h>


class GCodeProgramGeneratorBasicTest : public ::testing::Test
 {
 protected:
  void SetUp() override {
  }

  // void TearDown() override {}

//  GCodeProgramGenerator g();
  Slicer::SolidSurfaceModels models;
  Slicer::ExtrusionParamSets params;
};

namespace Slicer
{

TEST_F(GCodeProgramGeneratorBasicTest, TakesSolidSurfaceModelsAndExtrusionParamSetsAsInput)
{
	GCodeProgramGenerator(models, params);
}

TEST_F(GCodeProgramGeneratorBasicTest, ReturnsEmptyProgramForEmptyInput)
{
	ASSERT_EQ(GCodeProgramGenerator(models, params).getProgram()->num_blocks(), 0u);
}

}
