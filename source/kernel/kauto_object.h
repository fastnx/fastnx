#pragma once

#include <atomic>

#include <common/types.h>
#include <kernel/kernel.h>
namespace FastNx::Kernel {
    // 6.0.0
    enum class KObjectType {
        None,
        KProcess
    };

    // This class stores the runtime type information of the object, including a reference count associated with it
    class KAutoObject {
    public:
        virtual ~KAutoObject() = default;

        explicit KAutoObject(const KObjectType _type, Kernel &_kernel) : type(_type), kernel(_kernel) {}

        void AddReference(U64 value);
        void RemoveReference(U64 value) const;
        KAutoObject *GetObjectBy(U64 value, bool inclife = false) const;

        void IncreaseLifetime();
        void DeteriorateLifetime();

        KObjectType type;
    protected:
        std::atomic<U32> counter{};
        void *treenode{}; // < Intrusive red-black tree node
        Kernel &kernel;

        virtual void Destroyed() = 0;
    };
}
