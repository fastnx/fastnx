cmake_minimum_required(VERSION 3.30)
target_include_directories(fastnx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>
)

target_sources(fastnx PRIVATE
    fastnx.cpp
    loaders/nsp_es.cpp
    loaders/nsp_es.h
    loaders/app_types.cpp
    loaders/types.h
    horizon/switch_ns.cpp
    horizon/switch_ns.h
    common/container.h
    common/traits.h
    common/types.h
    device/processor.cpp
    device/capabilities.h
    device/syslimits.cpp
    fs_sys/types.h
    fs_sys/contains.cpp
    fs_sys/filters.cpp
    fs_sys/regex_file.cpp
    fs_sys/refs/editable_directory.cpp
    fs_sys/refs/editable_directory.h
    fs_sys/refs/buffered_file.cpp
    fs_sys/refs/buffered_file.h
    fs_sys/nx_fmt/partition_filesystem.cpp
    fs_sys/nx_fmt/partition_filesystem.h
)

add_subdirectory(core)
add_subdirectory(jit)
