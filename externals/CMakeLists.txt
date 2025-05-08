cmake_minimum_required(VERSION 3.28)

add_subdirectory(boost)
add_library(boost_libs INTERFACE)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_options(boost_container PRIVATE "-Wno-aggressive-loop-optimizations")
endif ()
target_link_libraries(boost_libs INTERFACE Boost::regex Boost::align Boost::endian Boost::algorithm Boost::container Boost::program_options Boost::interprocess Boost::fiber)

set(ENABLE_PROGRAMS CACHE FORCE "Build Mbed TLS programs." OFF)
set(ENABLE_TESTING CACHE FORCE "Build Mbed TLS tests." OFF)
add_subdirectory(mbedtls)

set(FMT_INSTALL CACHE FORCE "Generate the install target." OFF)
add_subdirectory(fmt)

set(LZ4_BUILD_CLI CACHE FORCE "Build lz4 program" OFF)

add_subdirectory(lz4/build/cmake)


