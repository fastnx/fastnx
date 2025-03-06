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
    loaders/homefs.cpp
    loaders/homefs.h
    loaders/romdir_mkfs.cpp
    loaders/romdir_mkfs.h
    horizon/switch_ns.cpp
    horizon/switch_ns.h
    horizon/key_set.cpp
    horizon/key_set.h
    common/container.h
    common/traits.h
    common/types.h
    common/values.h
    common/exception.h
    device/processor.cpp
    device/syslimits.cpp
    device/pages.cpp
    device/memory.cpp
    device/memory.h
    device/capabilities.h
    kernel/kernelctx.cpp
    kernel/kernelctx.h
    crypto/integrity.cpp
    crypto/hashsum.cpp
    crypto/hashsum.h
    crypto/safe_aes.cpp
    crypto/safe_aes.h
    crypto/types.h
)

add_subdirectory(core)
add_subdirectory(jit)
add_subdirectory(fs_sys)

# https://switchbrew.org/wiki/Glossary
