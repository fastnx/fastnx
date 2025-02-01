cmake_minimum_required(VERSION 3.30)
project(FastNx)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

add_executable(fastnx)

add_subdirectory(externals)
add_subdirectory(source)
target_link_libraries(fastnx PRIVATE boost_dep)
