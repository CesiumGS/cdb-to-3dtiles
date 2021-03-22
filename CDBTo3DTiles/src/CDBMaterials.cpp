#include "CDBMaterials.h"
#include "rapidxml_utils.hpp"

#include <iostream>

namespace CDBTo3DTiles {

static std::string MATERIAL_CLASS_NAME = "CDBMaterialsClass";
static std::string BASE_MATERIAL_ENUM_NAME = "CDBBaseMaterial";

void CDBMaterials::readMaterialsFile(std::filesystem::path materialsXmlPath)
{
    rapidxml::file<> xmlFile(materialsXmlPath.c_str());
    rapidxml::xml_document<> xml;
    xml.parse<0>(xmlFile.data());

    rapidxml::xml_node<> *baseMaterialTableNode = xml.first_node("Base_Material_Table");
    int i = 0;
    for (rapidxml::xml_node<> *baseMaterialNode = baseMaterialTableNode->first_node("Base_Material");
         baseMaterialNode;
         baseMaterialNode = baseMaterialNode->next_sibling()) {
        std::string name = baseMaterialNode->first_node("Name")->value();
        std::string description = baseMaterialNode->first_node("Description")->value();

        CDBBaseMaterial baseMaterial(name, description, i++);
        baseMaterials.insert(std::make_pair(name, baseMaterial));
    }
}

nlohmann::json CDBMaterials::generateSchema()
{
    nlohmann::json materialClass = nlohmann::json::object();
    // Composite material names.
    materialClass["properties"]["name"] = {{"type", "STRING"}};
    // Composite material substrate compositions.
    materialClass["properties"]["substrates"] = {{"type", "ARRAY"},
                                                 {"componentType", "ENUM"},
                                                 {"enumType", BASE_MATERIAL_ENUM_NAME}};
    // Composite material substrate weights.
    materialClass["properties"]["weights"] = {{"type", "ARRAY"}, {"componentType", "UINT8"}};
    // Composite material substrate weights.
    materialClass["properties"]["debugId"] = {{"type", "UINT8"}};

    // Enum of base materials
    nlohmann::json baseMaterialEnum = nlohmann::json::object();
    baseMaterialEnum["valueType"] = "UINT8";
    nlohmann::json values = nlohmann::json::array();
    for (auto const &[key, material] : baseMaterials) {
        values.emplace_back(nlohmann::json{{"name", material.name},
                                           {"description", material.description},
                                           {"value", material.value}});
    }
    baseMaterialEnum["values"] = values;
    
    nlohmann::json schema = nlohmann::json::object();
    schema["classes"][MATERIAL_CLASS_NAME] = materialClass;
    schema["enums"][BASE_MATERIAL_ENUM_NAME] = baseMaterialEnum;

    return schema;
}

} // namespace CDBTo3DTiles