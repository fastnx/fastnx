#include <algorithm>
#include <kernel/kauto_object.h>

namespace FastNx::Kernel {
    void KAutoObject::AddReference(const U64 value) {
        NX_ASSERT(!context.autorefs.contains(value));
        context.autorefs.emplace(value, this);
    }
    void KAutoObject::RemoveReference(U64 value) const {
        for (auto objsite{context.autorefs.begin()}; objsite != context.autorefs.end() && !value; ++objsite) {
            if (objsite->second == this)
                value = objsite->first;
        }

        NX_ASSERT(context.autorefs.contains(value));
        context.autorefs.erase(value);
    }

    KAutoObject *KAutoObject::GetObjectBy(const U64 value, const bool inclife) const {
        if (context.autorefs.contains(value)) {
            if (!inclife)
                return context.autorefs[value];
            auto *object{context.autorefs[value]};
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
