#include <kernel/ksynchronization_object.h>
namespace FastNx::Kernel {
    void KSynchronizationObject::Signal() const {
        for ([[maybe_unused]] const auto &thread: thrSynclist) {
        }
    }
}
