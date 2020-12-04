#pragma once

#include "CDBAttributes.h"
#include "CDBTile.h"
#include "Ellipsoid.h"
#include "EllipsoidTangentPlane.h"
#include "Scene.h"
#include "gdal_priv.h"

namespace CDBTo3DTiles {
class CDBGeometryVectors
{
public:
    CDBGeometryVectors(GDALDatasetUniquePtr dataset, CDBTile tile, const std::filesystem::path &CDBPath);

    inline const Mesh &getMesh() const noexcept { return m_mesh; }

    inline const CDBTile &getTile() const noexcept { return *m_tile; }

    inline const Core::BoundingRegion &getBoundingRegion() const noexcept { return *m_boundRegion; }

    inline const CDBInstancesAttributes &getInstancesAttributes() const noexcept
    {
        return m_instancesAttribs;
    }

    static std::optional<CDBGeometryVectors> createFromFile(const std::filesystem::path &file,
                                                            const std::filesystem::path &CDBPath);

private:
    void createPoint(GDALDataset *vectorDataset);

    void createPolyline(GDALDataset *vectorDataset);

    void createPolygonOrMultiPolygon(GDALDataset *vectorDataset);

    void createPolygon(int featureID,
                       const OGRPolygon *polygon,
                       const Core::Ellipsoid &ellipsoid,
                       Core::EllipsoidTangentPlane &tangentPlane);

    Mesh m_mesh;
    CDBInstancesAttributes m_instancesAttribs;
    std::optional<CDBTile> m_tile;
    std::optional<Core::BoundingRegion> m_boundRegion;
};
} // namespace CDBTo3DTiles
