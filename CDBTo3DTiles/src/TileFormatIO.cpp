#include "TileFormatIO.h"
#include "Ellipsoid.h"
#include "glm/gtc/matrix_access.hpp"
#include "nlohmann/json.hpp"

namespace CDBTo3DTiles {
static void createBatchTable(const CDBInstancesAttributes *instancesAttribs,
                             std::string &batchTableJson,
                             std::vector<uint8_t> &batchTableBuffer);

static void convertTilesetToJson(const CDBTile &tile, float geometricError, nlohmann::json &json);

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
        convertTilesetToJson(*root, 300000.0f, tilesetJson["root"]);
        tilesetJson["geometricError"] = tilesetJson["root"]["geometricError"];
        fs << tilesetJson << std::endl;
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

void convertTilesetToJson(const CDBTile &tile, float geometricError, nlohmann::json &json)
{
    const auto &boundRegion = tile.getBoundRegion();
    const auto &rectangle = boundRegion.getRectangle();
    json["geometricError"] = geometricError;
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
        json["content"]["uri"] = contentURI->string();
    }

    for (auto child : tile.getChildren()) {
        if (child == nullptr) {
            continue;
        }

        nlohmann::json childJson = nlohmann::json::object();
        convertTilesetToJson(*child, geometricError / 2.0f, childJson);
        json["children"].emplace_back(childJson);
    }
}

} // namespace CDBTo3DTiles
