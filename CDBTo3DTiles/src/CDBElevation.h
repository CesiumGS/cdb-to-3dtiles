#pragma once

#include "CDBTile.h"
#include "Cartographic.h"
#include "Scene.h"
#include "gdal_priv.h"
#include <filesystem>

namespace CDBTo3DTiles {

class CDBElevation
{
public:
    CDBElevation(Mesh uniformGridMesh, size_t gridWidth, size_t gridHeight, CDBTile tile);

    Mesh createSimplifiedMesh(size_t targetIndexCount, float targetError) const;

    inline const Mesh &getUniformGridMesh() const noexcept { return m_uniformGridMesh; }

    inline size_t getGridWidth() const noexcept { return m_gridWidth; }

    inline size_t getGridHeight() const noexcept { return m_gridHeight; }

    inline const CDBTile &getTile() const noexcept { return *m_tile; }

    inline void setTile(const CDBTile &tile) { m_tile = tile; }

    void indexUVRelativeToParent(const CDBTile &parentTile);

    std::optional<CDBElevation> createNorthWestSubRegion(bool reindexUVs) const;

    std::optional<CDBElevation> createNorthEastSubRegion(bool reindexUVs) const;

    std::optional<CDBElevation> createSouthWestSubRegion(bool reindexUVs) const;

    std::optional<CDBElevation> createSouthEastSubRegion(bool reindexUVs) const;

    static std::optional<CDBElevation> createFromFile(const std::filesystem::path &file);

private:
    CDBElevation createSubRegion(glm::uvec2 begin, const CDBTile &subRegionTile, bool reindexUV) const;

    Mesh createSubRegionMesh(glm::uvec2 gridFrom, glm::uvec2 gridTo, bool reindexUV) const;

    size_t m_gridWidth;
    size_t m_gridHeight;
    Mesh m_uniformGridMesh;
    std::optional<CDBTile> m_tile;
};

} // namespace CDBTo3DTiles
