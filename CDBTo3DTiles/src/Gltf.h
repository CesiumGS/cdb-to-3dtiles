#pragma once

#define NLOHMANN_JSON_HPP

#include <nlohmann/json.hpp>
#include "Scene.h"
#include "tiny_gltf.h"
#include <filesystem>
#include <functional>
#include <unordered_set>
#include <vector>

namespace CDBTo3DTiles {

tinygltf::Model createGltf(const Mesh &mesh, const Material *material, const Texture *texture, const Texture *featureIdTexture);

tinygltf::Model createGltf(const std::vector<Mesh> &meshes,
                           const std::vector<Material> &materials,
                           const std::vector<Texture> &textures);

uint createMetadataBufferView(tinygltf::Model *gltf, std::vector<uint8_t> data);
uint createMetadataBufferView(tinygltf::Model *gltf, std::vector<std::vector<uint8_t>> strings, size_t stringsByteLength);

bool ParseJsonAsValue(tinygltf::Value *ret, const nlohmann::json &o);

} // namespace CDBTo3DTiles
