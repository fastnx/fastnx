cmake_minimum_required(VERSION 3.30)
project(Fastnx)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(fastnx)

add_subdirectory(externals)
add_subdirectory(source)
target_link_libraries(fastnx PRIVATE boost_dep)
