cmake_minimum_required(VERSION 3.28)
target_include_directories(fastnx PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>
)

target_sources(fastnx PRIVATE
    loaders/nsp_es.cpp
    loaders/nsp_es.h
    loaders/app_types.cpp
    loaders/types.h
    loaders/application_directory.cpp
    loaders/application_directory.h
    loaders/gamefs.cpp
    loaders/gamefs.h
    loaders/nso_fmt.cpp
    loaders/nso_fmt.h
    horizon/switch_ns.cpp
    horizon/switch_ns.h
    horizon/key_set.cpp
    horizon/key_set.h
    horizon/nx_apps.cpp
    horizon/nx_apps.h
    horizon/process_loader.cpp
    horizon/process_loader.h
    common/main.cpp
    common/format.h
    common/bytes.h
    common/container.h
    common/traits.h
    common/types.h
    common/values.h
    common/exception.h
    common/async_logger.cpp
    common/async_logger.h
    common/memory.cpp
    common/memory.h
    common/allocators.cpp
    common/allocators.h
    debug/readonly_buffer.cpp
    debug/readonly_buffer.h
    debug/process_calltracer.cpp
    debug/process_calltracer.h
    device/processor.cpp
    device/syslimits.cpp
    device/pages.cpp
    device/memory.cpp
    device/memory.h
    device/capabilities.h
    kernel/types.h
    kernel/kernel.cpp
    kernel/kernel.h
    kernel/kauto_object.cpp
    kernel/kauto_object.h
    kernel/ksynchronization_object.cpp
    kernel/ksynchronization_object.h
    kernel/memory/k_memory.cpp
    kernel/memory/k_memory.h
    kernel/memory/kmemory_block_manager.cpp
    kernel/memory/kmemory_block_manager.h
    kernel/memory/kslab_heap.cpp
    kernel/memory/kslab_heap.h
    kernel/memory/ktls_pagemanager.cpp
    kernel/memory/ktls_pagemanager.h
    kernel/types/kprocess.cpp
    kernel/types/kprocess.h
    kernel/types/khandle_table.cpp
    kernel/types/khandle_table.h
    kernel/types/kthread.cpp
    kernel/types/kthread.h
    kernel/svc/types.h
    kernel/svc/memory.cpp
    kernel/svc/syscall.cpp
    kernel/svc/syscall.h
    kernel/threads/kscheduler.cpp
    kernel/threads/kscheduler.h
    runtime/spin_lock.cpp
    runtime/spin_lock.h
    runtime/entropy.cpp
    runtime/entropy.h
    runtime/lossless.cpp
    runtime/lossless.h
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
