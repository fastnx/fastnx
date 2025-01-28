cmake_minimum_required(VERSION 3.30)
target_include_directories(fastnx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>
)

target_sources(fastnx PRIVATE
    fastnx.cpp
    common/container.h
    fs_sys/types.h
    fs_sys/contains.cpp
    fs_sys/filters.cpp
    fs_sys/regex_file.cpp
    fs_sys/ssd/editable_directory.cpp
    fs_sys/ssd/editable_directory.h
)

add_subdirectory(core)
add_subdirectory(jit)
