#pragma once

#define TRAIT_SIZE_MATCH(_type, _size) static_assert(sizeof(_type) == _size && std::is_trivial_v<_type>)
