cmake_minimum_required(VERSION 3.30)

add_subdirectory(boost)
add_library(boost_dep INTERFACE)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_options(boost_container PRIVATE -Wno-aggressive-loop-optimizations)
endif ()
target_link_libraries(boost_dep INTERFACE Boost::regex Boost::container)
