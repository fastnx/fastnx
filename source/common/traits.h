#pragma once

#include <common/types.h>
namespace FastNx {
    template <typename T, U64 Size>
    concept TraitSizeMatch = sizeof(T) == Size && std::is_trivial_v<T>;
}
