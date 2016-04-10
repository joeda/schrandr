cmake_minimum_required (VERSION 3.1.0)
project (schrandr CXX)

find_package(X11)
    if(NOT X11_FOUND)
        message(FATAL_ERROR "Failed to find X11")
    endif(NOT X11_FOUND)

include(FindXCB_KDE.cmake)
    if(NOT XCB_FOUND)
        message(FATAL_ERROR "Failed to find XCB")
    endif(NOT XCB_FOUND)

add_library(logging logging.cpp)
add_library(xmanager xmanager.cpp)
add_library(monitor_setup monitor_setup.cpp)
add_library(edid edid.cpp)
add_executable(schrandr schrandr.cpp)


set_property(TARGET schrandr logging xmanager monitor_setup edid PROPERTY CXX_STANDARD 11)
set_property(TARGET schrandr logging xmanager monitor_setup edid PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(schrandr ${X11_LIBRARIES} ${X11_Xrandr_LIB} logging ${XCB_LIBRARIES} xmanager monitor_setup edid)
