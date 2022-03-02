#pragma once

#include "Utility.h"
#include <filesystem>
#include <optional>
#include <string>

namespace CDBTo3DTiles {

class CDBGeoCell
{
public:
    CDBGeoCell(int latitude, int longitude);

    inline int getLatitude() const noexcept { return m_latitude; }

    inline int getLongitude() const noexcept { return m_longitude; }

    inline int getZone() const noexcept { return m_zone; }

    int getLongitudeExtentInDegree() const;

    int getLatitudeExtentInDegree() const noexcept;

    std::string getLatitudeDirectoryName() const noexcept;

    std::string getLongitudeDirectoryName() const noexcept;

    const std::filesystem::path &getRelativePath() const noexcept;

    static std::optional<int> parseLatFromFilename(const std::string &filename);

    static std::optional<int> parseLongFromFilename(const std::string &filename);

private:
    friend bool operator==(const CDBGeoCell &lhs, const CDBGeoCell &rhs) noexcept;

    std::filesystem::path m_path;
    int m_latitude;
    int m_longitude;
    int m_zone;
};

} // namespace CDBTo3DTiles

namespace std {
template<>
struct hash<CDBTo3DTiles::CDBGeoCell>
{
    size_t operator()(CDBTo3DTiles::CDBGeoCell const &geoCell) const noexcept
    {
        size_t seed = 0;
        CDBTo3DTiles::hashCombine(seed, geoCell.getLongitude());
        CDBTo3DTiles::hashCombine(seed, geoCell.getLatitude());
        return seed;
    }
};
} // namespace std
