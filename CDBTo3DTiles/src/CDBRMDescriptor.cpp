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

    unsigned int stringOffsetBufferViewIndex = createMetadataBufferView(gltf, compositeMaterialNameOffsets);
    unsigned int stringsBufferViewIndex = createMetadataBufferView(gltf, compositeMaterialNames, currentOffset);
    unsigned int substratesBufferViewIndex = createMetadataBufferView(gltf, substrates);
    unsigned int weightsBufferViewIndex = createMetadataBufferView(gltf, weights);
    unsigned int arrayOffsetBufferViewIndex = createMetadataBufferView(gltf, arrayOffsets);

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

    // Create EXT_feature_metadata extension and add it to glTF.
    nlohmann::json extension = nlohmann::json::object();
    if (externalSchema)
        extension["schemaUri"] = SCHEMA_PATH;
    else
        extension["schema"] = materials->generateSchema();
    extension["featureTables"][CDB_MATERIAL_FEATURE_TABLE_NAME] = featureTable;

    tinygltf::Value extensionValue;
    CDBTo3DTiles::ParseJsonAsValue(&extensionValue, extension);
    gltf->extensions.insert(
        std::pair<std::string, tinygltf::Value>(std::string("EXT_feature_metadata"), extensionValue));
    gltf->extensionsUsed.emplace_back("EXT_feature_metadata");
}

} // namespace CDBTo3DTiles
