cmake_minimum_required(VERSION 3.28)

target_sources(fastnx PRIVATE
    types.h
    contains.cpp
    fsio.cpp
    fullpath.cpp
    filters.cpp
    match.cpp
    regex_file.cpp
    regex_file.h
    offset_file.cpp
    offset_file.h
    xts_file.cpp
    xts_file.h
    standard_file.cpp
    standard_file.h
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
    nx_fmt/content_archive.cpp
    nx_fmt/content_archive.h
)