#pragma once

#include <common/types.h>
namespace FastNx {
    void *Allocate(U64 size);
    void Free(void *pointer, U64 size);

    template <typename T>
    class SwappableAllocator {
    public:
        using value_type = T;
        using pointer = T*;
        using size_type = std::size_t;

        SwappableAllocator() = default;
        template <typename U>
        explicit SwappableAllocator(const SwappableAllocator<U>&) {}

        static pointer allocate(const size_type size) {
            return static_cast<T *>(Allocate(size * sizeof(T)));
        }
        static void deallocate(const pointer pointer, const size_type size) {
            Free(pointer, size * sizeof(T));
        }
    };

    template<typename T>
    class SwappableVector {
        public:
        SwappableVector() = default;
        ~SwappableVector() {
            SwappableAllocator<T>::deallocate(pointer, size * sizeof(T));
        }

        void resize(const U64 setsize) {
            pointer = SwappableAllocator<T>::allocate(sizeof(T) * setsize);
            size = setsize;
        }
        T & operator[](const U64 index) {
            if (index >= size)
                throw std::bad_alloc{};
            return pointer[index];
        }
        const T & operator[](const U64 index) const {
            if (index >= size)
                throw std::out_of_range{"Index out of range"};
            return pointer[index];
        }

        const T & front() const {
            if (!pointer)
                throw std::bad_alloc{};
            return *pointer;
        }

        U64 size{};
        T *pointer{};
    };
}