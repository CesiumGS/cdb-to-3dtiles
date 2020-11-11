#include "CDBGeoCell.h"
#include "CDB.h"

namespace CDBTo3DTiles {

static int getZoneFromLatitude(int latitude);

CDBGeoCell::CDBGeoCell(int latitude, int longitude)
{
    if (latitude < -90 || latitude > 90) {
        throw std::invalid_argument("Latitude " + std::to_string(latitude) + " is out of range");
    }

    if (longitude < -180 || latitude > 180) {
        throw std::invalid_argument("Longitude " + std::to_string(longitude) + " is out of range");
    }

    m_latitude = latitude;
    m_longitude = longitude;
    m_zone = getZoneFromLatitude(latitude);
    m_path = CDB::TILES / getLatitudeDirectoryName() / getLongitudeDirectoryName();
}

int CDBGeoCell::getLongitudeExtentInDegree() const
{
    if (m_latitude >= 89 && m_latitude < 90) {
        return 12;
    }

    if (m_latitude >= 80 && m_latitude < 89) {
        return 6;
    }

    if (m_latitude >= 75 && m_latitude < 80) {
        return 4;
    }

    if (m_latitude >= 70 && m_latitude < 75) {
        return 3;
    }

    if (m_latitude >= 50 && m_latitude < 70) {
        return 2;
    }

    if (m_latitude >= -50 && m_latitude < 50) {
        return 1;
    }

    if (m_latitude >= -70 && m_latitude < -50) {
        return 2;
    }

    if (m_latitude >= -75 && m_latitude < -70) {
        return 3;
    }

    if (m_latitude >= -80 && m_latitude < -75) {
        return 4;
    }

    if (m_latitude >= -89 && m_latitude < -80) {
        return 6;
    }

    if (m_latitude >= -90 && m_latitude < -89) {
        return 12;
    }

    throw std::invalid_argument("Latitude " + std::to_string(m_latitude) + " out of bound");
}

bool operator==(const CDBGeoCell &lhs, const CDBGeoCell &rhs) noexcept
{
    return lhs.m_latitude == rhs.m_latitude && lhs.m_longitude == rhs.m_longitude;
}

int CDBGeoCell::getLatitudeExtentInDegree() const noexcept
{
    return 1;
}

std::string CDBGeoCell::getLatitudeDirectoryName() const noexcept
{
    if (m_latitude < 0) {
        return "S" + std::to_string(-m_latitude);
    }

    return "N" + std::to_string(m_latitude);
}

std::string CDBGeoCell::getLongitudeDirectoryName() const noexcept
{
    if (m_longitude < 0) {
        return "W" + toStringWithZeroPadding(3, -m_longitude);
    }

    return "E" + toStringWithZeroPadding(3, m_longitude);
}

const std::filesystem::path &CDBGeoCell::getRelativePath() const noexcept
{
    return m_path;
}

std::optional<int> CDBGeoCell::parseLatFromFilename(const std::string &filename)
{
    int latitude;
    char NS;
    int result = sscanf(filename.c_str(), "%c%d", &NS, &latitude);
    if (result < 2) {
        return std::nullopt;
    }

    if (NS != 'S' && NS != 'N') {
        return std::nullopt;
    }

    latitude = NS == 'S' ? -latitude : latitude;
    if (latitude < -90 || latitude >= 90) {
        return std::nullopt;
    }

    return latitude;
}

std::optional<int> CDBGeoCell::parseLongFromFilename(const std::string &filename)
{
    int longitude;
    char WE;
    int result = sscanf(filename.c_str(), "%c%d", &WE, &longitude);
    if (result < 2) {
        return std::nullopt;
    }

    if (WE != 'W' && WE != 'E') {
        return std::nullopt;
    }

    longitude = WE == 'W' ? -longitude : longitude;
    if (longitude < -180 || longitude > 180) {
        return std::nullopt;
    }

    return longitude;
}

int getZoneFromLatitude(int latitude)
{
    if (latitude >= 89 && latitude < 90) {
        return 10;
    }

    if (latitude >= 80 && latitude < 89) {
        return 9;
    }

    if (latitude >= 75 && latitude < 80) {
        return 8;
    }

    if (latitude >= 70 && latitude < 75) {
        return 7;
    }

    if (latitude >= 50 && latitude < 70) {
        return 6;
    }

    if (latitude >= -50 && latitude < 50) {
        return 5;
    }

    if (latitude >= -70 && latitude < -50) {
        return 4;
    }

    if (latitude >= -75 && latitude < -70) {
        return 3;
    }

    if (latitude >= -80 && latitude < -75) {
        return 2;
    }

    if (latitude >= -89 && latitude < -80) {
        return 1;
    }

    if (latitude >= -90 && latitude < -89) {
        return 0;
    }

    throw std::invalid_argument("Latitude " + std::to_string(latitude) + " out of bound");
}

} // namespace CDBTo3DTiles
