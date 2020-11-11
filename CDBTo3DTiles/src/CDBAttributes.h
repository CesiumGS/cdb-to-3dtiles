#pragma once

#include "CDBTile.h"
#include "Cartographic.h"
#include "gdal_priv.h"

namespace CDBTo3DTiles {
class CDBClassesAttributes;

glm::dmat4 calculateModelOrientation(glm::dvec3 worldPosition, double orientation);

enum class CDBVectorCS2
{
    PointFeature = 1,
    PointFeatureClassLevel = 2,
    LinealFeature = 3,
    LinealFeatureClassLevel = 4,
    PolygonFeature = 5,
    PolygonFeatureClassLevel = 6,
    LinealFigurePointFeature = 7,
    LinealFigurePointFeatureClassLevel = 8,
    PolygonFigurePointFeature = 9,
    PolygonFigurePointFeatureClassLevel = 10,
    Relationship2DTileConnection = 11,
    Relationship2DDatasetConnection = 15,
    PointFeatureExtendedLevel = 16,
    LinealFeatureExtendedLevel = 17,
    PolygonFeatureExtendedLevel = 18,
    LinealFigurePointExtendedLevel = 19,
    PolygonFigurePointExtendedLevel = 20
};

class CDBInstancesAttributes
{
public:
    void addInstanceFeature(const OGRFeature &feature);

    void mergeClassesAttributes(const CDBClassesAttributes &classVectors) noexcept;

    inline size_t getInstancesCount() const noexcept { return m_CNAMs.size(); }

    inline const std::vector<std::string> &getCNAMs() const noexcept { return m_CNAMs; }

    inline const std::map<std::string, std::vector<int>> &getIntegerAttribs() const noexcept
    {
        return m_integerAttribs;
    }

    inline const std::map<std::string, std::vector<double>> &getDoubleAttribs() const noexcept
    {
        return m_doubleAttribs;
    }

    inline const std::map<std::string, std::vector<std::string>> &getStringAttribs() const noexcept
    {
        return m_stringAttribs;
    }

    inline std::vector<std::string> &getCNAMs() noexcept { return m_CNAMs; }

    inline std::map<std::string, std::vector<int>> &getIntegerAttribs() noexcept { return m_integerAttribs; }

    inline std::map<std::string, std::vector<double>> &getDoubleAttribs() noexcept { return m_doubleAttribs; }

    inline std::map<std::string, std::vector<std::string>> &getStringAttribs() noexcept
    {
        return m_stringAttribs;
    }

private:
    std::vector<std::string> m_CNAMs;
    std::map<std::string, std::vector<int>> m_integerAttribs;
    std::map<std::string, std::vector<double>> m_doubleAttribs;
    std::map<std::string, std::vector<std::string>> m_stringAttribs;
};

class CDBClassesAttributes
{
public:
    CDBClassesAttributes(GDALDatasetUniquePtr dataset, CDBTile tile);

    inline const CDBTile &getTile() const noexcept { return *m_tile; }

    inline const std::map<std::string, size_t> &getCNAMs() const noexcept { return m_CNAMs; }

    inline const std::map<std::string, std::vector<int>> &getIntegerAttribs() const noexcept
    {
        return m_integerAttribs;
    }

    inline const std::map<std::string, std::vector<double>> &getDoubleAttribs() const noexcept
    {
        return m_doubleAttribs;
    }

    inline const std::map<std::string, std::vector<std::string>> &getStringAttribs() const noexcept
    {
        return m_stringAttribs;
    }

private:
    void addClassFeaturesAttribs(const OGRFeature &feature);

    std::optional<CDBTile> m_tile;
    std::map<std::string, size_t> m_CNAMs;
    std::map<std::string, std::vector<int>> m_integerAttribs;
    std::map<std::string, std::vector<double>> m_doubleAttribs;
    std::map<std::string, std::vector<std::string>> m_stringAttribs;
};

class CDBModelsAttributes
{
public:
    CDBModelsAttributes(GDALDatasetUniquePtr dataset, CDBTile tile, const std::filesystem::path &CDBPath);

    inline const std::vector<Core::Cartographic> &getCartographicPositions() const noexcept
    {
        return m_cartographicPositions;
    }

    inline std::vector<Core::Cartographic> &getCartographicPositions() noexcept
    {
        return m_cartographicPositions;
    }

    inline const std::vector<double> &getOrientations() const noexcept { return m_orientations; }

    inline const std::vector<glm::vec3> &getScales() const noexcept { return m_scales; }

    inline const CDBTile &getTile() const noexcept { return *m_tile; }

    inline const CDBInstancesAttributes &getInstancesAttributes() const noexcept
    {
        return m_instancesAttribs;
    }

private:
    std::optional<CDBClassesAttributes> createClassesAttributes(const CDBTile &instancesTile,
                                                                const std::filesystem::path &CDBPath);

    std::vector<glm::vec3> m_scales;
    std::vector<double> m_orientations;
    std::vector<Core::Cartographic> m_cartographicPositions;
    CDBInstancesAttributes m_instancesAttribs;
    std::optional<CDBTile> m_tile;
};
} // namespace CDBTo3DTiles
