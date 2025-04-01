cmake_minimum_required(VERSION 3.28)
target_include_directories(fastnx PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>
)

target_sources(fastnx PRIVATE
    common/main.cpp
    loaders/nsp_es.cpp
    loaders/nsp_es.h
    loaders/app_types.cpp
    loaders/types.h
    loaders/homefs.cpp
    loaders/homefs.h
    loaders/gamefs.cpp
    loaders/gamefs.h
    horizon/switch_ns.cpp
    horizon/switch_ns.h
    horizon/key_set.cpp
    horizon/key_set.h
    common/bytes.h
    common/container.h
    common/traits.h
    common/types.h
    common/values.h
    common/exception.h
    common/async_logger.cpp
    common/async_logger.h
    device/processor.cpp
    device/syslimits.cpp
    device/pages.cpp
    device/memory.cpp
    device/memory.h
    device/capabilities.h
    kernel/kernel.cpp
    kernel/kernel.h
    kernel/pid.cpp
    kernel/kauto_object.cpp
    kernel/kauto_object.h
    kernel/dram/device_lpddr4.cpp
    kernel/dram/device_lpddr4.h
    kernel/types/kprocess.cpp
    kernel/types/kprocess.h
    crypto/integrity.cpp
    crypto/checksum.cpp
    crypto/checksum.h
    crypto/safe_aes.cpp
    crypto/safe_aes.h
    crypto/pki_nca.cpp
    crypto/ticket.cpp
    crypto/ticket.h
    crypto/types.h
)

add_subdirectory(core)
add_subdirectory(jit)
add_subdirectory(fs_sys)

# https://switchbrew.org/wiki/Glossary
