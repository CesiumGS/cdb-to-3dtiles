#include "CDBRMDescriptor.h"
#include "rapidxml_utils.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <vector>

namespace CDBTo3DTiles {

static std::string CDB_MATERIAL_CLASS_NAME = "CDBMaterialClass";
static std::string CDB_MATERIAL_PROPERTY_NAME = "compositeMaterialName";
static std::string CDB_MATERIAL_FEATURE_TABLE_NAME = "CDBMaterialFeatureTable";

CDBRMDescriptor::CDBRMDescriptor(std::filesystem::path xmlPath, const CDBTile &tile)
    : _tile{tile}
{
    rapidxml::file<> xmlFile(xmlPath.c_str());
    _xml.parse<0>(xmlFile.data());
}

void CDBRMDescriptor::addFeatureTable(tinygltf::Model *gltf)
{
    // Build vector of Composite Material names.
    std::vector<std::string> compositeMaterialNames;
    std::vector<uint8_t> compositeMaterialNameOffsets = {0};
    size_t currentOffset = 0;
    rapidxml::xml_node<> *tableNode = _xml.first_node("Composite_Material_Table");
    for (rapidxml::xml_node<> *materialNode = tableNode->first_node("Composite_Material"); materialNode;
        materialNode = materialNode->next_sibling()) {
        rapidxml::xml_node<> *materialNameNode = materialNode->first_node("Name");

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
    stringBufferView.byteLength = currentOffset; // The current offset will point to the end of the string buffer.
    gltf->bufferViews.emplace_back(stringBufferView);
    // Add strings to buffer.
    bufferData->resize(bufferSize + currentOffset);
    std::memcpy(bufferData->data() + bufferSize, compositeMaterialNames.data(), currentOffset);
    bufferSize += currentOffset;

    // Setup feature metadata class.
    nlohmann::json featureClass = nlohmann::json::object();
    featureClass["properties"][CDB_MATERIAL_PROPERTY_NAME]["type"] = "STRING";
    featureClass["properties"][CDB_MATERIAL_PROPERTY_NAME]["description"] = "The composite material name.";
    featureClass["properties"][CDB_MATERIAL_PROPERTY_NAME]["name"] = "Composite Material";

    // Setup feature table.
    nlohmann::json featureTable = nlohmann::json::object();
    featureTable["class"] = CDB_MATERIAL_CLASS_NAME;
    featureTable["count"] = compositeMaterialNames.size();
    featureTable["properties"][CDB_MATERIAL_PROPERTY_NAME]["bufferView"] = gltf->bufferViews.size() - 1;
    featureTable["properties"][CDB_MATERIAL_PROPERTY_NAME]["stringOffsetBufferView"] = gltf->bufferViews.size() - 2;

    // Create EXT_feature_metadata extension and add it to glTF.
    nlohmann::json extension = nlohmann::json::object();
    extension["schema"]["classes"][CDB_MATERIAL_CLASS_NAME] = featureClass;
    extension["featureTables"][CDB_MATERIAL_FEATURE_TABLE_NAME] = featureTable;

    tinygltf::Value extensionValue;
    tinygltf::ParseJsonAsValue(&extensionValue, extension);
    gltf->extensions.insert(std::pair<std::string, tinygltf::Value>(std::string("EXT_feature_metadata"), extensionValue));
    gltf->extensionsUsed.emplace_back("EXT_feature_metadata");
}

} // namespace CDBTo3DTiles
