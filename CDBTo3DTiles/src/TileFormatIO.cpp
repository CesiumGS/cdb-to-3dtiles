#include "TileFormatIO.h"
#include "Ellipsoid.h"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtx/quaternion.hpp"
#include "nlohmann/json.hpp"

namespace CDBTo3DTiles {

static float MAX_GEOMETRIC_ERROR = 300000.0f;
static std::string CDB_CLASS_NAME = "CDBClass";
static std::string CDB_FEATURE_TABLE_NAME = "CDBFeatureTable";

static void createBatchTable(const CDBInstancesAttributes *instancesAttribs,
                             std::string &batchTableJson,
                             std::vector<uint8_t> &batchTableBuffer);
static void convertTilesetToJson(const CDBTile &tile, float geometricError, nlohmann::json &json);
static bool ParseJsonAsValue(tinygltf::Value *ret, const nlohmann::json &o);

void combineTilesetJson(const std::vector<std::filesystem::path> &tilesetJsonPaths,
                        const std::vector<Core::BoundingRegion> &regions,
                        std::ofstream &fs,
                        bool use3dTilesNext)
{
    nlohmann::json tilesetJson;
    tilesetJson["asset"] = {{"version", "1.0"}};
    tilesetJson["geometricError"] = MAX_GEOMETRIC_ERROR;
    tilesetJson["root"] = nlohmann::json::object();
    tilesetJson["root"]["refine"] = "ADD";
    tilesetJson["root"]["geometricError"] = MAX_GEOMETRIC_ERROR;

    if (use3dTilesNext) {
        tilesetJson["extensionsUsed"] = nlohmann::json::array({"3DTILES_content_gltf"});
        tilesetJson["extensionsRequired"] = nlohmann::json::array({"3DTILES_content_gltf"});
        tilesetJson["extensions"]["3DTILES_content_gltf"]["extensionsUsed"] = nlohmann::json::array({"EXT_feature_metadata"});
    } 

    auto rootChildren = nlohmann::json::array();
    auto rootRegion = regions.front();
    for (size_t i = 0; i < tilesetJsonPaths.size(); ++i) {
        const auto &path = tilesetJsonPaths[i];
        const auto &childBoundRegion = regions[i];
        const auto &childRectangle = childBoundRegion.getRectangle();
        nlohmann::json childJson;
        childJson["geometricError"] = MAX_GEOMETRIC_ERROR;
        childJson["content"] = nlohmann::json::object();
        childJson["content"]["uri"] = path;
        childJson["boundingVolume"] = {{"region",
                                        {
                                            childRectangle.getWest(),
                                            childRectangle.getSouth(),
                                            childRectangle.getEast(),
                                            childRectangle.getNorth(),
                                            childBoundRegion.getMinimumHeight(),
                                            childBoundRegion.getMaximumHeight(),
                                        }}};

        rootChildren.emplace_back(childJson);
        rootRegion = rootRegion.computeUnion(childBoundRegion);
    }

    const auto &rootRectangle = rootRegion.getRectangle();
    tilesetJson["root"]["children"] = rootChildren;
    tilesetJson["root"]["boundingVolume"] = {{"region",
                                              {
                                                  rootRectangle.getWest(),
                                                  rootRectangle.getSouth(),
                                                  rootRectangle.getEast(),
                                                  rootRectangle.getNorth(),
                                                  rootRegion.getMinimumHeight(),
                                                  rootRegion.getMaximumHeight(),
                                              }}};

    fs << tilesetJson << std::endl;
}

void writeToTilesetJson(const CDBTileset &tileset, bool replace, std::ofstream &fs)
{
    nlohmann::json tilesetJson;
    tilesetJson["asset"] = {{"version", "1.0"}};
    tilesetJson["root"] = nlohmann::json::object();
    if (replace) {
        tilesetJson["root"]["refine"] = "REPLACE";
    } else {
        tilesetJson["root"]["refine"] = "ADD";
    }

    auto root = tileset.getRoot();
    if (root) {
        convertTilesetToJson(*root, MAX_GEOMETRIC_ERROR, tilesetJson["root"]);
        tilesetJson["geometricError"] = tilesetJson["root"]["geometricError"];
        fs << tilesetJson << std::endl;
    }
}

void createInstancingExtension([[maybe_unused]] tinygltf::Model *gltf,
                          const CDBModelsAttributes &modelsAttribs)
{
    const auto &cdbTile = modelsAttribs.getTile();
    [[maybe_unused]] const auto &instancesAttribs = modelsAttribs.getInstancesAttributes();
    const auto &cartographicPositions = modelsAttribs.getCartographicPositions();
    const auto &scales = modelsAttribs.getScales();
    const auto &orientation = modelsAttribs.getOrientations();

    size_t totalInstances = cartographicPositions.size();
    [[maybe_unused]] size_t totalTranslationSize = totalInstances * sizeof(glm::vec3);
    [[maybe_unused]] size_t totalRotationSize = totalInstances * sizeof(glm::vec3);
    [[maybe_unused]] size_t totalScaleSize = totalInstances * sizeof(glm::vec3);

    const auto &ellipsoid = Core::Ellipsoid::WGS84;
    const auto tileCenterCartographic = cdbTile.getBoundRegion().getRectangle().computeCenter();
    const auto tileCenterCartesian = ellipsoid.cartographicToCartesian(tileCenterCartographic);

    // Get existing glTF buffer.
    auto &bufferData = gltf->buffers[0].data;
    size_t originalBufferSize = bufferData.size();
    size_t bufferSize = bufferData.size();

    // Prepare bufferViews and accessors.
    tinygltf::BufferView translationBufferView;
    translationBufferView.buffer = 0;
    translationBufferView.byteOffset = bufferSize;
    translationBufferView.byteLength = TINYGLTF_TYPE_VEC3 * sizeof(float_t) * totalInstances;
    gltf->bufferViews.emplace_back(translationBufferView);
    bufferSize += translationBufferView.byteLength;
    tinygltf::Accessor translationAccessor;
    translationAccessor.count = totalInstances;
    translationAccessor.type = TINYGLTF_TYPE_VEC3;
    translationAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    translationAccessor.bufferView = static_cast<uint32_t>(gltf->bufferViews.size() - 1);
    const auto translationAccessorIndex = gltf->accessors.size();
    gltf->accessors.emplace_back(translationAccessor);
    

    tinygltf::BufferView rotationBufferView;
    rotationBufferView.buffer = 0;
    rotationBufferView.byteOffset = bufferSize;
    rotationBufferView.byteLength = TINYGLTF_TYPE_VEC4 * sizeof(float_t) * totalInstances;
    gltf->bufferViews.emplace_back(rotationBufferView);
    bufferSize += rotationBufferView.byteLength;
    tinygltf::Accessor rotationAccessor;
    rotationAccessor.count = totalInstances;
    rotationAccessor.type = TINYGLTF_TYPE_VEC4;
    rotationAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    rotationAccessor.bufferView = static_cast<uint32_t>(gltf->bufferViews.size() - 1);
    const auto rotationAccessorIndex = gltf->accessors.size();
    gltf->accessors.emplace_back(rotationAccessor);

    tinygltf::BufferView scaleBufferView;
    scaleBufferView.buffer = 0;
    scaleBufferView.byteOffset = bufferSize;
    scaleBufferView.byteLength = TINYGLTF_TYPE_VEC3 * sizeof(float_t) * totalInstances;
    gltf->bufferViews.emplace_back(scaleBufferView);
    bufferSize += scaleBufferView.byteLength;
    tinygltf::Accessor scaleAccessor;
    scaleAccessor.count = totalInstances;
    scaleAccessor.type = TINYGLTF_TYPE_VEC3;
    scaleAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    scaleAccessor.bufferView = static_cast<uint32_t>(gltf->bufferViews.size() - 1);
    const auto scaleAccessorIndex = gltf->accessors.size();
    gltf->accessors.emplace_back(scaleAccessor);

    bufferData.resize(bufferSize);

    // Iterate through instances.
    for (size_t i = 0; i < totalInstances; ++i) {
        glm::dvec3 positionCartesian = ellipsoid.cartographicToCartesian(cartographicPositions[i]);
        glm::fvec3 rtcPositionCartesian = glm::fvec3(positionCartesian - tileCenterCartesian);
        glm::dmat4 rotationMatrix = calculateModelOrientation(positionCartesian, orientation[i]);
        glm::fquat quaternion = glm::normalize(glm::quat_cast(rotationMatrix));
        glm::fvec3 scale = scales[i];

        auto translationOffset = originalBufferSize + (i * sizeof(glm::vec3));
        auto rotationOffset = originalBufferSize + translationBufferView.byteLength + (i * sizeof(glm::vec4));
        auto scaleOffset = originalBufferSize + translationBufferView.byteLength + rotationBufferView.byteLength + (i * sizeof(glm::vec3));

        std::memcpy(bufferData.data() + translationOffset, &rtcPositionCartesian[0], sizeof(glm::vec3));
        std::memcpy(bufferData.data() + rotationOffset, &quaternion[0], sizeof(glm::vec4));
        std::memcpy(bufferData.data() + scaleOffset, &scale[0], sizeof(glm::vec3));
    }

    // Create EXT_mesh_gpu_instancing JSON.
    nlohmann::json instancingExtension;
    instancingExtension["attributes"]["TRANSLATION"] = translationAccessorIndex;
    instancingExtension["attributes"]["ROTATION"] = rotationAccessorIndex;
    instancingExtension["attributes"]["SCALE"] = scaleAccessorIndex;
    
    // Add EXT_mesh_gpu_instancing to mesh.
    tinygltf::Value instancingExtensionValue;
    CDBTo3DTiles::ParseJsonAsValue(&instancingExtensionValue, instancingExtension);
    // TODO: Add test case for adding extension only to nodes that have meshes.
    gltf->nodes[1].extensions.insert(std::pair<std::string, tinygltf::Value>(std::string("EXT_mesh_gpu_instancing"), instancingExtensionValue));
    gltf->extensionsUsed.emplace_back("EXT_mesh_gpu_instancing");
    gltf->extensionsRequired.emplace_back("EXT_mesh_gpu_instancing");

    // Add RTC center to node.
    gltf->nodes[1].translation[0] += tileCenterCartesian.x;
    gltf->nodes[1].translation[1] += tileCenterCartesian.y;
    gltf->nodes[1].translation[2] += tileCenterCartesian.z;
    
    // Fix image URIs
    for (auto &image: gltf->images) {
        image.uri = "Gltf/" + image.uri;
    }
}

size_t writeToI3DM(std::string GltfURI,
                   const CDBModelsAttributes &modelsAttribs,
                   const std::vector<int> &attribIndices,
                   std::ofstream &fs)
{
    const auto &cdbTile = modelsAttribs.getTile();
    const auto &instancesAttribs = modelsAttribs.getInstancesAttributes();
    const auto &cartographicPositions = modelsAttribs.getCartographicPositions();
    const auto &scales = modelsAttribs.getScales();
    const auto &orientation = modelsAttribs.getOrientations();

    size_t totalInstances = attribIndices.size();
    size_t totalPositionSize = totalInstances * sizeof(glm::vec3);
    size_t totalScaleSize = totalInstances * sizeof(glm::vec3);
    size_t totalNormalUpSize = totalInstances * sizeof(glm::vec3);
    size_t totalNormalRightSize = totalInstances * sizeof(glm::vec3);

    // create feature table json
    const auto &ellipsoid = Core::Ellipsoid::WGS84;
    auto centerCartographic = cdbTile.getBoundRegion().getRectangle().computeCenter();
    auto center = ellipsoid.cartographicToCartesian(centerCartographic);
    size_t positionOffset = 0;
    size_t scaleOffset = totalPositionSize;
    size_t normalUpOffset = scaleOffset + totalScaleSize;
    size_t normalRightOffset = normalUpOffset + totalNormalUpSize;
    nlohmann::json featureTableJson;
    featureTableJson["INSTANCES_LENGTH"] = attribIndices.size();
    featureTableJson["RTC_CENTER"] = {center.x, center.y, center.z};
    featureTableJson["POSITION"] = {{"byteOffset", positionOffset}};
    featureTableJson["SCALE_NON_UNIFORM"] = {{"byteOffset", scaleOffset}};
    featureTableJson["NORMAL_UP"] = {{"byteOffset", normalUpOffset}};
    featureTableJson["NORMAL_RIGHT"] = {{"byteOffset", normalRightOffset}};

    // create feature table binary
    std::vector<unsigned char> featureTableBuffer;
    featureTableBuffer.resize(
        roundUp(totalPositionSize + totalScaleSize + totalNormalUpSize + totalNormalRightSize, 8));
    for (size_t i = 0; i < attribIndices.size(); ++i) {
        auto instanceIdx = attribIndices[i];
        glm::dvec3 worldPosition = ellipsoid.cartographicToCartesian(cartographicPositions[instanceIdx]);
        glm::vec3 positionRTC = worldPosition - center;

        glm::dmat4 rotation = calculateModelOrientation(worldPosition, orientation[instanceIdx]);
        glm::vec3 normalUp = glm::normalize(glm::column(rotation, 1));
        glm::vec3 normalRight = glm::normalize(glm::column(rotation, 0));

        std::memcpy(featureTableBuffer.data() + positionOffset + i * sizeof(glm::vec3),
                    &positionRTC[0],
                    sizeof(glm::vec3));

        std::memcpy(featureTableBuffer.data() + scaleOffset + i * sizeof(glm::vec3),
                    &scales[instanceIdx][0],
                    sizeof(glm::vec3));

        std::memcpy(featureTableBuffer.data() + normalUpOffset + i * sizeof(glm::vec3),
                    &normalUp,
                    sizeof(glm::vec3));

        std::memcpy(featureTableBuffer.data() + normalRightOffset + i * sizeof(glm::vec3),
                    &normalRight,
                    sizeof(glm::vec3));
    }

    // create batch table
    const auto &CNAMs = instancesAttribs.getCNAMs();
    const auto &integerAttribs = instancesAttribs.getIntegerAttribs();
    const auto &doubleAttribs = instancesAttribs.getDoubleAttribs();
    const auto &stringAttribs = instancesAttribs.getStringAttribs();
    size_t totalIntSize = roundUp(totalInstances * integerAttribs.size() * sizeof(int32_t), 8);
    size_t totalDoubleSize = totalInstances * doubleAttribs.size() * sizeof(double);
    std::vector<unsigned char> batchTableBuffer(totalIntSize + totalDoubleSize);

    nlohmann::json batchTableJson;
    batchTableJson["CNAM"] = nlohmann::json::array();
    for (auto idx : attribIndices) {
        batchTableJson["CNAM"].emplace_back(CNAMs[idx]);
    }

    for (auto pair : stringAttribs) {
        if (batchTableJson.find(pair.first) == batchTableJson.end()) {
            batchTableJson[pair.first] = nlohmann::json::array();
        }

        for (auto idx : attribIndices) {
            batchTableJson[pair.first].emplace_back(pair.second[idx]);
        }
    }

    size_t batchTableOffset = 0;
    for (auto pair : integerAttribs) {
        batchTableJson[pair.first]["byteOffset"] = batchTableOffset;
        batchTableJson[pair.first]["type"] = "SCALAR";
        batchTableJson[pair.first]["componentType"] = "INT";
        for (auto idx : attribIndices) {
            int value = pair.second[idx];
            std::memcpy(batchTableBuffer.data() + batchTableOffset, &value, sizeof(int32_t));
            batchTableOffset += sizeof(int32_t);
        }
    }

    batchTableOffset = roundUp(batchTableOffset, 8);
    for (auto pair : doubleAttribs) {
        batchTableJson[pair.first]["byteOffset"] = batchTableOffset;
        batchTableJson[pair.first]["type"] = "SCALAR";
        batchTableJson[pair.first]["componentType"] = "DOUBLE";
        for (auto idx : attribIndices) {
            double value = pair.second[idx];
            std::memcpy(batchTableBuffer.data() + batchTableOffset, &value, sizeof(double));
            batchTableOffset += sizeof(double);
        }
    }

    // create header
    std::string featureTableString = featureTableJson.dump();
    size_t headerToRoundUp = sizeof(I3dmHeader) + featureTableString.size();
    featureTableString += std::string(roundUp(headerToRoundUp, 8) - headerToRoundUp, ' ');

    std::string batchTableString = batchTableJson.dump();
    batchTableString += std::string(roundUp(batchTableString.size(), 8) - batchTableString.size(), ' ');

    GltfURI += std::string(roundUp(GltfURI.size(), 8) - GltfURI.size(), ' ');

    I3dmHeader header;
    header.magic[0] = 'i';
    header.magic[1] = '3';
    header.magic[2] = 'd';
    header.magic[3] = 'm';
    header.version = 1;
    header.byteLength = static_cast<uint32_t>(sizeof(header))
                        + static_cast<uint32_t>(featureTableString.size())
                        + static_cast<uint32_t>(featureTableBuffer.size())
                        + static_cast<uint32_t>(batchTableString.size())
                        + static_cast<uint32_t>(batchTableBuffer.size())
                        + static_cast<uint32_t>(GltfURI.size());
    header.featureTableJsonByteLength = static_cast<uint32_t>(featureTableString.size());
    header.featureTableBinByteLength = static_cast<uint32_t>(featureTableBuffer.size());
    header.batchTableJsonByteLength = static_cast<uint32_t>(batchTableString.size());
    header.batchTableBinByteLength = static_cast<uint32_t>(batchTableBuffer.size());
    header.gltfFormat = 0;

    fs.write(reinterpret_cast<const char *>(&header), sizeof(I3dmHeader));
    fs.write(featureTableString.data(), featureTableString.size());
    fs.write(reinterpret_cast<const char *>(featureTableBuffer.data()), featureTableBuffer.size());

    fs.write(batchTableString.data(), batchTableString.size());
    fs.write(reinterpret_cast<const char *>(batchTableBuffer.data()), batchTableBuffer.size());

    fs.write(GltfURI.data(), GltfURI.size());

    return header.byteLength;
}

void writeToB3DM(tinygltf::Model *gltf, const CDBInstancesAttributes *instancesAttribs, std::ofstream &fs)
{
    // create glb
    std::stringstream ss;
    tinygltf::TinyGLTF gltfIO;
    gltfIO.WriteGltfSceneToStream(gltf, ss, false, true);

    // put glb into buffer
    ss.seekp(0, std::ios::end);
    std::stringstream::pos_type offset = ss.tellp();
    std::vector<uint8_t> glbBuffer(roundUp(offset, 8), 0);
    ss.read(reinterpret_cast<char *>(glbBuffer.data()), glbBuffer.size());

    // create feature table
    size_t numOfBatchID = 0;
    if (instancesAttribs) {
        numOfBatchID = instancesAttribs->getInstancesCount();
    }
    std::string featureTableString = "{\"BATCH_LENGTH\":" + std::to_string(numOfBatchID) + "}";
    size_t headerToRoundUp = sizeof(B3dmHeader) + featureTableString.size();
    featureTableString += std::string(roundUp(headerToRoundUp, 8) - headerToRoundUp, ' ');

    // create batch table
    std::string batchTableHeader;
    std::vector<uint8_t> batchTableBuffer;
    createBatchTable(instancesAttribs, batchTableHeader, batchTableBuffer);

    // create header
    B3dmHeader header;
    header.magic[0] = 'b';
    header.magic[1] = '3';
    header.magic[2] = 'd';
    header.magic[3] = 'm';
    header.version = 1;
    header.byteLength = static_cast<uint32_t>(sizeof(header))
                        + static_cast<uint32_t>(featureTableString.size())
                        + static_cast<uint32_t>(batchTableHeader.size())
                        + static_cast<uint32_t>(batchTableBuffer.size())
                        + static_cast<uint32_t>(glbBuffer.size());
    header.featureTableJsonByteLength = static_cast<uint32_t>(featureTableString.size());
    header.featureTableBinByteLength = 0;
    header.batchTableJsonByteLength = static_cast<uint32_t>(batchTableHeader.size());
    header.batchTableBinByteLength = static_cast<uint32_t>(batchTableBuffer.size());

    fs.write(reinterpret_cast<const char *>(&header), sizeof(B3dmHeader));
    fs.write(featureTableString.data(), featureTableString.size());

    fs.write(batchTableHeader.data(), batchTableHeader.size());
    fs.write(reinterpret_cast<const char *>(batchTableBuffer.data()), batchTableBuffer.size());

    fs.write(reinterpret_cast<const char *>(glbBuffer.data()), glbBuffer.size());
}

void writeToGLTF(tinygltf::Model *gltf, const CDBInstancesAttributes *instancesAttribs, std::ofstream &fs) {

    // Add metadata.
    createFeatureMetadataClasses(gltf, instancesAttribs);

    // Create glTF stringstream
    std::stringstream ss;
    tinygltf::TinyGLTF gltfIO;
    gltfIO.SetStoreOriginalJSONForExtrasAndExtensions(true);
    gltfIO.WriteGltfSceneToStream(gltf, ss, false, true);

    // Create glTF unint8_t buffer
    ss.seekp(0, std::ios::end);
    std::stringstream::pos_type offset = ss.tellp();
    std::vector<uint8_t> glbBuffer(roundUp(offset, 8), 0);
    ss.read(reinterpret_cast<char *>(glbBuffer.data()), glbBuffer.size());

    // Write glTF buffer to file.6
    fs.write(reinterpret_cast<const char *>(glbBuffer.data()), glbBuffer.size());
}

void writeToCMPT(uint32_t numOfTiles,
                 std::ofstream &fs,
                 std::function<uint32_t(std::ofstream &, size_t tileIdx)> writeToTileFormat)
{
    CmptHeader header;
    header.magic[0] = 'c';
    header.magic[1] = 'm';
    header.magic[2] = 'p';
    header.magic[3] = 't';
    header.version = 1;
    header.titleLength = numOfTiles;
    header.byteLength = sizeof(header);

    fs.write(reinterpret_cast<char *>(&header), sizeof(header));
    for (size_t i = 0; i < numOfTiles; ++i) {
        header.byteLength += writeToTileFormat(fs, i);
    }

    fs.seekp(0, std::ios::beg);
    fs.write(reinterpret_cast<char *>(&header), sizeof(header));
}

void createBatchTable(const CDBInstancesAttributes *instancesAttribs,
                      std::string &batchTableJsonStr,
                      std::vector<uint8_t> &batchTableBuffer)
{
    if (instancesAttribs) {
        nlohmann::json batchTableJson;
        size_t instancesCount = instancesAttribs->getInstancesCount();
        const auto &CNAMs = instancesAttribs->getCNAMs();
        const auto &integerAttribs = instancesAttribs->getIntegerAttribs();
        const auto &doubleAttribs = instancesAttribs->getDoubleAttribs();
        const auto &stringAttribs = instancesAttribs->getStringAttribs();
        size_t totalIntegerSize = roundUp(integerAttribs.size() * sizeof(int32_t) * instancesCount, 8);
        size_t totalDoubleSize = doubleAttribs.size() * sizeof(double) * instancesCount;

        batchTableBuffer.resize(totalIntegerSize + totalDoubleSize);

        // Special keys of CDB attributes that map to class attribute
        batchTableJson["CNAM"] = CNAMs;

        // Per instance attributes
        for (const auto &keyValue : stringAttribs) {
            batchTableJson[keyValue.first] = keyValue.second;
        }

        size_t batchTableOffset = 0;
        size_t batchTableSize = 0;
        for (const auto &keyValue : integerAttribs) {
            batchTableSize = keyValue.second.size() * sizeof(int32_t);
            std::memcpy(batchTableBuffer.data() + batchTableOffset, keyValue.second.data(), batchTableSize);
            batchTableJson[keyValue.first]["byteOffset"] = batchTableOffset;
            batchTableJson[keyValue.first]["type"] = "SCALAR";
            batchTableJson[keyValue.first]["componentType"] = "INT";

            batchTableOffset += batchTableSize;
        }

        batchTableOffset = roundUp(batchTableOffset, 8);
        for (const auto &keyValue : doubleAttribs) {
            batchTableSize = keyValue.second.size() * sizeof(double);
            std::memcpy(batchTableBuffer.data() + batchTableOffset, keyValue.second.data(), batchTableSize);
            batchTableJson[keyValue.first]["byteOffset"] = batchTableOffset;
            batchTableJson[keyValue.first]["type"] = "SCALAR";
            batchTableJson[keyValue.first]["componentType"] = "DOUBLE";

            batchTableOffset += batchTableSize;
        }

        batchTableJsonStr = batchTableJson.dump();
        batchTableJsonStr += std::string(roundUp(batchTableJsonStr.size(), 8) - batchTableJsonStr.size(), ' ');
    }
}

void createFeatureMetadataClasses(
    tinygltf::Model *gltf,
    const CDBInstancesAttributes *instancesAttribs
    )
{
    if (instancesAttribs) {
        CDBAttributes attributes;

        // Add properties to CDB metadata class
        nlohmann::json metadataExtension;

        // Add data to buffer
        auto &metadataBufferData = gltf->buffers[0].data;

        for (size_t i = 0; i < gltf->meshes.size(); i++) {
            // Replace _BATCH_ID attribute with _FEATURE_ID_0
            int batchIdAccessorIndex = gltf->meshes[i].primitives[0].attributes["_BATCHID"];
            gltf->meshes[i].primitives[0].attributes.extract("_BATCHID");
            gltf->meshes[i].primitives[0].attributes.insert(std::pair<std::string, int>({std::string("_FEATURE_ID_0"), gltf->accessors[batchIdAccessorIndex].bufferView}));

            size_t instanceCount = instancesAttribs->getInstancesCount();
            const auto &integerAttributes = instancesAttribs->getIntegerAttribs();
            const auto &doubleAttributes = instancesAttribs->getDoubleAttribs();
            //const auto &stringAttributes = instancesAttribs->getStringAttribs();
            
            for (const auto &property : integerAttributes) {
                // Get size of metadata in bytes.
                size_t propertyBufferLength = sizeof(int32_t) * instanceCount;
                // Resize metadata buffer.
                size_t originalBufferLength = metadataBufferData.size();
                metadataBufferData.resize(metadataBufferData.size() + propertyBufferLength);
                // Copy metadata into buffer.
                std::memcpy(metadataBufferData.data() + originalBufferLength, property.second.data(), propertyBufferLength);
                // Add buffer view for property.
                tinygltf::BufferView bufferView;
                // Set the buffer to buffer.size() because buffer is added to glTF after all metadata bufferViews are added.
                bufferView.buffer = 0;
                bufferView.byteOffset = originalBufferLength;
                bufferView.byteLength = propertyBufferLength;
                gltf->bufferViews.emplace_back(bufferView);

                // Add property to class
                metadataExtension["schema"]["classes"][CDB_CLASS_NAME]["properties"][property.first]["name"] = attributes.names[property.first];
                metadataExtension["schema"]["classes"][CDB_CLASS_NAME]["properties"][property.first]["description"] = attributes.descriptions[property.first];
                metadataExtension["schema"]["classes"][CDB_CLASS_NAME]["properties"][property.first]["type"] = "INT32";

                // Add propety to feature table
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["class"] = CDB_CLASS_NAME;
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["count"] = instanceCount;
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["properties"][property.first]["bufferView"] = static_cast<int>(gltf->bufferViews.size() - 1);
            }
            
            for (const auto &property : doubleAttributes) {
                // Get size of metadata in bytes.
                size_t propertyBufferLength = sizeof(double_t) * instanceCount;
                // Resize metadata buffer.
                size_t originalBufferLength = metadataBufferData.size();
                // Check for padding for FLOAT64 type.
                originalBufferLength += originalBufferLength % 8 == 0 ? 0 : 4;
                metadataBufferData.resize(metadataBufferData.size() + propertyBufferLength);
                // Copy metadata into buffer.
                std::memcpy(metadataBufferData.data() + originalBufferLength, property.second.data(), propertyBufferLength);
                // Add buffer view for property
                tinygltf::BufferView bufferView;
                // Set the buffer to buffer.size() because buffer is added to glTF after all metadata bufferViews are added.
                bufferView.buffer = 0;
                bufferView.byteOffset = originalBufferLength;
                bufferView.byteLength = propertyBufferLength;
                gltf->bufferViews.emplace_back(bufferView);

                // Add property to class
                metadataExtension["schema"]["classes"][CDB_CLASS_NAME]["properties"][property.first]["name"] = attributes.names[property.first];
                metadataExtension["schema"]["classes"][CDB_CLASS_NAME]["properties"][property.first]["description"] = attributes.descriptions[property.first];
                metadataExtension["schema"]["classes"][CDB_CLASS_NAME]["properties"][property.first]["type"] = "FLOAT64";

                // Add propety to feature table
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["class"] = CDB_CLASS_NAME;
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["count"] = instanceCount;
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["properties"][property.first]["bufferView"] = static_cast<int>(gltf->bufferViews.size() - 1);

            }
            /*
            for (const auto &property : stringAttributes) {
                // Create string offsets buffer.
                std::vector<uint8_t> offsets;
                size_t offsetsBufferLength = sizeof(size_t) * instanceCount;
                size_t propertyBufferLength = 0;
                for (const auto &string : property.second) {
                    offsets.emplace_back(string.length());
                    propertyBufferLength += string.length();
                }

                offsetsBufferData.resize(offsetsBufferLength);
                std::memcpy(offsetsBufferData.data(), offsets.data(), offsetsBufferLength);
                gltf->buffers.emplace_back(offsetsBuffer);

                // Create string offsets bufferView.
                tinygltf::BufferView offsetsBufferView;
                offsetsBufferView.buffer = static_cast<int>(gltf->buffers.size() - 1);
                offsetsBufferView.byteOffset = 0;
                offsetsBufferView.byteLength = offsetsBufferLength;
                gltf->bufferViews.emplace_back(offsetsBufferView);

                // Create strings buffer.
                tinygltf::Buffer buffer;
                auto &metadataBufferData = buffer.data;
                metadataBufferData.resize(propertyBufferLength);
                std::memcpy(metadataBufferData.data(), property.second.data(), propertyBufferLength);
                gltf->buffers.emplace_back(buffer);

                // Add buffer view for property
                tinygltf::BufferView bufferView;
                bufferView.buffer = static_cast<int>(gltf->buffers.size() - 1);
                bufferView.byteOffset = 0;
                bufferView.byteLength = propertyBufferLength;
                gltf->bufferViews.emplace_back(bufferView);

                // Add property to class
                metadataExtension["schema"]["classes"][CDB_CLASS_NAME]["properties"][property.first]["name"] = property.first;
                metadataExtension["schema"]["classes"][CDB_CLASS_NAME]["properties"][property.first]["type"] = "STRING";

                // Add propety to feature table
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["class"] = CDB_CLASS_NAME;
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["elementCount"] = instanceCount;
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["properties"][property.first]["bufferView"] = static_cast<int>(gltf->bufferViews.size() - 1);
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["properties"][property.first]["offsetType"] = "UINT8";
                metadataExtension["featureTables"][CDB_FEATURE_TABLE_NAME]["properties"][property.first]["stringOffsetBufferView"] = static_cast<int>(gltf->bufferViews.size() - 2);
            }
            */
            // Add feature ID attributes to mesh.primitive
            nlohmann::json primitiveExtension;
            primitiveExtension["featureIdAttributes"] =
            { 
                {
                    { "featureTable",  CDB_FEATURE_TABLE_NAME },
                    { "featureIds", { 
                        { "attribute", "_FEATURE_ID_0" } 
                    }}
                }
                
            };

            tinygltf::Value primitiveExtensionValue;
            CDBTo3DTiles::ParseJsonAsValue(&primitiveExtensionValue, primitiveExtension);
            gltf->meshes[i].primitives[0].extensions.insert(std::pair<std::string, tinygltf::Value>(std::string("EXT_feature_metadata"), primitiveExtensionValue));
        }

        tinygltf::Value metadataExtensionValue;
        CDBTo3DTiles::ParseJsonAsValue(&metadataExtensionValue, metadataExtension);
        gltf->extensions.insert(std::pair<std::string, tinygltf::Value>(std::string("EXT_feature_metadata"), metadataExtensionValue));
        gltf->extensionsUsed.emplace_back("EXT_feature_metadata");
    }
}

void convertTilesetToJson(const CDBTile &tile, float geometricError, nlohmann::json &json)
{
    const auto &boundRegion = tile.getBoundRegion();
    const auto &rectangle = boundRegion.getRectangle();
    json["boundingVolume"] = {{"region",
                               {
                                   rectangle.getWest(),
                                   rectangle.getSouth(),
                                   rectangle.getEast(),
                                   rectangle.getNorth(),
                                   boundRegion.getMinimumHeight(),
                                   boundRegion.getMaximumHeight(),
                               }}};

    auto contentURI = tile.getCustomContentURI();
    if (contentURI) {
        json["content"] = nlohmann::json::object();
        json["content"]["uri"] = *contentURI;
    }

   const std::vector<CDBTile *> &children = tile.getChildren();

    if (children.empty()) {
        json["geometricError"] = 0.0f;
    } else {
        json["geometricError"] = geometricError;

        for (auto child : children) {
            if (child == nullptr) {
                continue;
            }

            nlohmann::json childJson = nlohmann::json::object();
            convertTilesetToJson(*child, geometricError / 2.0f, childJson);
            json["children"].emplace_back(childJson);
        }
    }
}

static bool ParseJsonAsValue(tinygltf::Value *ret, const nlohmann::json &o) {
  tinygltf::Value val{};
  switch (o.type()) {
    case nlohmann::json::value_t::object: {
      tinygltf::Value::Object value_object;
      for (auto it = o.begin(); it != o.end(); it++) {
        tinygltf::Value entry;
        CDBTo3DTiles::ParseJsonAsValue(&entry, it.value());
        if (entry.Type() != tinygltf::NULL_TYPE)
          value_object.emplace(it.key(), std::move(entry));
      }
      if (value_object.size() > 0) val = tinygltf::Value(std::move(value_object));
    } break;
    case nlohmann::json::value_t::array: {
      tinygltf::Value::Array value_array;
      value_array.reserve(o.size());
      for (auto it = o.begin(); it != o.end(); it++) {
        tinygltf::Value entry;
        CDBTo3DTiles::ParseJsonAsValue(&entry, it.value());
        if (entry.Type() != tinygltf::NULL_TYPE)
          value_array.emplace_back(std::move(entry));
      }
      if (value_array.size() > 0) val = tinygltf::Value(std::move(value_array));
    } break;
    case nlohmann::json::value_t::string:
      val = tinygltf::Value(o.get<std::string>());
      break;
    case nlohmann::json::value_t::boolean:
      val = tinygltf::Value(o.get<bool>());
      break;
    case nlohmann::json::value_t::number_integer:
    case nlohmann::json::value_t::number_unsigned:
      val = tinygltf::Value(static_cast<int>(o.get<int64_t>()));
      break;
    case nlohmann::json::value_t::number_float:
      val = tinygltf::Value(o.get<double>());
      break;
    case nlohmann::json::value_t::null:
    case nlohmann::json::value_t::discarded:
    default:
      // default:
      break;
  }
  if (ret) *ret = std::move(val);

  return val.Type() != tinygltf::NULL_TYPE;
}

} // namespace CDBTo3DTiles
