#include "CDBRMDescriptor.h"
#include "rapidxml_utils.hpp"
#include "json.hpp"
#include "tiny_gltf.h"
#include <vector>
#include <iostream>

namespace CDBTo3DTiles {

static std::string CDB_MATERIAL_CLASS_NAME = "CDBMaterialClass";
static std::string CDB_MATERIAL_PROPERTY_NAME = "compositeMaterialName";
static std::string CDB_MATERIAL_FEATURE_TABLE_NAME = "CDBMaterialFeatureTable";

static bool ParseJsonAsValue(tinygltf::Value *ret, const nlohmann::json &o);

CDBRMDescriptor::CDBRMDescriptor(std::filesystem::path xmlPath, const CDBTile &tile)
    : _xmlPath{xmlPath}, _tile{tile}
{

  std::cout << _xmlPath.c_str() << std::endl;
}

void CDBRMDescriptor::addFeatureTable(tinygltf::Model *gltf)
{
    rapidxml::file<> xmlFile(_xmlPath.c_str());
    rapidxml::xml_document<> xml;
    xml.parse<0>(xmlFile.data());

    // Build vector of Composite Material names.
    int currentId = 0;
    std::vector<uint8_t> debugIds;
    std::vector<std::string> compositeMaterialNames;
    std::vector<uint8_t> compositeMaterialNameOffsets = {0};
    size_t currentOffset = 0;
    rapidxml::xml_node<> *tableNode = xml.first_node("Composite_Material_Table");
    for (rapidxml::xml_node<> *materialNode = tableNode->first_node("Composite_Material"); materialNode;
        materialNode = materialNode->next_sibling()) {

        debugIds.emplace_back(currentId++);
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

    // Adds debug ID to buffer.
    tinygltf::BufferView debugBufferView;
    debugBufferView.buffer = 0;
    debugBufferView.byteOffset = bufferSize;
    debugBufferView.byteLength = sizeof(uint8_t) * debugIds.size();; // The current offset will point to the end of the string buffer.
    gltf->bufferViews.emplace_back(debugBufferView);
    // Add debug ID to buffer.
    bufferData->resize(bufferSize + debugBufferView.byteLength);
    std::memcpy(bufferData->data() + bufferSize, debugIds.data(), debugBufferView.byteLength);
    bufferSize += debugBufferView.byteLength;

    // Setup feature metadata class.
    nlohmann::json featureClass = nlohmann::json::object();
    featureClass["properties"][CDB_MATERIAL_PROPERTY_NAME]["type"] = "STRING";
    featureClass["properties"][CDB_MATERIAL_PROPERTY_NAME]["description"] = "The composite material name.";
    featureClass["properties"][CDB_MATERIAL_PROPERTY_NAME]["name"] = "Composite Material";

    featureClass["properties"]["debugId"]["type"] = "UINT8";
    featureClass["properties"]["debugId"]["description"] = "DEBUG";
    featureClass["properties"]["debugId"]["name"] = "DEBUG";

    // Setup feature table.
    nlohmann::json featureTable = nlohmann::json::object();
    featureTable["class"] = CDB_MATERIAL_CLASS_NAME;
    featureTable["count"] = compositeMaterialNames.size();
    featureTable["properties"][CDB_MATERIAL_PROPERTY_NAME]["bufferView"] = gltf->bufferViews.size() - 2;
    featureTable["properties"][CDB_MATERIAL_PROPERTY_NAME]["stringOffsetBufferView"] = gltf->bufferViews.size() - 3;
    featureTable["properties"][CDB_MATERIAL_PROPERTY_NAME]["offsetType"] = "UINT8";

    featureTable["properties"]["debugId"]["bufferView"] = gltf->bufferViews.size() - 1;

    // Create EXT_feature_metadata extension and add it to glTF.
    nlohmann::json extension = nlohmann::json::object();
    extension["schema"]["classes"][CDB_MATERIAL_CLASS_NAME] = featureClass;
    extension["featureTables"][CDB_MATERIAL_FEATURE_TABLE_NAME] = featureTable;

    tinygltf::Value extensionValue;
    ParseJsonAsValue(&extensionValue, extension);
    gltf->extensions.insert(std::pair<std::string, tinygltf::Value>(std::string("EXT_feature_metadata"), extensionValue));
    gltf->extensionsUsed.emplace_back("EXT_feature_metadata");
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
