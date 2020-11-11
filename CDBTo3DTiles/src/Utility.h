#pragma once

#include "Scene.h"
#include <cstddef>
#include <functional>

namespace CDBTo3DTiles {
template<class T>
inline void hashCombine(size_t &seed, const T &v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename T>
inline std::string toStringWithZeroPadding(size_t numOfZeroes, T num)
{
    std::string numStr = std::to_string(num);
    if (numOfZeroes > numStr.size()) {
        return std::string(numOfZeroes - numStr.size(), '0') + numStr;
    }

    return numStr;
}

inline size_t roundUp(size_t numToRound, size_t multiple)
{
    return ((numToRound + multiple - 1) / multiple) * multiple;
}
} // namespace CDBTo3DTiles
