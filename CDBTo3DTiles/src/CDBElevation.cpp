#include "CDBElevation.h"
#include "BoundingRegion.h"
#include "Ellipsoid.h"
#include "Math.h"
#include "glm/gtc/type_ptr.hpp"
#include "meshoptimizer.h"

namespace CDBTo3DTiles {

static std::vector<double> getRasterElevationHeights(GDALDatasetUniquePtr &rasterData, glm::ivec2 rasterSize);

static Mesh generateElevationMesh(const std::vector<double> terrainHeights,
                                  Core::Cartographic topLeft,
                                  glm::uvec2 rasterSize,
                                  glm::dvec2 pixelSize);

static void loadElevation(const std::filesystem::path &path,
                          Core::Cartographic topLeft,
                          glm::ivec2 &rasterSize,
                          Mesh &mesh);

static void extractVerticesFromExistingSimplifiedMesh(const Mesh &existingMesh,
                                                      Mesh &simplified,
                                                      std::vector<int> &remap,
                                                      unsigned &totalUniqueVertices,
                                                      unsigned idx0,
                                                      unsigned idx1,
                                                      unsigned idx2);

CDBElevation::CDBElevation(Mesh uniformGridMesh, size_t gridWidth, size_t gridHeight, CDBTile tile)
    : m_gridWidth{gridWidth}
    , m_gridHeight{gridHeight}
    , m_uniformGridMesh{std::move(uniformGridMesh)}
    , m_tile{std::move(tile)}
{}

Mesh CDBElevation::createSimplifiedMesh(size_t targetIndexCount, float targetError) const
{
    std::vector<unsigned int> lod(m_uniformGridMesh.indices.size());
    lod.resize(meshopt_simplify(&lod[0],
                                m_uniformGridMesh.indices.data(),
                                m_uniformGridMesh.indices.size(),
                                glm::value_ptr(m_uniformGridMesh.positionRTCs[0]),
                                m_uniformGridMesh.positionRTCs.size(),
                                sizeof(glm::vec3),
                                targetIndexCount,
                                targetError));

    Mesh simplified;
    simplified.aabb = AABB();
    simplified.material = m_uniformGridMesh.material;

    const auto &ellipsoid = Core::Ellipsoid::WGS84;
    const auto &boundRegion = m_tile->getBoundRegion();
    const auto &rectangle = boundRegion.getRectangle();
    auto tileCenter = rectangle.computeCenter();
    auto geodeticNormal = ellipsoid.geodeticSurfaceNormal(tileCenter);
    unsigned count = 0;
    std::vector<int> visible(m_uniformGridMesh.indices.size(), -1);
    for (size_t i = 0; i < lod.size(); i += 3) {
        auto idx0 = lod[i];
        auto idx1 = lod[i + 1];
        auto idx2 = lod[i + 2];

        glm::dvec3 p0 = m_uniformGridMesh.positions[idx0];
        glm::dvec3 p1 = m_uniformGridMesh.positions[idx1];
        glm::dvec3 p2 = m_uniformGridMesh.positions[idx2];

        glm::dvec3 normal = glm::cross(p1 - p0, p2 - p0);
        if (glm::dot(normal, geodeticNormal) < 0.0) {
            extractVerticesFromExistingSimplifiedMesh(m_uniformGridMesh,
                                                      simplified,
                                                      visible,
                                                      count,
                                                      idx2,
                                                      idx1,
                                                      idx0);
        } else {
            extractVerticesFromExistingSimplifiedMesh(m_uniformGridMesh,
                                                      simplified,
                                                      visible,
                                                      count,
                                                      idx0,
                                                      idx1,
                                                      idx2);
        }
    }

    // calculate position rtc
    simplified.positionRTCs.reserve(simplified.positions.size());
    glm::dvec3 center = simplified.aabb->center();
    for (size_t i = 0; i < simplified.positions.size(); ++i) {
        glm::vec3 positionRTC = simplified.positions[i] - center;
        simplified.positionRTCs.emplace_back(positionRTC);
    }

    return simplified;
}

void CDBElevation::indexUVRelativeToParent(const CDBTile &parentTile)
{
    auto parentLevel = parentTile.getLevel();
    if (parentLevel > m_tile->getLevel()) {
        return;
    }

    parentLevel = glm::max(parentLevel, 0);
    size_t verticesWidth = m_gridWidth + 1;
    size_t verticesHeight = m_gridHeight + 1;
    double relativeWidth = glm::pow(2.0, m_tile->getLevel() - parentLevel);
    double invGridWidth = 1.0 / static_cast<double>(m_gridWidth + 1);
    double invWidth = 1.0 / relativeWidth * invGridWidth;
    double beginU = static_cast<double>(m_tile->getRREF()) / relativeWidth;
    double beginV = (relativeWidth - static_cast<double>(m_tile->getUREF()) - 1) / relativeWidth;
    glm::vec2 beginUV = glm::vec2(static_cast<float>(beginU), static_cast<float>(beginV));
    m_uniformGridMesh.UVs.clear();
    m_uniformGridMesh.UVs.reserve(verticesHeight * verticesWidth);
    for (size_t y = 0; y < verticesHeight; ++y) {
        for (size_t x = 0; x < verticesWidth; ++x) {
            double u = static_cast<double>(x) * invWidth + beginUV.x;
            double v = static_cast<double>(y) * invWidth + beginUV.y;
            m_uniformGridMesh.UVs.emplace_back(static_cast<float>(u), static_cast<float>(v));
        }
    }
}

std::optional<CDBElevation> CDBElevation::createNorthWestSubRegion(bool reindexUV) const
{
    if (m_gridWidth % 2 != 0 || m_gridHeight % 2 != 0) {
        return std::nullopt;
    }

    if (m_tile->getLevel() < 0) {
        return createSubRegion(glm::uvec2(0), CDBTile::createChildForNegativeLOD(*m_tile), reindexUV);
    }

    return createSubRegion(glm::uvec2(0), CDBTile::createNorthWestForPositiveLOD(*m_tile), reindexUV);
}

std::optional<CDBElevation> CDBElevation::createNorthEastSubRegion(bool reindexUV) const
{
    if (m_gridWidth % 2 != 0 || m_gridHeight % 2 != 0) {
        return std::nullopt;
    }

    glm::uvec2 regionBegin = glm::uvec2(m_gridWidth / 2, 0);

    if (m_tile->getLevel() < 0) {
        return createSubRegion(regionBegin, CDBTile::createChildForNegativeLOD(*m_tile), reindexUV);
    }

    return createSubRegion(regionBegin, CDBTile::createNorthEastForPositiveLOD(*m_tile), reindexUV);
}

std::optional<CDBElevation> CDBElevation::createSouthWestSubRegion(bool reindexUV) const
{
    if (m_gridWidth % 2 != 0 || m_gridHeight % 2 != 0) {
        return std::nullopt;
    }

    glm::uvec2 regionBegin = glm::uvec2(0, m_gridHeight / 2);

    if (m_tile->getLevel() < 0) {
        return createSubRegion(regionBegin, CDBTile::createChildForNegativeLOD(*m_tile), reindexUV);
    }

    return createSubRegion(regionBegin, CDBTile::createSouthWestForPositiveLOD(*m_tile), reindexUV);
}

std::optional<CDBElevation> CDBElevation::createSouthEastSubRegion(bool reindexUV) const
{
    if (m_gridWidth % 2 != 0 || m_gridHeight % 2 != 0) {
        return std::nullopt;
    }

    glm::uvec2 regionBegin = glm::uvec2(m_gridWidth / 2, m_gridHeight / 2);

    if (m_tile->getLevel() < 0) {
        return createSubRegion(regionBegin, CDBTile::createChildForNegativeLOD(*m_tile), reindexUV);
    }

    return createSubRegion(regionBegin, CDBTile::createSouthEastForPositiveLOD(*m_tile), reindexUV);
}

std::optional<CDBElevation> CDBElevation::createFromFile(const std::filesystem::path &file)
{
    if (file.extension() != ".tif") {
        return std::nullopt;
    }

    auto tile = CDBTile::createFromFile(file.stem().string());
    if (!tile) {
        return std::nullopt;
    }

    // CS_1 == 1 && CS_2 == 1: A grid of data representing the Elevation at the surface of the Earth.
    if (tile->getCS_1() == 1 && tile->getCS_2() == 1) {
        // triangulate raster mesh
        const Core::BoundingRegion &region = tile->getBoundRegion();
        const Core::GlobeRectangle &rectangle = region.getRectangle();
        Core::Cartographic topLeft(rectangle.getWest(), rectangle.getNorth());
        glm::ivec2 rasterSize(0);
        Mesh uniformGridMesh;
        loadElevation(file, topLeft, rasterSize, uniformGridMesh);

        if (uniformGridMesh.positions.empty()) {
            return std::nullopt;
        }

        // initialize grid size
        int gridWidth = rasterSize.x;
        int gridHeight = rasterSize.y;

        return CDBElevation(std::move(uniformGridMesh), gridWidth, gridHeight, *tile);
    }

    return std::nullopt;
}

CDBElevation CDBElevation::createSubRegion(glm::uvec2 regionBegin,
                                           const CDBTile &subRegionTile,
                                           bool reindexUV) const
{
    size_t regionGridWidth = m_gridWidth / 2;
    size_t regionGridHeight = m_gridHeight / 2;
    Mesh elevation = createSubRegionMesh(regionBegin,
                                         regionBegin + glm::uvec2(regionGridWidth, regionGridHeight),
                                         reindexUV);

    return CDBElevation(elevation, regionGridWidth, regionGridHeight, subRegionTile);
}

Mesh CDBElevation::createSubRegionMesh(glm::uvec2 gridFrom, glm::uvec2 gridTo, bool reindexUV) const
{
    // create elevation mesh
    Mesh elevation;
    elevation.aabb = AABB();

    size_t verticesWidth = m_gridWidth + 1;
    size_t regionVerticesWidth = gridTo.x - gridFrom.x + 1;
    size_t regionVerticesHeight = gridTo.y - gridFrom.y + 1;
    size_t totalRegionIndices = (regionVerticesWidth - 1) * (regionVerticesHeight - 1) * 6;
    size_t totalRegionVertices = regionVerticesWidth * regionVerticesHeight;

    elevation.positions.reserve(totalRegionVertices);
    elevation.positionRTCs.reserve(totalRegionVertices);
    elevation.UVs.reserve(totalRegionVertices);
    elevation.indices.reserve(totalRegionIndices);
    for (uint32_t y = gridFrom.y; y < gridTo.y + 1; ++y) {
        for (uint32_t x = gridFrom.x; x < gridTo.x + 1; ++x) {
            size_t currIndex = y * verticesWidth + x;
            glm::dvec3 position = m_uniformGridMesh.positions[currIndex];
            elevation.aabb->merge(position);
            elevation.positions.emplace_back(position);
            if (reindexUV) {
                elevation.UVs.emplace_back(glm::vec2(static_cast<float>(x - gridFrom.x)
                                                         / static_cast<float>(regionVerticesWidth - 1),
                                                     static_cast<float>(y - gridFrom.y)
                                                         / static_cast<float>(regionVerticesHeight - 1)));
            } else {
                elevation.UVs.emplace_back(m_uniformGridMesh.UVs[currIndex]);
            }

            if (x < gridTo.x && y < gridTo.y) {
                uint32_t subX = x - gridFrom.x;
                uint32_t subY = y - gridFrom.y;
                elevation.indices.emplace_back(subY * regionVerticesWidth + subX + 1);
                elevation.indices.emplace_back(subY * regionVerticesWidth + subX);
                elevation.indices.emplace_back((subY + 1) * regionVerticesWidth + subX);

                elevation.indices.emplace_back((subY + 1) * regionVerticesWidth + subX);
                elevation.indices.emplace_back((subY + 1) * regionVerticesWidth + subX + 1);
                elevation.indices.emplace_back(subY * regionVerticesWidth + subX + 1);
            }
        }
    }

    glm::dvec3 center = elevation.aabb->center();
    for (size_t i = 0; i < totalRegionVertices; ++i) {
        glm::vec3 positionRTC = elevation.positions[i] - center;
        elevation.positionRTCs.emplace_back(positionRTC);
    }

    return elevation;
}

void extractVerticesFromExistingSimplifiedMesh(const Mesh &existingSimplifiedMesh,
                                               Mesh &newSimplifiedMesh,
                                               std::vector<int> &remap,
                                               unsigned &totalUniqueVertices,
                                               unsigned idx0,
                                               unsigned idx1,
                                               unsigned idx2)
{
    if (remap[idx0] == -1) {
        newSimplifiedMesh.aabb->merge(existingSimplifiedMesh.positions[idx0]);
        newSimplifiedMesh.positions.emplace_back(existingSimplifiedMesh.positions[idx0]);
        newSimplifiedMesh.UVs.emplace_back(existingSimplifiedMesh.UVs[idx0]);

        remap[idx0] = totalUniqueVertices;
        ++totalUniqueVertices;
    }

    if (remap[idx1] == -1) {
        newSimplifiedMesh.aabb->merge(existingSimplifiedMesh.positions[idx1]);
        newSimplifiedMesh.positions.emplace_back(existingSimplifiedMesh.positions[idx1]);
        newSimplifiedMesh.UVs.emplace_back(existingSimplifiedMesh.UVs[idx1]);

        remap[idx1] = totalUniqueVertices;
        ++totalUniqueVertices;
    }

    if (remap[idx2] == -1) {
        newSimplifiedMesh.aabb->merge(existingSimplifiedMesh.positions[idx2]);
        newSimplifiedMesh.positions.emplace_back(existingSimplifiedMesh.positions[idx2]);
        newSimplifiedMesh.UVs.emplace_back(existingSimplifiedMesh.UVs[idx2]);

        remap[idx2] = totalUniqueVertices;
        ++totalUniqueVertices;
    }

    newSimplifiedMesh.indices.emplace_back(remap[idx0]);
    newSimplifiedMesh.indices.emplace_back(remap[idx1]);
    newSimplifiedMesh.indices.emplace_back(remap[idx2]);
}

std::vector<double> getRasterElevationHeights(GDALDatasetUniquePtr &rasterData, glm::ivec2 rasterSize)
{
    auto heightBand = rasterData->GetRasterBand(1);
    auto rasterDataType = heightBand->GetRasterDataType();
    if (rasterDataType != GDT_Float32 && rasterDataType != GDT_Float64) {
        return {};
    }

    int rasterWidth = rasterSize.x;
    int rasterHeight = rasterSize.y;
    std::vector<double> elevationHeights(rasterWidth * rasterHeight, 0.0);
    if (GDALRasterIO(heightBand,
                     GDALRWFlag::GF_Read,
                     0,
                     0,
                     rasterWidth,
                     rasterHeight,
                     elevationHeights.data(),
                     rasterWidth,
                     rasterHeight,
                     GDALDataType::GDT_Float64,
                     0,
                     0)
        != CE_None) {
        return {};
    }

    return elevationHeights;
}

Mesh generateElevationMesh(const std::vector<double> elevationHeights,
                           Core::Cartographic topLeft,
                           glm::uvec2 rasterSize,
                           glm::dvec2 pixelSize)
{
    // CDB uses only WG84 ellipsoid
    Core::Ellipsoid ellipsoid = Core::Ellipsoid::WGS84;

    // create elevation mesh
    Mesh elevation;
    elevation.aabb = AABB();

    // calculate bounding box, bounding regions, positions, uv, and indices
    size_t rasterWidth = rasterSize.x;
    size_t rasterHeight = rasterSize.y;
    size_t verticesWidth = rasterWidth + 1;
    size_t verticesHeight = rasterHeight + 1;
    float inverseWidth = 1.0f / static_cast<float>(verticesWidth);
    float inverseHeight = 1.0f / static_cast<float>(verticesHeight);

    size_t totalVertices = verticesWidth * verticesHeight;
    size_t totalIndices = (verticesWidth - 1) * (verticesHeight - 1) * 6;
    elevation.positions.reserve(totalVertices);
    elevation.positionRTCs.reserve(totalVertices);
    elevation.UVs.reserve(totalVertices);
    elevation.indices.reserve(totalIndices);

    for (size_t y = 0; y < verticesHeight; ++y) {
        for (size_t x = 0; x < verticesWidth; ++x) {
            double longitude = topLeft.longitude + glm::radians(static_cast<double>(x) * pixelSize.x);
            double latitude = topLeft.latitude + glm::radians(static_cast<double>(y) * pixelSize.y);
            double height = static_cast<double>(
                elevationHeights[glm::min(y, rasterHeight - 1) * rasterWidth + glm::min(x, rasterWidth - 1)]);
            Core::Cartographic cartographic(longitude, latitude, height);
            glm::dvec3 position = ellipsoid.cartographicToCartesian(cartographic);

            elevation.positions.emplace_back(position);
            elevation.aabb->merge(position);
            elevation.UVs.emplace_back(static_cast<float>(x) * inverseWidth,
                                       static_cast<float>(y) * inverseHeight);
            if (x < verticesWidth - 1 && y < verticesHeight - 1) {
                elevation.indices.emplace_back(y * verticesWidth + x + 1);
                elevation.indices.emplace_back(y * verticesWidth + x);
                elevation.indices.emplace_back((y + 1) * verticesWidth + x);

                elevation.indices.emplace_back((y + 1) * verticesWidth + x);
                elevation.indices.emplace_back((y + 1) * verticesWidth + x + 1);
                elevation.indices.emplace_back(y * verticesWidth + x + 1);
            }
        }
    }

    // calculate position rtc
    glm::dvec3 center = elevation.aabb->center();
    for (size_t i = 0; i < totalVertices; ++i) {
        glm::vec3 positionRTC = elevation.positions[i] - center;
        elevation.positionRTCs.emplace_back(positionRTC);
    }

    return elevation;
}

void loadElevation(const std::filesystem::path &path,
                   Core::Cartographic topLeft,
                   glm::ivec2 &rasterSize,
                   Mesh &mesh)
{
    std::string file = path.string();
    GDALDatasetUniquePtr rasterData = GDALDatasetUniquePtr(
        (GDALDataset *) GDALOpen(file.c_str(), GDALAccess::GA_ReadOnly));

    if (rasterData == nullptr) {
        return;
    }

    // retrieve raster basic info
    double geoTransform[6];
    rasterData->GetGeoTransform(geoTransform);
    if (geoTransform[2] != 0.0 || geoTransform[4] != 0.0) {
        return;
    }

    rasterSize = glm::uvec2(static_cast<unsigned>(rasterData->GetRasterXSize()),
                            static_cast<unsigned>(rasterData->GetRasterYSize()));
    glm::dvec2 pixelSize(geoTransform[1], geoTransform[5]);

    // retrieve heights
    auto elevationHeights = getRasterElevationHeights(rasterData, rasterSize);
    if (elevationHeights.empty()) {
        return;
    }

    // generate elevation mesh
    mesh = generateElevationMesh(elevationHeights, topLeft, rasterSize, pixelSize);
}

} // namespace CDBTo3DTiles
