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
    loaders/application_directory.cpp
    loaders/application_directory.h
    loaders/gamefs.cpp
    loaders/gamefs.h
    loaders/nsoexe.cpp
    loaders/nsoexe.h
    horizon/switch_ns.cpp
    horizon/switch_ns.h
    horizon/key_set.cpp
    horizon/key_set.h
    horizon/nx_apps.cpp
    horizon/nx_apps.h
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
    kernel/types.h
    kernel/kernel.cpp
    kernel/kernel.h
    kernel/routines.cpp
    kernel/kauto_object.cpp
    kernel/kauto_object.h
    kernel/ksynchronization_object.cpp
    kernel/ksynchronization_object.h
    kernel/memory/device_lpddr4.cpp
    kernel/memory/device_lpddr4.h
    kernel/memory/kmemory_block_manager.cpp
    kernel/memory/kmemory_block_manager.h
    kernel/memory/kslab_heap.cpp
    kernel/memory/kslab_heap.h
    kernel/memory/kpage_table.cpp
    kernel/memory/kpage_table.h
    kernel/types/kprocess.cpp
    kernel/types/kprocess.h
    kernel/types/kprocess.cpp
    kernel/types/kprocess.h
    runtime/spin_lock.cpp
    runtime/spin_lock.h
    runtime/entropy.cpp
    runtime/entropy.h
    crypto/integrity.cpp
    crypto/checksum.cpp
    crypto/checksum.h
    crypto/aes_cipher.cpp
    crypto/aes_cipher.h
    crypto/pki_nca.cpp
    crypto/ticket.cpp
    crypto/ticket.h
    crypto/types.h
)

add_subdirectory(core)
add_subdirectory(jit)
add_subdirectory(fs_sys)

# https://switchbrew.org/wiki/Glossary
