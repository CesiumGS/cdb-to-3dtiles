#include "CDBRMDescriptor.h"
#include "rapidxml_utils.hpp"
#include "tiny_gltf.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

namespace CDBTo3DTiles {

static std::string CDB_MATERIAL_CLASS_NAME = "CDBMaterialsClass";
static std::string CDB_MATERIAL_FEATURE_TABLE_NAME = "CDBMaterialFeatureTable";

static bool ParseJsonAsValue(tinygltf::Value *ret, const nlohmann::json &o);

CDBRMDescriptor::CDBRMDescriptor(std::filesystem::path xmlPath, const CDBTile &tile)
    : _xmlPath{xmlPath}
    , _tile{tile}
{}

void CDBRMDescriptor::addFeatureTable(CDBMaterials *materials, tinygltf::Model *gltf)
{
    // Parse RMDescriptor XML file.
    rapidxml::file<> xmlFile(_xmlPath.c_str());
    rapidxml::xml_document<> xml;
    xml.parse<0>(xmlFile.data());

    // Build vector of Composite Material names.
    int currentId = 1;
    std::vector<uint8_t> debugIds = {0};
    std::vector<std::string> compositeMaterialNames = {"X"};
    std::vector<uint8_t> substrates = {0};
    std::vector<uint8_t> weights = {0};
    std::vector<uint8_t> arrayOffsets = {0, 1}; // Substrates and weights can share an arrayOffsetBuffer
    size_t substrateOffset = 1;
    std::vector<uint8_t> compositeMaterialNameOffsets = {0,1};
    size_t currentOffset = 2;

    // Iterate through Composite_Material(s).
    rapidxml::xml_node<> *tableNode = xml.first_node("Composite_Material_Table");
    for (rapidxml::xml_node<> *materialNode = tableNode->first_node("Composite_Material"); materialNode;
        materialNode = materialNode->next_sibling()) {
        
        debugIds.emplace_back(currentId++);
        rapidxml::xml_node<> *materialNameNode = materialNode->first_node("Name");

        // Iterate through Material(s).
        rapidxml::xml_node<> *substrateNode = materialNode->first_node("Primary_Substrate");
        for (rapidxml::xml_node<> *baseMaterialNode = substrateNode->first_node("Material"); baseMaterialNode;
            baseMaterialNode = baseMaterialNode->next_sibling()) {
            // Increase substrate offset.
            substrateOffset++;
            // Get base material index from name.
            auto baseMaterial = &materials->baseMaterials.find(baseMaterialNode->first_node("Name")->value())->second;
            int enumIndex = baseMaterial->value;
            // Get weight.
            int weight = std::stoi(baseMaterialNode->first_node("Weight")->value());

            substrates.emplace_back(enumIndex);
            weights.emplace_back(weight);
        }
        // Insert substrate offset.
        arrayOffsets.emplace_back(substrateOffset);

        // Insert name.
        compositeMaterialNames.emplace_back(materialNameNode->value());
        // Update start offset of next name.
        currentOffset += materialNameNode->value_size();
        // Insert offset.
        compositeMaterialNameOffsets.emplace_back(currentOffset);
    }

    // Get glTF buffer.
    auto bufferData = &gltf->buffers[0].data;
    size_t bufferSize = bufferData->size();

    // Setup bufferView for string offsets.
    size_t offsetsBufferSize = sizeof(uint8_t) * compositeMaterialNameOffsets.size();
    tinygltf::BufferView offsetsBufferView;
    offsetsBufferView.buffer = 0;
    offsetsBufferView.byteOffset = bufferSize;
    offsetsBufferView.byteLength = offsetsBufferSize;
    gltf->bufferViews.emplace_back(offsetsBufferView);
    // Add offsets to buffer.
    bufferData->resize(bufferSize + offsetsBufferSize);
    std::memcpy(bufferData->data() + bufferSize, compositeMaterialNameOffsets.data(), offsetsBufferSize);
    bufferSize += offsetsBufferSize;

    // Add string to buffer.
    tinygltf::BufferView stringBufferView;
    stringBufferView.buffer = 0;
    stringBufferView.byteOffset = bufferSize;
    stringBufferView.byteLength
        = currentOffset; // The current offset will point to the end of the string buffer.
    gltf->bufferViews.emplace_back(stringBufferView);
    // Add strings to buffer.
    bufferData->resize(bufferSize + currentOffset);
    std::memcpy(bufferData->data() + bufferSize, compositeMaterialNames.data(), currentOffset);
    bufferSize += currentOffset;

    // Setup bufferView for substrates.
    size_t substratesBufferSize = sizeof(uint8_t) * substrates.size();
    tinygltf::BufferView substratesBufferView;
    substratesBufferView.buffer = 0;
    substratesBufferView.byteOffset = bufferSize;
    substratesBufferView.byteLength = substratesBufferSize;
    gltf->bufferViews.emplace_back(substratesBufferView);
    // Add substrates to buffer.
    bufferData->resize(bufferSize + substratesBufferSize);
    std::memcpy(bufferData->data() + bufferSize, substrates.data(), substratesBufferSize);
    bufferSize += substratesBufferSize;

    // Setup bufferView for weight.
    tinygltf::BufferView weightsBufferView;
    weightsBufferView.buffer = 0;
    weightsBufferView.byteOffset = bufferSize;
    weightsBufferView.byteLength = substratesBufferSize;
    gltf->bufferViews.emplace_back(weightsBufferView);
    // Add weights to buffer.
    bufferData->resize(bufferSize + substratesBufferSize);
    std::memcpy(bufferData->data() + bufferSize, weights.data(), substratesBufferSize);
    bufferSize += substratesBufferSize;

    // Setup bufferView for offsets for weights and substrates.
    size_t arrayOffsetBufferSize = sizeof(uint8_t) * arrayOffsets.size();
    tinygltf::BufferView arrayOffsetBufferView;
    arrayOffsetBufferView.buffer = 0;
    arrayOffsetBufferView.byteOffset = bufferSize;
    arrayOffsetBufferView.byteLength = arrayOffsetBufferSize;
    gltf->bufferViews.emplace_back(arrayOffsetBufferView);
    // Add weights to buffer.
    bufferData->resize(bufferSize + arrayOffsetBufferSize);
    std::memcpy(bufferData->data() + bufferSize, arrayOffsets.data(), arrayOffsetBufferSize);
    bufferSize += arrayOffsetBufferSize;

    // Adds debug ID to buffer.
    tinygltf::BufferView debugBufferView;
    debugBufferView.buffer = 0;
    debugBufferView.byteOffset = bufferSize;
    debugBufferView.byteLength = sizeof(uint8_t) * debugIds.size();
    // The current offset will point to the end of the string buffer.
    gltf->bufferViews.emplace_back(debugBufferView);
    // Add debug ID to buffer.
    bufferData->resize(bufferSize + debugBufferView.byteLength);
    std::memcpy(bufferData->data() + bufferSize, debugIds.data(), debugBufferView.byteLength);
    bufferSize += debugBufferView.byteLength;

    // Setup feature metadata class.

    // Setup feature table.
    nlohmann::json featureTable = nlohmann::json::object();
    featureTable["class"] = CDB_MATERIAL_CLASS_NAME;
    featureTable["count"] = compositeMaterialNames.size();

    featureTable["properties"]["name"]["bufferView"] = gltf->bufferViews.size() - 5;
    featureTable["properties"]["name"]["offsetType"] = "UINT8";
    featureTable["properties"]["name"]["stringOffsetBufferView"] = gltf->bufferViews.size()
                                                                                       - 6;
                                                                                    
    
    featureTable["properties"]["substrates"]["bufferView"] = gltf->bufferViews.size() - 4;
    featureTable["properties"]["substrates"]["offsetType"] = "UINT8";
    featureTable["properties"]["substrates"]["arrayOffsetBufferView"] = gltf->bufferViews.size()
                                                                                       - 2;

    featureTable["properties"]["weights"]["bufferView"] = gltf->bufferViews.size() - 3;
    featureTable["properties"]["weights"]["offsetType"] = "UINT8";
    featureTable["properties"]["weights"]["arrayOffsetBufferView"] = gltf->bufferViews.size()
                                                                                       - 2;

    featureTable["properties"]["debugId"]["bufferView"] = gltf->bufferViews.size() - 1;

    // Create EXT_feature_metadata extension and add it to glTF.
    nlohmann::json extension = nlohmann::json::object();
    extension["schema"] = materials->generateSchema();
    extension["featureTables"][CDB_MATERIAL_FEATURE_TABLE_NAME] = featureTable;

    tinygltf::Value extensionValue;
    ParseJsonAsValue(&extensionValue, extension);
    gltf->extensions.insert(
        std::pair<std::string, tinygltf::Value>(std::string("EXT_feature_metadata"), extensionValue));
    gltf->extensionsUsed.emplace_back("EXT_feature_metadata");
}

static bool ParseJsonAsValue(tinygltf::Value *ret, const nlohmann::json &o)
{
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
        if (value_object.size() > 0)
            val = tinygltf::Value(std::move(value_object));
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
        if (value_array.size() > 0)
            val = tinygltf::Value(std::move(value_array));
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
    if (ret)
        *ret = std::move(val);

    return val.Type() != tinygltf::NULL_TYPE;
}

} // namespace CDBTo3DTiles
