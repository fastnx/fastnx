#include <algorithm>
#include <kernel/kauto_object.h>

namespace FastNx::Kernel {
    void KAutoObject::AddReference(const U64 value) {
        if (!kernel.autorefs.contains(value))
            kernel.autorefs.emplace(value, this);
    }
    void KAutoObject::RemoveReference(U64 value) const {
        for (auto objsite{kernel.autorefs.begin()}; objsite != kernel.autorefs.end() && !value; ++objsite) {
            if (objsite->second == this)
                value = objsite->first;
        }
        kernel.autorefs.erase(value);
    }

    KAutoObject *KAutoObject::GetObjectBy(const U64 value, const bool inclife) const {
        if (kernel.autorefs.contains(value)) {
            if (!inclife)
                return kernel.autorefs[value];
            auto *object{kernel.autorefs[value]};
            object->IncreaseLifetime();
            return object;
        }
        return nullptr;
    }
    void KAutoObject::IncreaseLifetime() {
        NX_ASSERT(counter >= 0);
        counter.fetch_add(1, std::memory_order_acq_rel);
    }
    void KAutoObject::DeteriorateLifetime() {
        counter.fetch_sub(1, std::memory_order_seq_cst);
        if (counter> 0)
            return;

        RemoveReference(0);
        Destroyed();
    }
}
