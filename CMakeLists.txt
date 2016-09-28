cmake_minimum_required (VERSION 2.6)
project (schrandr CXX)
option(BUILD_TESTS "Build test programs" OFF)

include(FindXCB_KDE.cmake)
    if(NOT XCB_FOUND)
        message(FATAL_ERROR "Failed to find XCB")
    endif(NOT XCB_FOUND)
    
include(FindJSONCpp.cmake)
    if(NOT JSONCPP_FOUND)
        message(FATAL_ERROR "Failed to find JSONCPP")
    endif(NOT JSONCPP_FOUND)
    
set(CORE_LIBS
    logging.cpp
    pid_file_holder.cpp
    cmdline_args.cpp
    xmanager.cpp
    monitor_setup.cpp
    edid.cpp
    config.cpp
    jsonadapter.cpp
    mode.cpp
    mode_change.cpp)
set(EXT_LIBS ${XCB_LIBRARIES} ${JSONCPP_LIBRARIES})

include_directories(${PROJECT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_library(SchrandrLibs ${CORE_LIBS})

add_executable(schrandr schrandr.cpp)

set_property(TARGET schrandr SchrandrLibs PROPERTY CXX_STANDARD 11)
set_property(TARGET schrandr SchrandrLibs PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(schrandr SchrandrLibs ${EXT_LIBS})

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
    add_subdirectory(tests)
endif(BUILD_TESTS)
