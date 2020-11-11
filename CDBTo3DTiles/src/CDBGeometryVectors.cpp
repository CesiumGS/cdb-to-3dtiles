#include "CDBGeometryVectors.h"
#include "mapbox/earcut.hpp"
#include "ogrsf_frmts.h"

namespace CDBTo3DTiles {

static std::optional<CDBClassesAttributes> createClassesAttributes(const CDBTile &instancesTile,
                                                                   const std::filesystem::path &CDBPath);

CDBGeometryVectors::CDBGeometryVectors(GDALDatasetUniquePtr dataset,
                                       CDBTile tile,
                                       const std::filesystem::path &CDBPath)
    : m_tile{std::move(tile)}
{
    int CS_2 = tile.getCS_2();
    switch (CS_2) {
    case static_cast<int>(CDBVectorCS2::PointFeature):
        createPoint(dataset.get());
        break;
    case static_cast<int>(CDBVectorCS2::LinealFeature):
        createPolyline(dataset.get());
        break;
    case static_cast<int>(CDBVectorCS2::PolygonFeature):
        createPolygonOrMultiPolygon(dataset.get());
        break;
    default:
        break;
    }

    // merge instance attributes with class attributes
    auto classesAttributes = createClassesAttributes(tile, CDBPath);
    if (classesAttributes) {
        m_instancesAttribs.mergeClassesAttributes(*classesAttributes);
    }
}

std::optional<CDBGeometryVectors> CDBGeometryVectors::createFromFile(const std::filesystem::path &file,
                                                                     const std::filesystem::path &CDBPath)
{
    if (file.extension() != ".dbf") {
        return std::nullopt;
    }

    auto tile = CDBTile::createFromFile(file.stem().string());
    if (!tile) {
        return std::nullopt;
    }

    int CS_2 = tile->getCS_2();
    if (CS_2 == static_cast<int>(CDBVectorCS2::PointFeature)
        || CS_2 == static_cast<int>(CDBVectorCS2::LinealFeature)
        || CS_2 == static_cast<int>(CDBVectorCS2::PolygonFeature)) {
        GDALDatasetUniquePtr dataset = GDALDatasetUniquePtr(
            (GDALDataset *) GDALOpenEx(file.string().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
        if (dataset) {
            auto geometryVector = CDBGeometryVectors(std::move(dataset), *tile, CDBPath);

            return geometryVector;
        }
    }

    return std::nullopt;
}

void CDBGeometryVectors::createPoint(GDALDataset *vectorDataset)
{
    m_mesh.aabb = AABB();
    m_mesh.primitiveType = PrimitiveType::Points;

    const auto &ellipsoid = Core::Ellipsoid::WGS84;
    int featureID = 0;
    for (int i = 0; i < vectorDataset->GetLayerCount(); ++i) {
        OGRLayer *layer = vectorDataset->GetLayer(i);
        for (const auto &feature : *layer) {
            m_instancesAttribs.addInstanceFeature(*feature);

            const OGRGeometry *geometry = feature->GetGeometryRef();
            if (geometry != nullptr && wkbFlatten(geometry->getGeometryType()) == wkbPoint) {
                const OGRPoint *p = geometry->toPoint();

                Core::Cartographic cartographic(glm::radians(p->getX()), glm::radians(p->getY()), p->getZ());

                auto position = ellipsoid.cartographicToCartesian(cartographic);

                m_mesh.aabb->merge(position);
                m_mesh.positions.emplace_back(position);
                m_mesh.batchIDs.emplace_back(featureID);

                ++featureID;
            }
        }
    }

    auto center = m_mesh.aabb->center();
    m_mesh.positionRTCs.reserve(m_mesh.positions.size());
    for (auto position : m_mesh.positions) {
        m_mesh.positionRTCs.emplace_back(position - center);
    }
}

void CDBGeometryVectors::createPolyline(GDALDataset *vectorDataset)
{
    m_mesh.aabb = AABB();
    m_mesh.primitiveType = PrimitiveType::Lines;

    const auto &ellipsoid = Core::Ellipsoid::WGS84;
    int featureID = 0;
    for (int i = 0; i < vectorDataset->GetLayerCount(); ++i) {
        OGRLayer *layer = vectorDataset->GetLayer(i);
        for (const auto &feature : *layer) {
            m_instancesAttribs.addInstanceFeature(*feature);

            const OGRGeometry *geometry = feature->GetGeometryRef();
            if (geometry != nullptr && wkbFlatten(geometry->getGeometryType()) == wkbLineString) {
                const OGRLineString *lineString = geometry->toLineString();
                for (int j = 0; j < lineString->getNumPoints(); ++j) {
                    OGRPoint p;
                    lineString->getPoint(j, &p);

                    Core::Cartographic cartographic(glm::radians(p.getX()), glm::radians(p.getY()), p.getZ());

                    auto position = ellipsoid.cartographicToCartesian(cartographic);

                    m_mesh.aabb->merge(position);
                    m_mesh.positions.emplace_back(position);
                    m_mesh.batchIDs.emplace_back(featureID);

                    if (j > 0) {
                        auto index = m_mesh.positions.size() - 1;
                        m_mesh.indices.emplace_back(index - 1);
                        m_mesh.indices.emplace_back(index);
                    }
                }

                ++featureID;
            }
        }
    }

    auto center = m_mesh.aabb->center();
    m_mesh.positionRTCs.reserve(m_mesh.positions.size());
    for (auto position : m_mesh.positions) {
        m_mesh.positionRTCs.emplace_back(position - center);
    }
}

void CDBGeometryVectors::createPolygonOrMultiPolygon(GDALDataset *vectorDataset)
{
    m_mesh.aabb = AABB();

    int featureID = 0;
    for (int i = 0; i < vectorDataset->GetLayerCount(); ++i) {
        OGRLayer *layer = vectorDataset->GetLayer(i);
        Core::Ellipsoid ellipsoid = Core::Ellipsoid::WGS84;
        Core::GlobeRectangle rectangle = m_tile->getBoundRegion().getRectangle();
        Core::Cartographic tileCenter = rectangle.computeCenter();
        Core::EllipsoidTangentPlane tangentPlane(ellipsoid.cartographicToCartesian(tileCenter));

        for (const auto &feature : *layer) {
            m_instancesAttribs.addInstanceFeature(*feature);

            const OGRGeometry *geometry = feature->GetGeometryRef();
            if (geometry != nullptr && wkbFlatten(geometry->getGeometryType()) == wkbMultiPolygon) {
                const OGRMultiPolygon *multiPolygon = geometry->toMultiPolygon();
                for (auto polygon : *multiPolygon) {
                    createPolygon(featureID, polygon, ellipsoid, tangentPlane);
                }

                ++featureID;
            } else if (geometry != nullptr && wkbFlatten(geometry->getGeometryType()) == wkbPolygon) {
                const OGRPolygon *polygon = geometry->toPolygon();
                createPolygon(featureID, polygon, ellipsoid, tangentPlane);
                ++featureID;
            }
        }
    }

    auto center = m_mesh.aabb->center();
    m_mesh.positionRTCs.reserve(m_mesh.positions.size());
    for (auto position : m_mesh.positions) {
        m_mesh.positionRTCs.emplace_back(position - center);
    }
}

void CDBGeometryVectors::createPolygon(int featureID,
                                       const OGRPolygon *polygon,
                                       const Core::Ellipsoid &ellipsoid,
                                       Core::EllipsoidTangentPlane &tangentPlane)
{
    uint32_t currPositionSize = static_cast<uint32_t>(m_mesh.positions.size());
    std::vector<std::vector<std::pair<double, double>>> mapboxRings;
    for (auto lineRing : *polygon) {
        std::vector<std::pair<double, double>> mapboxRing;
        for (auto point : *lineRing) {
            Core::Cartographic cartographic(glm::radians(point.getX()),
                                            glm::radians(point.getY()),
                                            point.getZ());
            glm::dvec3 position = ellipsoid.cartographicToCartesian(cartographic);
            glm::dvec2 projectPosition = tangentPlane.projectPointToNearestOnPlane(position);
            mapboxRing.emplace_back(projectPosition.x, projectPosition.y);

            m_mesh.positions.emplace_back(position);
            m_mesh.batchIDs.emplace_back(featureID);
            m_mesh.aabb->merge(position);
        }

        mapboxRings.emplace_back(mapboxRing);
    }

    auto indices = mapbox::earcut<uint32_t>(mapboxRings);
    for (auto index : indices) {
        index += currPositionSize;
        m_mesh.indices.emplace_back(index);
    }
}

std::optional<CDBClassesAttributes> createClassesAttributes(const CDBTile &instancesTile,
                                                            const std::filesystem::path &CDBPath)
{
    int CS_2 = instancesTile.getCS_2();
    if (CS_2 == static_cast<int>(CDBVectorCS2::PointFeature)) {
        CS_2 = static_cast<int>(CDBVectorCS2::PointFeatureClassLevel);
    } else if (CS_2 == static_cast<int>(CDBVectorCS2::LinealFeature)) {
        CS_2 = static_cast<int>(CDBVectorCS2::LinealFeatureClassLevel);
    } else if (CS_2 == static_cast<int>(CDBVectorCS2::PolygonFeature)) {
        CS_2 = static_cast<int>(CDBVectorCS2::PolygonFeatureClassLevel);
    } else {
        return std::nullopt;
    }

    CDBTile classVectorsTile = CDBTile(instancesTile.getGeoCell(),
                                       instancesTile.getDataset(),
                                       instancesTile.getCS_1(),
                                       CS_2,
                                       instancesTile.getLevel(),
                                       instancesTile.getUREF(),
                                       instancesTile.getRREF());

    auto classLevelPath = CDBPath / (classVectorsTile.getRelativePath().string() + ".dbf");
    if (!std::filesystem::exists(classLevelPath)) {
        return std::nullopt;
    }

    GDALDatasetUniquePtr dataset = GDALDatasetUniquePtr(
        (GDALDataset *) GDALOpenEx(classLevelPath.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    if (!dataset) {
        return std::nullopt;
    }

    return CDBClassesAttributes(std::move(dataset), std::move(classVectorsTile));
}

} // namespace CDBTo3DTiles
