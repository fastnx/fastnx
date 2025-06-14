project(FastNx)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Wall -Werror -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -s -flto")

add_executable(fastnx)

add_subdirectory(externals)
add_subdirectory(source)

target_link_libraries(fastnx PRIVATE
    Boost::regex Boost::align Boost::endian Boost::algorithm Boost::container Boost::program_options Boost::interprocess Boost::fiber
    MbedTLS::mbedcrypto
    fmt::fmt
    lz4
    pugixml::pugixml
    dynarmic
    tsl::robin_map
)
