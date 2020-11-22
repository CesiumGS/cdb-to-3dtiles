#pragma once

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

inline std::vector<std::string> splitString(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> results;
    size_t last = 0;
    size_t next = 0;
    while ((next = str.find(delimiter, last)) != std::string::npos) {
        results.emplace_back(str.substr(last, next - last));
        last = next + 1;
    }

    results.emplace_back(str.substr(last));
    return results;
}
} // namespace CDBTo3DTiles
