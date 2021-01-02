#pragma once
#include <cstdint>

template <typename T>
static inline constexpr T align(T offset, T alignment) {
    T mask = ~static_cast<T>(alignment-1);

    return (offset + (alignment-1)) & mask;
}
