cmake_minimum_required(VERSION 3.28)

target_sources(fastnx PRIVATE
    types.h
    contains.cpp
    fsio.cpp
    fullpath.cpp
    filters.cpp
    match.cpp
    cnmt.cpp
    cnmt.h
    linkable_directory.cpp
    linkable_directory.h
    vfs/regex_file.cpp
    vfs/regex_file.h
    vfs/offset_file.cpp
    vfs/offset_file.h
    vfs/standard_file.cpp
    vfs/standard_file.h
    aes/xts_file.cpp
    aes/xts_file.h
    aes/ctr_file.cpp
    aes/ctr_file.h
    refs/directory_file_access.cpp
    refs/directory_file_access.h
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
    nx_fmt/romfs.cpp
    nx_fmt/romfs.h
)