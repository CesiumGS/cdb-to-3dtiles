#include "CDBRMDescriptor.h"
#include "rapidxml_utils.hpp"
#include "tiny_gltf.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#include "Gltf.h"
#include <tinyutf8/tinyutf8.h>

namespace CDBTo3DTiles {

static const std::string CDB_MATERIAL_CLASS_NAME = "CDBMaterialsClass";
static const std::string CDB_MATERIAL_FEATURE_TABLE_NAME = "CDBMaterialFeatureTable";
static const std::string SCHEMA_PATH = "../../../../../materials.json";

static bool ParseJsonAsValue(tinygltf::Value *ret, const nlohmann::json &o);

CDBRMDescriptor::CDBRMDescriptor(std::filesystem::path xmlPath, const CDBTile &tile)
    : _xmlPath{xmlPath}
    , _tile{tile}
{}

void CDBRMDescriptor::addFeatureTableToGltf(CDBMaterials *materials, tinygltf::Model *gltf, bool externalSchema)
{
    // Parse RMDescriptor XML file.
    rapidxml::file<> xmlFile(_xmlPath.c_str());
    rapidxml::xml_document<> xml;
    xml.parse<0>(xmlFile.data());

    // Build vector of Composite Material names.
    int currentId = 1;
    std::vector<uint8_t> debugIds = {0};
    std::vector<std::vector<uint8_t>> compositeMaterialNames = {{0}};
    std::vector<uint8_t> substrates = {0};
    std::vector<uint8_t> weights = {0};
    std::vector<uint8_t> arrayOffsets = {0, 1}; // Substrates and weights can share an arrayOffsetBuffer
    size_t substrateOffset = 1;
    std::vector<uint8_t> compositeMaterialNameOffsets = {0,1};
    size_t currentOffset = 1;

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
        std::string name = materialNameNode->value();
        compositeMaterialNames.emplace_back(std::vector<uint8_t> (name.begin(), name.end()));

        // Update start offset of next name.
        currentOffset += materialNameNode->value_size();
        // Insert offset.
        compositeMaterialNameOffsets.emplace_back(currentOffset);
    }

    int stringOffsetBufferViewIndex = createMetadataBufferView(gltf, compositeMaterialNameOffsets);
    int stringsBufferViewIndex = createMetadataBufferView(gltf, compositeMaterialNames, currentOffset);
    int substratesBufferViewIndex = createMetadataBufferView(gltf, substrates);
    int weightsBufferViewIndex = createMetadataBufferView(gltf, weights);
    int arrayOffsetBufferViewIndex = createMetadataBufferView(gltf, arrayOffsets);
    int debugIdBufferViewIndex = createMetadataBufferView(gltf, debugIds);

    // Setup feature table.
    nlohmann::json featureTable = nlohmann::json::object();
    featureTable["class"] = CDB_MATERIAL_CLASS_NAME;
    featureTable["count"] = compositeMaterialNames.size();

    featureTable["properties"]["name"]["bufferView"] = stringsBufferViewIndex;
    featureTable["properties"]["name"]["offsetType"] = "UINT8";
    featureTable["properties"]["name"]["stringOffsetBufferView"] = stringOffsetBufferViewIndex;
                                                                                    
    
    featureTable["properties"]["substrates"]["bufferView"] = substratesBufferViewIndex;
    featureTable["properties"]["substrates"]["offsetType"] = "UINT8";
    featureTable["properties"]["substrates"]["arrayOffsetBufferView"] = arrayOffsetBufferViewIndex;

    featureTable["properties"]["weights"]["bufferView"] = weightsBufferViewIndex;
    featureTable["properties"]["weights"]["offsetType"] = "UINT8";
    featureTable["properties"]["weights"]["arrayOffsetBufferView"] = arrayOffsetBufferViewIndex;

    featureTable["properties"]["debugId"]["bufferView"] = debugIdBufferViewIndex;

    // Create EXT_feature_metadata extension and add it to glTF.
    nlohmann::json extension = nlohmann::json::object();
    if (externalSchema)
        extension["schemaUri"] = SCHEMA_PATH;
    else
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
