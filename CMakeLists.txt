cmake_minimum_required (VERSION 3.1.0)
project (schrandr CXX)

find_package(X11)

    if(NOT X11_FOUND)
        message(FATAL_ERROR "Failed to find X11")
    endif(NOT X11_FOUND)

add_executable(schrandr schrandr.cpp)

set_property(TARGET schrandr PROPERTY CXX_STANDARD 11)
set_property(TARGET schrandr PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(schrandr ${X11_LIBRARIES} ${X11_Xrandr_LIB})