#include "CDBTo3DTiles.h"
#include "CDB.h"
#include "boost/functional/hash.hpp"
#include <fstream>
#include <iostream>

const boost::filesystem::path CDBTo3DTiles::TERRAIN_PATH = "Terrains";
const boost::filesystem::path CDBTo3DTiles::IMAGERY_PATH = "Imagery";

void CDBTo3DTiles::Convert(const boost::filesystem::path &CDBPath, const boost::filesystem::path &output)
{
    auto terrainDirectory = output / TERRAIN_PATH;
    auto imageryDirectory = output / TERRAIN_PATH / IMAGERY_PATH;
    boost::filesystem::create_directories(terrainDirectory);
    boost::filesystem::create_directories(imageryDirectory);

    CDB cdb(CDBPath);
    cdb.ForEachGeoCell([&](const boost::filesystem::path &geoCellPath, const CDBGeoCell &geoCell) {
        cdb.ForEachImageryTile(geoCellPath,
                               [&](CDBImagery &imagery) { ConvertImagery(imagery, imageryDirectory); });

        cdb.ForEachElevationTile(geoCellPath, [&](const CDBTerrain &terrain) {
            ConvertTerrain(terrain, terrainDirectory);
        });

        SaveGeoCellConversions(geoCell, output);
    });
}

void CDBTo3DTiles::ConvertTerrain(const CDBTerrain &terrain,
                                  const boost::filesystem::path &outputTerrainDirectory)
{
    // create gltf file
    auto terrainB3DM = terrain.tile.encodedCDBFullName + ".b3dm";
    auto b3dmFullPath = outputTerrainDirectory / terrainB3DM;
    WriteTerrainToB3DM(terrain, b3dmFullPath.string());

    auto rootIt = _terrainRootTiles.find(terrain.tile.geoCell);
    auto &tiles = _terrainTiles[terrain.tile.geoCell];
    int rootIndex;
    if (rootIt == _terrainRootTiles.end()) {
        // TODO: calculate geometric error for the root
        // TODO: calculate GeoCell bounding region
        tiles.emplace_back(0, 0, -10, 313000.0f, BoundRegion(), "");
        rootIndex = tiles.size() - 1;
        _terrainRootTiles.insert({terrain.tile.geoCell, rootIndex});
    } else {
        rootIndex = rootIt->second;
    }

    Tile tile(terrain.tile.x, terrain.tile.y, terrain.tile.level, 1.0f, terrain.mesh.region, terrainB3DM);
    InsertTerrainTileToTileset(tile, rootIndex, tiles);
}

void CDBTo3DTiles::ConvertImagery(CDBImagery &imagery, const boost::filesystem::path &outputImageryDirectory)
{
    auto imageryFileName = imagery.tile.encodedCDBFullName + ".jpg";
    auto imageryRelativePath = IMAGERY_PATH / imageryFileName;
    auto imageryPath = outputImageryDirectory / imageryFileName;

    // save the content
    auto driver = (GDALDriver *) GDALGetDriverByName("JPEG");
    if (driver != nullptr) {
        GDALImage dest
            = driver->CreateCopy(imageryPath.c_str(), imagery.image.data(), FALSE, nullptr, nullptr, nullptr);

        // keep track of file name for terrain conversion. We use imagery as texture for terrain
        _imagery.insert({imagery.tile, imageryRelativePath.string()});
    }
}

void CDBTo3DTiles::SaveGeoCellConversions(const CDBGeoCell &geoCell,
                                          const boost::filesystem::path &outputDirectory)
{
    // Save Terrain
    auto terrainRootIt = _terrainRootTiles.find(geoCell);
    const auto &terrainTiles = _terrainTiles.find(geoCell);
    if (terrainRootIt != _terrainRootTiles.end() && terrainTiles != _terrainTiles.end()) {
        nlohmann::json tileset;
        tileset["asset"] = {{"version", "1.0"}};
        tileset["root"] = nlohmann::json::object();
        ConvertTerrainTileToJson(terrainTiles->second[terrainRootIt->second],
                                 terrainTiles->second,
                                 tileset["root"]);

        tileset["geometricError"] = tileset["root"]["geometricError"];

        auto tilesetPath = outputDirectory / TERRAIN_PATH / "tileset.json";
        std::ofstream fs(tilesetPath.string());
        fs << tileset << std::endl;
    }
}

void CDBTo3DTiles::CreateTerrainBufferAndAccessor(tinygltf::Model &terrainModel,
                                                  void *destBuffer,
                                                  const void *sourceBuffer,
                                                  size_t bufferIndex,
                                                  size_t bufferViewOffset,
                                                  size_t bufferViewLength,
                                                  int bufferViewTarget,
                                                  size_t accessorComponentCount,
                                                  int accessorComponentType,
                                                  int accessorType)
{
    std::memcpy(destBuffer, sourceBuffer, bufferViewLength);

    tinygltf::BufferView bufferView;
    bufferView.buffer = bufferIndex;
    bufferView.byteOffset = bufferViewOffset;
    bufferView.byteLength = bufferViewLength;
    bufferView.target = bufferViewTarget;

    tinygltf::Accessor accessor;
    accessor.bufferView = terrainModel.bufferViews.size();
    accessor.byteOffset = 0;
    accessor.count = accessorComponentCount;
    accessor.componentType = accessorComponentType;
    accessor.type = accessorType;

    terrainModel.bufferViews.emplace_back(bufferView);
    terrainModel.accessors.emplace_back(accessor);
}

tinygltf::Model CDBTo3DTiles::CreateTerrainGltf(const CDBTerrain &terrain)
{
    tinygltf::Model terrainModel;
    terrainModel.asset.version = "2.0";

    const auto &terrainMesh = terrain.mesh;
    auto center = terrainMesh.boundBox.center();
    auto positionMin = terrainMesh.boundBox.min - center;
    auto positionMax = terrainMesh.boundBox.max - center;
    size_t totalBufferSize = terrainMesh.indices.size() * sizeof(uint32_t)
                             + terrainMesh.positionsRtc.size() * sizeof(glm::vec3)
                             + terrainMesh.normals.size() * sizeof(glm::vec3)
                             + terrainMesh.uv.size() * sizeof(glm::vec2);

    tinygltf::Buffer buffer;
    auto &bufferData = buffer.data;
    bufferData.resize(totalBufferSize);

    tinygltf::Primitive primitive;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    auto bufferIndex = terrainModel.buffers.size();

    // copy  indices
    size_t offset = 0;
    size_t nextSize = terrainMesh.indices.size() * sizeof(uint32_t);
    CreateTerrainBufferAndAccessor(terrainModel,
                                   bufferData.data() + offset,
                                   terrainMesh.indices.data(),
                                   bufferIndex,
                                   offset,
                                   nextSize,
                                   TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER,
                                   terrainMesh.indices.size(),
                                   TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT,
                                   TINYGLTF_TYPE_SCALAR);

    primitive.indices = terrainModel.accessors.size() - 1;

    // copy positions
    offset += nextSize;
    nextSize = terrainMesh.positionsRtc.size() * sizeof(glm::vec3);
    CreateTerrainBufferAndAccessor(terrainModel,
                                   bufferData.data() + offset,
                                   terrainMesh.positionsRtc.data(),
                                   bufferIndex,
                                   offset,
                                   nextSize,
                                   TINYGLTF_TARGET_ARRAY_BUFFER,
                                   terrainMesh.positionsRtc.size(),
                                   TINYGLTF_COMPONENT_TYPE_FLOAT,
                                   TINYGLTF_TYPE_VEC3);

    auto &positionsAccessor = terrainModel.accessors.back();
    positionsAccessor.minValues = {positionMin.x, positionMin.y, positionMin.z};
    positionsAccessor.maxValues = {positionMax.x, positionMax.y, positionMax.z};

    primitive.attributes["POSITION"] = terrainModel.accessors.size() - 1;

    // copy normals
    offset += nextSize;
    nextSize = terrainMesh.normals.size() * sizeof(glm::vec3);
    CreateTerrainBufferAndAccessor(terrainModel,
                                   bufferData.data() + offset,
                                   terrainMesh.normals.data(),
                                   bufferIndex,
                                   offset,
                                   nextSize,
                                   TINYGLTF_TARGET_ARRAY_BUFFER,
                                   terrainMesh.normals.size(),
                                   TINYGLTF_COMPONENT_TYPE_FLOAT,
                                   TINYGLTF_TYPE_VEC3);

    primitive.attributes["NORMAL"] = terrainModel.accessors.size() - 1;

    // copy uv
    offset += nextSize;
    nextSize = terrainMesh.uv.size() * sizeof(glm::vec2);
    CreateTerrainBufferAndAccessor(terrainModel,
                                   bufferData.data() + offset,
                                   terrainMesh.uv.data(),
                                   bufferIndex,
                                   offset,
                                   nextSize,
                                   TINYGLTF_TARGET_ARRAY_BUFFER,
                                   terrainMesh.uv.size(),
                                   TINYGLTF_COMPONENT_TYPE_FLOAT,
                                   TINYGLTF_TYPE_VEC2);

    primitive.attributes["TEXCOORD_0"] = terrainModel.accessors.size() - 1;

    // add buffer to terrain model
    terrainModel.buffers.emplace_back(buffer);

    // material
    auto imageryPath = _imagery.find(terrain.tile);
    if (imageryPath != _imagery.end()) {
        tinygltf::Sampler sampler;
        sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
        sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
        terrainModel.samplers.emplace_back(sampler);

        tinygltf::Image image;
        image.uri = imageryPath->second;
        terrainModel.images.emplace_back(image);

        tinygltf::Texture texture;
        texture.sampler = terrainModel.samplers.size() - 1;
        texture.source = terrainModel.images.size() - 1;
        terrainModel.textures.emplace_back(texture);

        tinygltf::Material material;
        material.pbrMetallicRoughness.baseColorTexture.index = terrainModel.textures.size() - 1;
        material.pbrMetallicRoughness.baseColorFactor = {1.0, 1.0, 1.0, 1.0};
        material.pbrMetallicRoughness.roughnessFactor = 1.0;
        material.pbrMetallicRoughness.metallicFactor = 0.0;
        terrainModel.materials.emplace_back(material);

        primitive.material = terrainModel.materials.size() - 1;
    }

    // add mesh
    tinygltf::Mesh mesh;
    mesh.primitives.emplace_back(primitive);
    terrainModel.meshes.emplace_back(mesh);

    // create nodes
    tinygltf::Node rootNode;
    rootNode.matrix = {1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
    rootNode.children = {1};
    terrainModel.nodes.emplace_back(rootNode);

    tinygltf::Node meshNode;
    meshNode.name = "RTC_CENTER", meshNode.mesh = 0;
    meshNode.translation = {center.x, center.y, center.z};
    terrainModel.nodes.emplace_back(meshNode);

    // create scene
    tinygltf::Scene scene;
    scene.nodes.emplace_back(0);
    terrainModel.scenes.emplace_back(scene);

    return terrainModel;
}

void CDBTo3DTiles::WriteTerrainToB3DM(const CDBTerrain &terrain, const boost::filesystem::path &output)
{
    // create feature table
    std::string featureTableString = "{\"BATCH_LENGTH\":0}  "; // Built in padding

    // create glb
    std::stringstream ss;
    tinygltf::Model terrainGltf = CreateTerrainGltf(terrain);
    tinygltf::TinyGLTF gltfIO;
    gltfIO.WriteGltfSceneToStream(&terrainGltf, ss, false, true);

    // put glb into buffer
    ss.seekp(0, std::ios::end);
    std::stringstream::pos_type offset = ss.tellp();
    std::vector<uint8_t> glbBuffer(offset, 0);
    ss.read(reinterpret_cast<char *>(glbBuffer.data()), offset);

    // create header
    B3dmHeader header;
    header.magic[0] = 'b';
    header.magic[1] = '3';
    header.magic[2] = 'd';
    header.magic[3] = 'm';
    header.version = 1;
    header.byteLength = sizeof(header) + featureTableString.size() + glbBuffer.size();
    header.featureTableJsonByteLength = featureTableString.size();
    header.featureTableBinByteLength = 0;
    header.batchTableJsonByteLength = 0;
    header.batchTableBinByteLength = 0;

    std::ofstream fs(output.string(), std::ios::binary);
    fs.write(reinterpret_cast<const char *>(&header), sizeof(B3dmHeader));
    fs.write(featureTableString.data(), featureTableString.size());
    fs.write(reinterpret_cast<const char *>(glbBuffer.data()), glbBuffer.size());
}

glm::ivec2 CDBTo3DTiles::GetQuadtreeRelativeChild(const CDBTo3DTiles::Tile &tile,
                                                  const CDBTo3DTiles::Tile &root)
{
    double powerOf2 = glm::pow(2, tile.level - root.level - 1);
    size_t x = tile.x / powerOf2 - root.x * 2;
    size_t y = tile.y / powerOf2 - root.y * 2;
    return {x, y};
}

void CDBTo3DTiles::InsertTerrainTileToTileset(const CDBTo3DTiles::Tile &insert,
                                              int rootIndex,
                                              std::vector<CDBTo3DTiles::Tile> &tiles)
{
    // we are at the right level here. Just copy the data over
    if (insert.level == tiles[rootIndex].level && insert.x == tiles[rootIndex].x
        && insert.y == tiles[rootIndex].y) {
        tiles[rootIndex].x = insert.x;
        tiles[rootIndex].y = insert.y;
        tiles[rootIndex].level = insert.level;
        tiles[rootIndex].region = insert.region;
        tiles[rootIndex].contentUri = insert.contentUri;
        return;
    }

    // root has only one child if its level < 0
    if (tiles[rootIndex].level < 0) {
        if (tiles[rootIndex].children.empty()) {
            tiles.emplace_back(tiles[rootIndex].x,
                               tiles[rootIndex].y,
                               tiles[rootIndex].level + 1,
                               tiles[rootIndex].geometricError / 2.0,
                               BoundRegion(),
                               "");
            tiles[rootIndex].children.emplace_back(tiles.size() - 1);
        }

        InsertTerrainTileToTileset(insert, tiles[rootIndex].children.back(), tiles);
        return;
    }

    // For positive level, root is a quadtree.
    // So we determine which child we will insert the tile to
    if (tiles[rootIndex].children.size() < 4) {
        tiles[rootIndex].children.resize(4, -1);
    }

    glm::ivec2 relativeChild = GetQuadtreeRelativeChild(insert, tiles[rootIndex]);
    size_t childIdx = relativeChild.y * 2 + relativeChild.x;
    if (tiles[rootIndex].children[childIdx] == -1) {
        tiles.emplace_back(relativeChild.x + 2 * tiles[rootIndex].x,
                           relativeChild.y + 2 * tiles[rootIndex].y,
                           tiles[rootIndex].level + 1,
                           tiles[rootIndex].geometricError / 2.0,
                           BoundRegion(),
                           "");
        tiles[rootIndex].children[childIdx] = tiles.size() - 1;
    }

    InsertTerrainTileToTileset(insert, tiles[rootIndex].children[childIdx], tiles);
}

void CDBTo3DTiles::ConvertTerrainTileToJson(const CDBTo3DTiles::Tile &tile,
                                            const std::vector<Tile> &tiles,
                                            nlohmann::json &json)
{
    json["geometricError"] = tile.geometricError;
    json["refine"] = "REPLACE";
    json["boundingVolume"] = {{"region",
                               {
                                   tile.region.west,
                                   tile.region.south,
                                   tile.region.east,
                                   tile.region.north,
                                   tile.region.minHeight,
                                   tile.region.maxHeight,
                               }}};

    if (!tile.contentUri.empty()) {
        json["content"] = nlohmann::json::object();
        json["content"]["uri"] = tile.contentUri;
    }

    for (auto child : tile.children) {
        if (child == -1) {
            continue;
        }

        nlohmann::json childJson = nlohmann::json::object();
        ConvertTerrainTileToJson(tiles[child], tiles, childJson);
        json["children"].emplace_back(childJson);
    }
}

size_t CDBTo3DTiles::CDBGeoCellHash::operator()(const CDBGeoCell &geoCell) const
{
    using boost::hash_combine;
    using boost::hash_value;

    std::size_t seed = 0;
    hash_combine(seed, hash_value(geoCell.longitude));
    hash_combine(seed, hash_value(geoCell.latitude));

    return seed;
}

size_t CDBTo3DTiles::CDBTileHash::operator()(const CDBTile &tile) const
{
    using boost::hash_combine;
    using boost::hash_value;

    std::size_t seed = 0;
    hash_combine(seed, hash_value(tile.geoCell.longitude));
    hash_combine(seed, hash_value(tile.geoCell.latitude));
    hash_combine(seed, hash_value(tile.level));
    hash_combine(seed, hash_value(tile.x));
    hash_combine(seed, hash_value(tile.y));

    return seed;
}

bool CDBTo3DTiles::CDBTileEqual::operator()(const CDBTile &lhs, const CDBTile &rhs) const noexcept
{
    return (lhs.geoCell.longitude == rhs.geoCell.longitude) && (lhs.geoCell.latitude == rhs.geoCell.latitude)
           && (lhs.level == rhs.level) && (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

bool CDBTo3DTiles::CDBGeoCellEqual::operator()(const CDBGeoCell &lhs, const CDBGeoCell &rhs) const noexcept
{
    return (lhs.longitude == rhs.longitude) && (lhs.latitude == rhs.latitude);
}
