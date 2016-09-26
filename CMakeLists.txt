cmake_minimum_required (VERSION 3.1.0)
project (schrandr CXX)

include(FindXCB_KDE.cmake)
    if(NOT XCB_FOUND)
        message(FATAL_ERROR "Failed to find XCB")
    endif(NOT XCB_FOUND)
    
include(FindJSONCpp.cmake)
    if(NOT JSONCPP_FOUND)
        message(FATAL_ERROR "Failed to find JSONCPP")
    endif(NOT JSONCPP_FOUND)
    
set(CORE_LIBS logging xmanager monitor_setup edid config jsonadapter mode mode_change)
set(EXT_LIBS ${XCB_LIBRARIES} ${JSONCPP_LIBRARIES})

foreach(LIB ${CORE_LIBS})
    add_library(${LIB} "${LIB}.cpp")
endforeach(LIB)

add_executable(schrandr schrandr.cpp)


set_property(TARGET schrandr ${CORE_LIBS} PROPERTY CXX_STANDARD 11)
set_property(TARGET schrandr ${CORE_LIBS} PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(schrandr ${CORE_LIBS} ${EXT_LIBS})
