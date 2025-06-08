#pragma once

#include <cstring>
#include <boost/align/align_down.hpp>
#include <boost/align/detail/is_aligned.hpp>
#include <sys/mman.h>

#include <common/types.h>
namespace FastNx {
    void *Allocate(U64 size);
    void Free(void *pointer, U64 size);

    template<typename T>
    class SwappableAllocator {
    public:
        using value_type = T;
        using pointer = T*;
        using size_type = std::size_t;

        SwappableAllocator() = default;
        explicit SwappableAllocator(const SwappableAllocator&) {}

        static pointer allocate(const size_type size) {
            return static_cast<T *>(Allocate(size * sizeof(T)));
        }
        static void deallocate(const pointer pointer, const size_type size) {
            Free(pointer, size * sizeof(T));
        }
    };

    template<typename T> requires(boost::alignment::detail::is_alignment(sizeof(T)))
    class SwappableVector {
        public:
        SwappableVector() = default;
        ~SwappableVector() {
            SwappableAllocator<T>::deallocate(pointer, size * sizeof(T));
        }

        static bool Initialized(void * begin) {
            U16 result{};
            auto *objects{boost::alignment::align_down(begin, PageSize)};
            if (mincore(objects, PageSize, reinterpret_cast<U8 *>(&result)) == 0)
                return result;
            return {};
        }
        void Construct(void * begin) {
            for (auto object: std::span{static_cast<T *>(begin), PageSize / sizeof(T)})
                if constexpr (std::is_default_constructible_v<T>)
                    std::construct_at(&object);
                else if constexpr (std::is_trivial_v<T>)
                    std::memset(&object, 0, sizeof(T));
        }

        void resize(const U64 setsize) {
            pointer = SwappableAllocator<T>::allocate(sizeof(T) * setsize);
            for (auto *base{reinterpret_cast<U8 *>(pointer)}; ; base += PageSize) {
                if (Initialized(base))
                    Construct(base);
                else break;
            }

            size = setsize;
        }
        T & operator[](const U64 index) {
            if (index >= size)
                throw std::bad_alloc{};
            if (!Initialized(&pointer[index]))
                Construct(boost::alignment::align_down(&pointer[index], PageSize));
            return pointer[index];
        }

        const T & front() {
            if (!pointer)
                throw std::bad_alloc{};
            if (!Initialized(pointer))
                Construct(pointer);
            return *pointer;
        }

        U64 size{};
        T *pointer{};
    };
}