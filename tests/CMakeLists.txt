 
cmake_minimum_required(VERSION 2.6)
# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${PROJECT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
 
# Link runTests with what we want to test and the GTest and pthread library
add_executable(runTests tests.cpp)
add_test(TestTest runTests)

set_property(TARGET runTests SchrandrLibs PROPERTY CXX_STANDARD 11)
set_property(TARGET runTests SchrandrLibs PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(runTests SchrandrLibs ${EXT_LIBS} ${GTEST_LIBRARIES})
