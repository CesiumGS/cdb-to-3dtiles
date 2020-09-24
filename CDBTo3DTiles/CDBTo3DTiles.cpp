#include "CDBTo3DTiles.h"
#include "CDB.h"
#include "Ellipsoid.h"
#include "boost/functional/hash.hpp"
#include "glm/gtx/transform.hpp"
#include "zip.h"
#include <fstream>
#include <iostream>
#include <utility>

const boost::filesystem::path CDBTo3DTiles::TERRAIN_PATH = "Terrains";
const boost::filesystem::path CDBTo3DTiles::IMAGERY_PATH = "Imagery";
const boost::filesystem::path CDBTo3DTiles::GSMODEL_PATH = "GSModels";
const boost::filesystem::path CDBTo3DTiles::GSMODEL_TEXTURE_PATH = "GSTextures";

void CDBTo3DTiles::Convert(const boost::filesystem::path &CDBPath, const boost::filesystem::path &output)
{
    auto terrainDirectory = output / TERRAIN_PATH;
    auto imageryDirectory = output / TERRAIN_PATH / IMAGERY_PATH;
    auto GSModelDirectory = output / GSMODEL_PATH;
    auto GSModelTexturesDirectory = output / GSMODEL_PATH / GSMODEL_TEXTURE_PATH;
    boost::filesystem::create_directories(terrainDirectory);
    boost::filesystem::create_directories(imageryDirectory);
    boost::filesystem::create_directories(GSModelDirectory);
    boost::filesystem::create_directories(GSModelTexturesDirectory);

    CDB cdb(CDBPath);
    cdb.ForEachGeoCell([&](const boost::filesystem::path &geoCellPath, const CDBGeoCell &geoCell) {
        //        // convert imagery
        //        cdb.ForEachImageryTile(geoCellPath,
        //                               [&](CDBImagery &imagery) { ConvertImagery(imagery, imageryDirectory); });

        //        // convert terrain
        //        cdb.ForEachElevationTile(geoCellPath, [&](const CDBTerrain &terrain) {
        //            ConvertTerrain(terrain, terrainDirectory);
        //        });

        // convert GSModel
        cdb.ForEachGSModelTile(geoCellPath,
                               [&](CDBGSModel &model) { ConvertGSModel(model, GSModelDirectory); });

        SaveGeoCellConversions(geoCell, output);
    });
}

void CDBTo3DTiles::ConvertGSModel(const CDBGSModel &model,
                                  const boost::filesystem::path &outputGSModelDirectory)
{
    auto GSModelB3DM = model.tile.encodedCDBName + ".b3dm";
    auto b3dmFullPath = outputGSModelDirectory / GSModelB3DM;
    auto textureDirectory = outputGSModelDirectory / GSMODEL_TEXTURE_PATH;
    tinygltf::Model modelGltf = CreateGSModelGltf(model, textureDirectory);
    WriteToB3DM(&modelGltf, b3dmFullPath.string());

    auto rootIt = _modelRootTiles.find(model.tile.geoCell);
    auto &tiles = _modelTiles[model.tile.geoCell];
    int rootIndex;
    if (rootIt == _modelRootTiles.end()) {
        tiles.emplace_back(0, 0, -10, CDB::CalculateTileExtent(model.tile.geoCell, -10, 0, 0), "");
        rootIndex = tiles.size() - 1;
        _modelRootTiles.insert({model.tile.geoCell, rootIndex});
    } else {
        rootIndex = rootIt->second;
    }

    Tile tile(model.tile.x, model.tile.y, model.tile.level, model.region, GSModelB3DM);
    InsertTileToTileset(model.tile.geoCell, tile, rootIndex, tiles);
}

void CDBTo3DTiles::ConvertTerrain(const CDBTerrain &terrain,
                                  const boost::filesystem::path &outputTerrainDirectory)
{
    // create gltf file
    auto terrainB3DM = terrain.tile.encodedCDBName + ".b3dm";
    auto b3dmFullPath = outputTerrainDirectory / terrainB3DM;
    tinygltf::Model terrainGltf = CreateTerrainGltf(terrain);
    WriteToB3DM(&terrainGltf, b3dmFullPath.string());

    auto rootIt = _terrainRootTiles.find(terrain.tile.geoCell);
    auto &tiles = _terrainTiles[terrain.tile.geoCell];
    int rootIndex;
    if (rootIt == _terrainRootTiles.end()) {
        tiles.emplace_back(0, 0, -10, CDB::CalculateTileExtent(terrain.tile.geoCell, -10, 0, 0), "");
        rootIndex = tiles.size() - 1;
        _terrainRootTiles.insert({terrain.tile.geoCell, rootIndex});
    } else {
        rootIndex = rootIt->second;
    }

    Tile tile(terrain.tile.x, terrain.tile.y, terrain.tile.level, terrain.mesh.region, terrainB3DM);
    InsertTileToTileset(terrain.tile.geoCell, tile, rootIndex, tiles);
}

void CDBTo3DTiles::ConvertImagery(CDBImagery &imagery, const boost::filesystem::path &outputImageryDirectory)
{
    auto imageryFileName = imagery.tile.encodedCDBName + ".jpg";
    auto imageryRelativePath = IMAGERY_PATH / imageryFileName;
    auto imageryPath = outputImageryDirectory / imageryFileName;

    // save the content
    auto driver = (GDALDriver *) GDALGetDriverByName("JPEG");
    if (driver != nullptr) {
        GDALDatasetWrapper dest
            = driver->CreateCopy(imageryPath.c_str(), imagery.image.data(), FALSE, nullptr, nullptr, nullptr);

        // keep track of file name for terrain conversion. We use imagery as texture for terrain
        _imagery.insert({imagery.tile, imageryRelativePath.string()});
    }
}

void CDBTo3DTiles::SaveGeoCellConversions(const CDBGeoCell &geoCell,
                                          const boost::filesystem::path &outputDirectory)
{
    // Save Terrains
    auto terrainRootIt = _terrainRootTiles.find(geoCell);
    const auto &terrainTiles = _terrainTiles.find(geoCell);
    if (terrainRootIt != _terrainRootTiles.end() && terrainTiles != _terrainTiles.end()) {
        nlohmann::json tileset;
        tileset["asset"] = {{"version", "1.0"}};
        tileset["root"] = nlohmann::json::object();
        tileset["root"]["refine"] = "REPLACE";

        // TODO: calculate geometric error for the root
        ConvertTilesetToJson(terrainTiles->second[terrainRootIt->second],
                             313000.0f,
                             terrainTiles->second,
                             tileset["root"]);

        tileset["geometricError"] = tileset["root"]["geometricError"];

        auto tilesetPath = outputDirectory / TERRAIN_PATH / "tileset.json";
        std::ofstream fs(tilesetPath.string());
        fs << tileset << std::endl;
    }

    // Save Models
    auto modelRootIt = _modelRootTiles.find(geoCell);
    const auto &modelTiles = _modelTiles.find(geoCell);
    if (modelRootIt != _modelRootTiles.end() && modelTiles != _modelTiles.end()) {
        nlohmann::json tileset;
        tileset["asset"] = {{"version", "1.0"}};
        tileset["root"] = nlohmann::json::object();
        tileset["root"]["refine"] = "ADD";

        // TODO: calculate geometric error for the root
        ConvertTilesetToJson(modelTiles->second[modelRootIt->second],
                             313000.0f,
                             modelTiles->second,
                             tileset["root"]);

        tileset["geometricError"] = tileset["root"]["geometricError"];

        auto tilesetPath = outputDirectory / GSMODEL_PATH / "tileset.json";
        std::ofstream fs(tilesetPath.string());
        fs << tileset << std::endl;
    }
}

void CDBTo3DTiles::CreateBufferAndAccessor(tinygltf::Model &modelGltf,
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

    tinygltf::BufferView bufferViewGltf;
    bufferViewGltf.buffer = bufferIndex;
    bufferViewGltf.byteOffset = bufferViewOffset;
    bufferViewGltf.byteLength = bufferViewLength;
    bufferViewGltf.target = bufferViewTarget;

    tinygltf::Accessor accessorGltf;
    accessorGltf.bufferView = modelGltf.bufferViews.size();
    accessorGltf.byteOffset = 0;
    accessorGltf.count = accessorComponentCount;
    accessorGltf.componentType = accessorComponentType;
    accessorGltf.type = accessorType;

    modelGltf.bufferViews.emplace_back(bufferViewGltf);
    modelGltf.accessors.emplace_back(accessorGltf);
}

tinygltf::Model CDBTo3DTiles::CreateTerrainGltf(const CDBTerrain &terrain)
{
    tinygltf::Model terrainGltf;
    terrainGltf.asset.version = "2.0";

    const auto &terrainMesh = terrain.mesh;
    auto center = terrainMesh.boundBox.center();
    auto positionMin = terrainMesh.boundBox.min - center;
    auto positionMax = terrainMesh.boundBox.max - center;
    size_t totalBufferSize = terrainMesh.indices.size() * sizeof(uint32_t)
                             + terrainMesh.positionsRtc.size() * sizeof(glm::vec3)
                             + terrainMesh.normals.size() * sizeof(glm::vec3)
                             + terrainMesh.uv.size() * sizeof(glm::vec2);

    tinygltf::Buffer bufferGltf;
    auto &bufferData = bufferGltf.data;
    bufferData.resize(totalBufferSize);

    tinygltf::Primitive primitiveGltf;
    primitiveGltf.mode = TINYGLTF_MODE_TRIANGLES;

    auto bufferIndex = terrainGltf.buffers.size();

    // copy  indices
    size_t offset = 0;
    size_t nextSize = terrainMesh.indices.size() * sizeof(uint32_t);
    CreateBufferAndAccessor(terrainGltf,
                            bufferData.data() + offset,
                            terrainMesh.indices.data(),
                            bufferIndex,
                            offset,
                            nextSize,
                            TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER,
                            terrainMesh.indices.size(),
                            TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT,
                            TINYGLTF_TYPE_SCALAR);

    primitiveGltf.indices = terrainGltf.accessors.size() - 1;

    // copy positions
    offset += nextSize;
    nextSize = terrainMesh.positionsRtc.size() * sizeof(glm::vec3);
    CreateBufferAndAccessor(terrainGltf,
                            bufferData.data() + offset,
                            terrainMesh.positionsRtc.data(),
                            bufferIndex,
                            offset,
                            nextSize,
                            TINYGLTF_TARGET_ARRAY_BUFFER,
                            terrainMesh.positionsRtc.size(),
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            TINYGLTF_TYPE_VEC3);

    auto &positionsAccessor = terrainGltf.accessors.back();
    positionsAccessor.minValues = {positionMin.x, positionMin.y, positionMin.z};
    positionsAccessor.maxValues = {positionMax.x, positionMax.y, positionMax.z};

    primitiveGltf.attributes["POSITION"] = terrainGltf.accessors.size() - 1;

    // copy normals
    offset += nextSize;
    nextSize = terrainMesh.normals.size() * sizeof(glm::vec3);
    CreateBufferAndAccessor(terrainGltf,
                            bufferData.data() + offset,
                            terrainMesh.normals.data(),
                            bufferIndex,
                            offset,
                            nextSize,
                            TINYGLTF_TARGET_ARRAY_BUFFER,
                            terrainMesh.normals.size(),
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            TINYGLTF_TYPE_VEC3);

    primitiveGltf.attributes["NORMAL"] = terrainGltf.accessors.size() - 1;

    // copy uv
    offset += nextSize;
    nextSize = terrainMesh.uv.size() * sizeof(glm::vec2);
    CreateBufferAndAccessor(terrainGltf,
                            bufferData.data() + offset,
                            terrainMesh.uv.data(),
                            bufferIndex,
                            offset,
                            nextSize,
                            TINYGLTF_TARGET_ARRAY_BUFFER,
                            terrainMesh.uv.size(),
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            TINYGLTF_TYPE_VEC2);

    primitiveGltf.attributes["TEXCOORD_0"] = terrainGltf.accessors.size() - 1;

    // add buffer to terrain model
    terrainGltf.buffers.emplace_back(bufferGltf);

    // material
    auto imageryPath = _imagery.find(terrain.tile);
    if (imageryPath != _imagery.end()) {
        tinygltf::Sampler sampler;
        sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
        sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
        terrainGltf.samplers.emplace_back(sampler);

        tinygltf::Image image;
        image.uri = imageryPath->second;
        terrainGltf.images.emplace_back(image);

        tinygltf::Texture texture;
        texture.sampler = terrainGltf.samplers.size() - 1;
        texture.source = terrainGltf.images.size() - 1;
        terrainGltf.textures.emplace_back(texture);

        tinygltf::Material material;
        material.pbrMetallicRoughness.baseColorTexture.index = terrainGltf.textures.size() - 1;
        material.pbrMetallicRoughness.baseColorFactor = {1.0, 1.0, 1.0, 1.0};
        material.pbrMetallicRoughness.roughnessFactor = 1.0;
        material.pbrMetallicRoughness.metallicFactor = 0.0;
        terrainGltf.materials.emplace_back(material);

        primitiveGltf.material = terrainGltf.materials.size() - 1;
    }

    // add mesh
    tinygltf::Mesh meshGltf;
    meshGltf.primitives.emplace_back(primitiveGltf);
    terrainGltf.meshes.emplace_back(meshGltf);

    // create nodes
    tinygltf::Node rootNodeGltf;
    rootNodeGltf.matrix = {1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
    rootNodeGltf.children = {1};
    terrainGltf.nodes.emplace_back(rootNodeGltf);

    tinygltf::Node meshNodeGltf;
    meshNodeGltf.name = "RTC_CENTER", meshNodeGltf.mesh = 0;
    meshNodeGltf.translation = {center.x, center.y, center.z};
    terrainGltf.nodes.emplace_back(meshNodeGltf);

    // create scene
    tinygltf::Scene sceneGltf;
    sceneGltf.nodes.emplace_back(0);
    terrainGltf.scenes.emplace_back(sceneGltf);

    return terrainGltf;
}

void CDBTo3DTiles::WriteToB3DM(tinygltf::Model *gltf, const boost::filesystem::path &output)
{
    // create feature table
    std::string featureTableString = "{\"BATCH_LENGTH\":0}  "; // Built in padding

    // create glb
    std::stringstream ss;
    tinygltf::TinyGLTF gltfIO;
    gltfIO.WriteGltfSceneToStream(gltf, ss, false, true);

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

tinygltf::Model CDBTo3DTiles::CreateGSModelGltf(const CDBGSModel &model,
                                                const boost::filesystem::path &textureDirectory)
{
    tinygltf::Model GSModelGltf;
    GSModelGltf.asset.version = "2.0";

    // batch material with mesh
    size_t totalBufferSize = 0;
    auto WG84 = Ellipsoid::WGS84();
    auto tileBoundRegion = CDB::CalculateTileExtent(model.tile.geoCell,
                                                    model.tile.level,
                                                    model.tile.x,
                                                    model.tile.y);
    auto tileCenterCart = Cartographic((tileBoundRegion.east + tileBoundRegion.west) / 2.0f,
                                       (tileBoundRegion.north + tileBoundRegion.south) / 2.0f,
                                       0.0f);
    glm::dvec3 tileCenter = WG84.CartographicToCartesian(tileCenterCart);
    std::unordered_set<MaterialTexturePair, MaterialTexturePairHash, MaterialTexturePairEqual> materialToMesh;
    for (size_t i = 0; i < model.scenes.size(); ++i) {
        const auto &scene = model.scenes[i];
        auto scenePosition = model.positions[i];
        glm::vec3 up = WG84.GeodeticSurfaceNormalFromCartesian(scenePosition);
        glm::vec3 east = glm::normalize(glm::vec3(-scenePosition.y, scenePosition.x, 0.0));
        glm::vec3 north = glm::cross(up, east);
        glm::mat4 ENU = glm::rotate(glm::mat4(1.0f), glm::radians<float>(model.angleOfOrientations[i]), up);
        ENU = ENU
              * glm::mat4(glm::vec4(east, 0.0f),
                          glm::vec4(north, 0.0f),
                          glm::vec4(up, 0.0f),
                          glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        for (const auto &mesh : scene.meshes) {
            totalBufferSize += mesh.indices.size() * sizeof(uint32_t)
                               + mesh.positions.size() * sizeof(glm::vec3)
                               + mesh.normals.size() * sizeof(glm::vec3)
                               + mesh.uvs.size() * sizeof(glm::vec2);

            Material material;
            Texture texture;
            if (mesh.material >= 0) {
                material = scene.materials[mesh.material];
            }

            if (mesh.texture >= 0) {
                texture = scene.textures[mesh.texture];
            }

            auto pairIt = materialToMesh.insert({material, texture});
            auto &materialTextureIt = pairIt.first;

            for (size_t j = 0; j < mesh.positions.size(); ++j) {
                glm::vec3 worldPosition = glm::vec3(ENU * glm::vec4(mesh.positions[j], 1.0f))
                                          + static_cast<glm::vec3>(model.positions[i] - tileCenter);
                materialTextureIt->boundBox.merge(worldPosition);
                materialTextureIt->positionsRTC.emplace_back(worldPosition);
            }

            std::copy(mesh.normals.begin(),
                      mesh.normals.end(),
                      std::back_inserter(materialTextureIt->normals));

            std::copy(mesh.uvs.begin(), mesh.uvs.end(), std::back_inserter(materialTextureIt->uvs));
        }
    }

    // create buffer for all scenes
    tinygltf::Buffer bufferGltf;
    auto &bufferData = bufferGltf.data;
    bufferData.resize(totalBufferSize);
    GSModelGltf.buffers.emplace_back(std::move(bufferGltf));

    // create nodes
    tinygltf::Node rootNodeGltf;
    rootNodeGltf.matrix = {1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
    rootNodeGltf.children = {1};
    GSModelGltf.nodes.emplace_back(rootNodeGltf);

    tinygltf::Node meshNodeGltf;
    meshNodeGltf.name = "RTC_CENTER", meshNodeGltf.mesh = 0;
    meshNodeGltf.translation = {tileCenter.x, tileCenter.y, tileCenter.z};
    GSModelGltf.nodes.emplace_back(meshNodeGltf);

    // add material and texture
    std::unordered_map<std::string, int> modelTextures;
    AddCDBMaterialToGltf(textureDirectory, materialToMesh, modelTextures, GSModelGltf);

    // add scene to Gltf
    AddCDBSceneToGltf(materialToMesh, GSModelGltf);

    // create scene
    tinygltf::Scene sceneGltf;
    sceneGltf.nodes.emplace_back(0);
    GSModelGltf.scenes.emplace_back(sceneGltf);

    return GSModelGltf;
}

void CDBTo3DTiles::AddCDBMaterialToGltf(
    const boost::filesystem::path &textureDirectory,
    std::unordered_set<MaterialTexturePair, MaterialTexturePairHash, MaterialTexturePairEqual>
        &materialTextures,
    std::unordered_map<std::string, int> &modelTextures,
    tinygltf::Model &GSModelGltf)
{
    // all texture use the same sampler
    tinygltf::Sampler sampler;
    GSModelGltf.samplers.emplace_back(sampler);

    auto textureDirectoryFilename = textureDirectory.filename();
    for (auto &materialTexture : materialTextures) {
        const auto &texture = materialTexture.texture;
        if (!texture.path.empty()) {
            auto textureFilename = (texture.path.stem().string() + ".jpg");
            auto textureOutputPath = textureDirectory / textureFilename;
            auto texturePathString = texture.path.string();
            if (_writtenTextures.find(texturePathString) == _writtenTextures.end()) {
                GDALDatasetWrapper image = (GDALDataset *) GDALOpen(texture.path.c_str(), GA_ReadOnly);
                auto driver = (GDALDriver *) GDALGetDriverByName("JPEG");
                if (driver && image.data()) {
                    GDALDatasetWrapper dest = driver->CreateCopy(textureOutputPath.c_str(),
                                                                 image.data(),
                                                                 FALSE,
                                                                 nullptr,
                                                                 nullptr,
                                                                 nullptr);

                    _writtenTextures.insert(texturePathString);
                }
            }

            if (_writtenTextures.find(texturePathString) != _writtenTextures.end()
                && modelTextures.find(texturePathString) == modelTextures.end()) {
                tinygltf::Image GltfImage;
                GltfImage.uri = (textureDirectoryFilename / textureFilename).string();
                GSModelGltf.images.emplace_back(GltfImage);

                tinygltf::Texture GltfTexture;
                GltfTexture.sampler = GSModelGltf.samplers.size() - 1;
                GltfTexture.source = GSModelGltf.images.size() - 1;
                GSModelGltf.textures.emplace_back(GltfTexture);

                modelTextures.insert({texturePathString, GSModelGltf.textures.size() - 1});
            }
        }

        const auto &material = materialTexture.material;
        glm::vec3 specularColor = material.specular;
        float specularIntensity = specularColor.r * 0.2125 + specularColor.g * 0.7154
                                  + specularColor.b * 0.0721;

        float roughnessFactor = material.shininess;
        roughnessFactor = material.shininess / 1000.0;
        roughnessFactor = 1.0 - roughnessFactor;
        roughnessFactor = glm::clamp(roughnessFactor, 0.0f, 1.0f);

        if (specularIntensity < 0.0) {
            roughnessFactor *= (1.0 - specularIntensity);
        }

        tinygltf::Material materialGltf;
        auto textureIdx = modelTextures.find(texture.path.string());
        if (textureIdx != modelTextures.end()) {
            materialGltf.pbrMetallicRoughness.baseColorTexture.index = textureIdx->second;
        }

        materialGltf.pbrMetallicRoughness.baseColorFactor = {material.diffuse.r,
                                                             material.diffuse.g,
                                                             material.diffuse.b,
                                                             1.0};
        materialGltf.pbrMetallicRoughness.roughnessFactor = roughnessFactor;
        materialGltf.pbrMetallicRoughness.metallicFactor = 0.0;
        GSModelGltf.materials.emplace_back(materialGltf);

        materialTexture.materialGltfIndex = GSModelGltf.materials.size() - 1;
    }
}

glm::ivec2 CDBTo3DTiles::GetQuadtreeRelativeChild(const CDBTo3DTiles::Tile &tile,
                                                  const CDBTo3DTiles::Tile &root)
{
    double powerOf2 = glm::pow(2, tile.level - root.level - 1);
    size_t x = tile.x / powerOf2 - root.x * 2;
    size_t y = tile.y / powerOf2 - root.y * 2;
    return {x, y};
}

void CDBTo3DTiles::InsertTileToTileset(const CDBGeoCell &geoCell,
                                       const CDBTo3DTiles::Tile &insert,
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
        std::copy(insert.children.begin(),
                  insert.children.end(),
                  std::back_inserter(tiles[rootIndex].children));
        return;
    }

    // root has only one child if its level < 0
    if (tiles[rootIndex].level < 0) {
        if (tiles[rootIndex].children.empty()) {
            int childX = tiles[rootIndex].x;
            int childY = tiles[rootIndex].y;
            int childLevel = tiles[rootIndex].level + 1;
            tiles.emplace_back(childX,
                               childY,
                               childLevel,
                               CDB::CalculateTileExtent(geoCell, childLevel, childX, childY),
                               "");
            tiles[rootIndex].children.emplace_back(tiles.size() - 1);
        }

        InsertTileToTileset(geoCell, insert, tiles[rootIndex].children.back(), tiles);
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
        int childX = relativeChild.x + 2 * tiles[rootIndex].x;
        int childY = relativeChild.y + 2 * tiles[rootIndex].y;
        int childLevel = tiles[rootIndex].level + 1;
        tiles.emplace_back(childX,
                           childY,
                           childLevel,
                           CDB::CalculateTileExtent(geoCell, childLevel, childX, childY),
                           "");
        tiles[rootIndex].children[childIdx] = tiles.size() - 1;
    }

    InsertTileToTileset(geoCell, insert, tiles[rootIndex].children[childIdx], tiles);
}

void CDBTo3DTiles::ConvertTilesetToJson(const CDBTo3DTiles::Tile &tile,
                                        float geometricError,
                                        const std::vector<Tile> &tiles,
                                        nlohmann::json &json)
{
    json["geometricError"] = geometricError;
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
        ConvertTilesetToJson(tiles[child], geometricError / 2.0f, tiles, childJson);
        json["children"].emplace_back(childJson);
    }
}

void CDBTo3DTiles::AddCDBSceneToGltf(
    std::unordered_set<MaterialTexturePair, MaterialTexturePairHash, MaterialTexturePairEqual>
        &materialTextures,
    tinygltf::Model &GSModelGltf)
{
    // create next node
    auto &bufferGltf = GSModelGltf.buffers.front();
    auto &bufferData = bufferGltf.data;
    size_t bufferOffset = 0;
    size_t bufferIndex = GSModelGltf.buffers.size() - 1;
    size_t nodeOffset = GSModelGltf.nodes.size();
    size_t index = 0;
    size_t nextSize = 0;
    for (const auto &materialTexture : materialTextures) {
        tinygltf::Primitive primitiveGltf;
        primitiveGltf.material = materialTexture.materialGltfIndex;
        primitiveGltf.mode = TINYGLTF_MODE_TRIANGLES;

        // copy positions
        if (materialTexture.positionsRTC.size() > 0) {
            bufferOffset += nextSize;
            nextSize = materialTexture.positionsRTC.size() * sizeof(glm::vec3);
            CreateBufferAndAccessor(GSModelGltf,
                                    bufferData.data() + bufferOffset,
                                    materialTexture.positionsRTC.data(),
                                    bufferIndex,
                                    bufferOffset,
                                    nextSize,
                                    TINYGLTF_TARGET_ARRAY_BUFFER,
                                    materialTexture.positionsRTC.size(),
                                    TINYGLTF_COMPONENT_TYPE_FLOAT,
                                    TINYGLTF_TYPE_VEC3);

            auto &positionsAccessor = GSModelGltf.accessors.back();
            const auto &boundBox = materialTexture.boundBox;
            positionsAccessor.minValues = {boundBox.min.x, boundBox.min.y, boundBox.min.z};
            positionsAccessor.maxValues = {boundBox.max.x, boundBox.max.y, boundBox.max.z};

            primitiveGltf.attributes["POSITION"] = GSModelGltf.accessors.size() - 1;
        }

        // copy normals
        if (materialTexture.normals.size() > 0) {
            bufferOffset += nextSize;
            nextSize = materialTexture.normals.size() * sizeof(glm::vec3);
            CreateBufferAndAccessor(GSModelGltf,
                                    bufferData.data() + bufferOffset,
                                    materialTexture.normals.data(),
                                    bufferIndex,
                                    bufferOffset,
                                    nextSize,
                                    TINYGLTF_TARGET_ARRAY_BUFFER,
                                    materialTexture.normals.size(),
                                    TINYGLTF_COMPONENT_TYPE_FLOAT,
                                    TINYGLTF_TYPE_VEC3);

            primitiveGltf.attributes["NORMAL"] = GSModelGltf.accessors.size() - 1;
        }

        // copy uv
        if (materialTexture.uvs.size() > 0) {
            bufferOffset += nextSize;
            nextSize = materialTexture.uvs.size() * sizeof(glm::vec2);
            CreateBufferAndAccessor(GSModelGltf,
                                    bufferData.data() + bufferOffset,
                                    materialTexture.uvs.data(),
                                    bufferIndex,
                                    bufferOffset,
                                    nextSize,
                                    TINYGLTF_TARGET_ARRAY_BUFFER,
                                    materialTexture.uvs.size(),
                                    TINYGLTF_COMPONENT_TYPE_FLOAT,
                                    TINYGLTF_TYPE_VEC2);

            primitiveGltf.attributes["TEXCOORD_0"] = GSModelGltf.accessors.size() - 1;
        }

        // add mesh
        tinygltf::Mesh meshGltf;
        meshGltf.primitives.emplace_back(primitiveGltf);
        GSModelGltf.meshes.emplace_back(meshGltf);

        // create node
        tinygltf::Node nodeGltf;
        nodeGltf.mesh = GSModelGltf.meshes.size() - 1;
        GSModelGltf.nodes.emplace_back(nodeGltf);

        // add to root node
        auto &rootNode = GSModelGltf.nodes[nodeOffset - 1];
        rootNode.children.emplace_back(nodeOffset + index);
        ++index;
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

size_t CDBTo3DTiles::MaterialTexturePairHash::operator()(const CDBTo3DTiles::MaterialTexturePair &key) const
{
    using boost::hash_combine;
    using boost::hash_value;

    std::size_t seed = 0;
    const auto &material = key.material;
    const auto &texturePath = key.texture.path.string();

    hash_combine(seed, hash_value(texturePath));

    hash_combine(seed, hash_value(material.ambient.r));
    hash_combine(seed, hash_value(material.ambient.g));
    hash_combine(seed, hash_value(material.ambient.b));

    hash_combine(seed, hash_value(material.diffuse.r));
    hash_combine(seed, hash_value(material.diffuse.g));
    hash_combine(seed, hash_value(material.diffuse.b));

    hash_combine(seed, hash_value(material.specular.r));
    hash_combine(seed, hash_value(material.specular.g));
    hash_combine(seed, hash_value(material.specular.b));

    hash_combine(seed, hash_value(material.emissive.r));
    hash_combine(seed, hash_value(material.emissive.g));
    hash_combine(seed, hash_value(material.emissive.b));

    hash_combine(seed, hash_value(material.shininess));
    hash_combine(seed, hash_value(material.alpha));

    return seed;
}

bool CDBTo3DTiles::MaterialTexturePairEqual::operator()(
    const CDBTo3DTiles::MaterialTexturePair &lhs, const CDBTo3DTiles::MaterialTexturePair &rhs) const noexcept
{
    return lhs.material.ambient == rhs.material.ambient && lhs.material.diffuse == rhs.material.diffuse
           && lhs.material.specular == rhs.material.specular && lhs.material.emissive == rhs.material.emissive
           && lhs.material.shininess == rhs.material.shininess && lhs.material.alpha == rhs.material.alpha
           && lhs.texture.path == rhs.texture.path;
}
