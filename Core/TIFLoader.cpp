#include "TIFLoader.h"
#include "GDALImage.h"
#include "gdal_priv.h"
#include <memory>

static std::vector<float> GetRasterTerrainHeights(const boost::filesystem::path &rasterPath,
                                                  GDALImage &rasterData,
                                                  glm::ivec2 rasterSize);

static TerrainMesh GenerateTerrainMesh(const std::vector<float> terrainHeights,
                                       Cartographic topLeft,
                                       Cartographic bottomRight,
                                       glm::ivec2 rasterSize,
                                       glm::dvec2 pixelSize,
                                       const TIFOption &option);

TerrainMesh LoadTIFFile(const boost::filesystem::path &filePath, const TIFOption &option)
{
    std::string file = filePath.string();
    GDALImage rasterData(file, GDALAccess::GA_ReadOnly);

    // retrieve raster basic info
    double geoTransform[6];
    rasterData.data()->GetGeoTransform(geoTransform);
    if (geoTransform[2] != 0.0 || geoTransform[4] != 0.0) {
        throw std::runtime_error(file + " is not a north-up raster");
    }

    glm::ivec2 rasterSize(rasterData.data()->GetRasterXSize(), rasterData.data()->GetRasterYSize());
    glm::dvec2 pixelSize(geoTransform[1], geoTransform[5]);
    Cartographic topLeft;
    if (option.topLeft != nullptr) {
        topLeft = Cartographic(option.topLeft->longitude, option.topLeft->latitude, 0.0);
    } else {
        topLeft = Cartographic(geoTransform[0] + pixelSize.x, geoTransform[3] + pixelSize.y, 0.0);
    }
    Cartographic bottomRight(topLeft.longitude + rasterSize.x * pixelSize.x,
                             topLeft.latitude + rasterSize.y * pixelSize.y,
                             0.0);

    // retrieve heights
    auto terrainHeights = GetRasterTerrainHeights(filePath, rasterData, rasterSize);

    // generate terrain mesh
    return GenerateTerrainMesh(terrainHeights, topLeft, bottomRight, rasterSize, pixelSize, option);
}

std::vector<float> GetRasterTerrainHeights(const boost::filesystem::path &rasterPath,
                                           GDALImage &rasterData,
                                           glm::ivec2 rasterSize)
{
    auto heightBand = rasterData.data()->GetRasterBand(1);
    if (heightBand->GetRasterDataType() == GDT_Byte || heightBand->GetRasterDataType() == GDT_Unknown) {
        throw std::runtime_error(rasterPath.string()
                                 + std::string(" values are not integer or floating point"));
    }

    int rasterWidth = rasterSize.x;
    int rasterHeight = rasterSize.y;
    std::vector<float> terrainHeights(rasterWidth * rasterHeight, 0.0f);
    if (GDALRasterIO(heightBand,
                     GDALRWFlag::GF_Read,
                     0,
                     0,
                     rasterWidth,
                     rasterHeight,
                     terrainHeights.data(),
                     rasterWidth,
                     rasterHeight,
                     GDALDataType::GDT_Float32,
                     0,
                     0)
        != CE_None) {
        throw std::runtime_error(rasterPath.string() + std::string(" cannot read terrain height value"));
    }

    return terrainHeights;
}

TerrainMesh GenerateTerrainMesh(const std::vector<float> terrainHeights,
                                Cartographic topLeft,
                                Cartographic bottomRight,
                                glm::ivec2 rasterSize,
                                glm::dvec2 pixelSize,
                                const TIFOption &option)
{
    // config option
    const Ellipsoid &ellipsoid = option.ellipsoid ? *option.ellipsoid : Ellipsoid::WGS84();

    // create terrain mesh
    TerrainMesh terrain;
    int rasterWidth = rasterSize.x;
    int rasterHeight = rasterSize.y;

    // calculate bounding box, bounding regions, positions, uv, normals, and indices
    double minHeight = terrainHeights.front();
    double maxHeight = terrainHeights.front();
    int verticesWidth = rasterWidth + 1;
    int verticesHeight = rasterHeight + 1;
    float inverseWidth = 1.0f / verticesWidth;
    float inverseHeight = 1.0f / verticesHeight;

    size_t totalVertices = verticesWidth * verticesHeight;
    size_t totalIndices = (verticesWidth - 1) * (verticesHeight - 1) * 6;
    terrain.positions.reserve(totalVertices);
    terrain.positionsRtc.reserve(totalVertices);
    terrain.uv.reserve(totalVertices);
    terrain.indices.reserve(totalIndices);
    for (int y = 0; y < verticesHeight; ++y) {
        for (int x = 0; x < verticesWidth; ++x) {
            // TODO: currently, height is repeated for raster edge. We need to query the adjacent tile for proper triangulation
            // or upsample from the parent to fill tile that is not available in this lod
            Cartographic cartographic;
            cartographic.longitude = glm::radians(topLeft.longitude + x * pixelSize.x);
            cartographic.latitude = glm::radians(topLeft.latitude + y * pixelSize.y);
            cartographic.height = static_cast<double>(
                terrainHeights[glm::min(y, rasterHeight - 1) * rasterWidth + glm::min(x, rasterWidth - 1)]);
            glm::dvec3 position = ellipsoid.CartographicToCartesian(cartographic);

            minHeight = glm::min(minHeight, cartographic.height);
            minHeight = glm::max(maxHeight, cartographic.height);
            terrain.positions.emplace_back(position);
            terrain.boundBox.mergePoint(position);
            terrain.uv.emplace_back(x * inverseWidth, y * inverseHeight);
            if (x < verticesWidth - 1 && y < verticesHeight - 1) {
                terrain.indices.emplace_back(y * verticesWidth + x + 1);
                terrain.indices.emplace_back(y * verticesWidth + x);
                terrain.indices.emplace_back((y + 1) * verticesWidth + x);

                terrain.indices.emplace_back((y + 1) * verticesWidth + x);
                terrain.indices.emplace_back((y + 1) * verticesWidth + x + 1);
                terrain.indices.emplace_back(y * verticesWidth + x + 1);
            }
        }
    }

    // calculate normals
    terrain.normals.resize(totalVertices, glm::vec3(0.0));
    for (size_t i = 0; i < terrain.indices.size(); i += 3) {
        uint32_t idx0 = terrain.indices[i];
        uint32_t idx1 = terrain.indices[i + 1];
        uint32_t idx2 = terrain.indices[i + 2];

        glm::dvec3 p0 = terrain.positions[idx0];
        glm::dvec3 p1 = terrain.positions[idx1];
        glm::dvec3 p2 = terrain.positions[idx2];

        glm::vec3 normal = glm::cross(p1 - p0, p2 - p0);
        terrain.normals[idx0] += normal;
        terrain.normals[idx1] += normal;
        terrain.normals[idx2] += normal;
    }

    // normalize normal and calculate position rtc
    glm::dvec3 center = terrain.boundBox.center();
    for (size_t i = 0; i < totalVertices; ++i) {
        auto &normal = terrain.normals[i];
        if (glm::abs(glm::dot(normal, normal)) > 0.0000001) {
            normal = glm::normalize(normal);
        }

        glm::vec3 positionRtc = terrain.positions[i] - center;
        terrain.positionsRtc.emplace_back(positionRtc);
    }

    // find terrain bounding region
    terrain.region.minHeight = minHeight;
    terrain.region.maxHeight = maxHeight;
    terrain.region.west = glm::radians(topLeft.longitude);
    terrain.region.south = glm::radians(bottomRight.latitude);
    terrain.region.east = glm::radians(bottomRight.longitude);
    terrain.region.north = glm::radians(topLeft.latitude);

    return terrain;
}
