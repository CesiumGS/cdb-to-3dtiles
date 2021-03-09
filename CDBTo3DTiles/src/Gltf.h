#pragma once

#include "Scene.h"
#include "tiny_gltf.h"
#include <filesystem>
#include <functional>
#include <unordered_set>
#include <vector>

namespace CDBTo3DTiles {

tinygltf::Model createGltf(const Mesh &mesh, const Material *material, const Texture *texture, bool use3dTilesNext = false);

tinygltf::Model createGltf(const std::vector<Mesh> &meshes,
                           const std::vector<Material> &materials,
                           const std::vector<Texture> &textures,
                           bool use3dTilesNext = false);

void combineGltfs(tinygltf::Model *model, std::vector<tinygltf::Model> glbs);

} // namespace CDBTo3DTiles
