cmake_minimum_required(VERSION 3.17)
project(basic_test C CXX)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/955c7f837efad184ec63e771c42542d37545eaef.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_definitions(-D_USE_MATH_DEFINES)

enable_testing()

add_executable(GCodeProgramGenerator_test GCodeProgramGenerator_test.cpp ${CMAKE_SOURCE_DIR}/../../src/cpp/GCodeProgramGenerator.cpp)

set(TEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../../src/cpp)
set(JSON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../../../json/single_include/nlohmann)
set(EIGEN_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../../../eigen)
set(GPR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../../../gpr/src)
set(CGAL_INCLUDE_DIR D:/Programy/CGAL/include)
set(BOOST_INCLUDE_DIR D:/Programy/boost_1_75_0)

#message(JSON_INCLUDE_DIR="${JSON_INCLUDE_DIR}")

target_include_directories(GCodeProgramGenerator_test PRIVATE
	${TEST_INCLUDE_DIR}
	${JSON_INCLUDE_DIR}
	${EIGEN_INCLUDE_DIR}
	${BOOST_INCLUDE_DIR}
	${GPR_INCLUDE_DIR}
	${CGAL_INCLUDE_DIR})

target_link_libraries(GCodeProgramGenerator_test gtest gtest_main)


include(GoogleTest)
gtest_discover_tests(GCodeProgramGenerator_test)

