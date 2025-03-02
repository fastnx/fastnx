cmake_minimum_required(VERSION 3.30)

target_sources(fastnx PRIVATE
    types.h
    contains.cpp
    fsio.cpp
    fullpath.cpp
    filters.cpp
    regex_file.cpp
    regex_file.h
    offset_file.cpp
    offset_file.h
    refs/editable_directory.cpp
    refs/editable_directory.h
    refs/buffered_file.cpp
    refs/buffered_file.h
    refs/huge_file.cpp
    refs/huge_file.h
    nx_fmt/partition_filesystem.cpp
    nx_fmt/partition_filesystem.h
    nx_fmt/submission_package.cpp
    nx_fmt/submission_package.h
)