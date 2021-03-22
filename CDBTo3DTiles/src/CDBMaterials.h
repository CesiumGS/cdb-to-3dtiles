#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <map>

namespace CDBTo3DTiles {

struct CDBBaseMaterial
{
    std::string name;
    std::string description;
    int value;
    // ENHANCEMENT: Add color property to use in styling.

    CDBBaseMaterial(std::string _name, std::string _description, int _value)
        : name(_name)
        , description(_description)
        , value(_value)
    {}
};

struct CDBCompositeMaterial
{
    int index;
    std::string name;
    std::map<CDBBaseMaterial, int> weightedMaterials;
};

class CDBMaterials
{
public:
    void readMaterialsFile(std::filesystem::path materialsXmlPath);
    nlohmann::json generateSchema();
    std::map<std::string, CDBBaseMaterial> baseMaterials;
};
} // namespace CDBTo3DTiles