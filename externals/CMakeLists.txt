cmake_minimum_required(VERSION 3.28)

add_subdirectory(boost)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_options(boost_container PRIVATE "-Wno-aggressive-loop-optimizations")
endif ()

set(ENABLE_PROGRAMS CACHE FORCE "Build Mbed TLS programs." OFF)
set(ENABLE_TESTING CACHE FORCE "Build Mbed TLS tests." OFF)
add_subdirectory(mbedtls)

set(FMT_INSTALL CACHE FORCE "Generate the install target." OFF)
add_subdirectory(fmt)

set(LZ4_BUILD_CLI CACHE FORCE "Build lz4 program" OFF)

add_subdirectory(lz4/build/cmake)

set(PUGIXML_INSTALL CACHE FORCE "Enable installation rules" OFF)
add_subdirectory(pugixml)

if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(DYNARMIC_IGNORE_ASSERTS ON)
    set(DYNARMIC_FRONTENDS "A64")
    add_subdirectory(dynarmic)
    target_compile_options(dynarmic PRIVATE -Wno-stack-usage)
endif ()
