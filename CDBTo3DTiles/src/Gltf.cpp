#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Gltf.h"
#include "Utility.h"

namespace std {
template<>
struct hash<tinygltf::Sampler>
{
    size_t operator()(tinygltf::Sampler const &sampler) const noexcept
    {
        size_t seed = 0;
        CDBTo3DTiles::hashCombine(seed, sampler.minFilter);
        CDBTo3DTiles::hashCombine(seed, sampler.magFilter);
        CDBTo3DTiles::hashCombine(seed, sampler.wrapS);
        CDBTo3DTiles::hashCombine(seed, sampler.wrapT);
        CDBTo3DTiles::hashCombine(seed, sampler.wrapR);
        return seed;
    }
};

} // namespace std

namespace CDBTo3DTiles {

static const std::string CDB_MATERIAL_CLASS_NAME = "CDBMaterialClass";
static const std::string CDB_MATERIAL_PROPERTY_NAME = "compositeMaterialName";
static const std::string CDB_MATERIAL_FEATURE_TABLE_NAME = "CDBMaterialFeatureTable";

static void createGltfTexture(const Texture &texture,
                              tinygltf::Model &gltf,
                              std::unordered_map<tinygltf::Sampler, unsigned> *samplerCache);

static void createGltfMaterial(const Material &material, tinygltf::Model &gltf);

static size_t createGltfMesh(const Mesh &mesh,
                             size_t rootIndex,
                             tinygltf::Model &gltf,
                             std::vector<unsigned char> &bufferData,
                             size_t bufferOffset);

static int primitiveTypeToGltfMode(PrimitiveType type);

static void createBufferAndAccessor(tinygltf::Model &modelGltf,
                                    void *destBuffer,
                                    const void *sourceBuffer,
                                    size_t bufferIndex,
                                    size_t bufferViewOffset,
                                    size_t bufferViewLength,
                                    int bufferViewTarget,
                                    size_t accessorComponentCount,
                                    int accessorComponentType,
                                    int accessorType);

static int convertToGltfFilterMode(TextureFilter mode);

tinygltf::Model createGltf(const Mesh &mesh, const Material *material, const Texture *texture, const Texture *featureIdTexture)
{
    static const std::filesystem::path TEXTURE_SUB_DIR = "Textures";

    tinygltf::Model gltf;
    gltf.asset.version = "2.0";
    if (material && material->unlit) {
        gltf.extensionsUsed.emplace_back("KHR_materials_unlit");
    }

    // create root nodes
    tinygltf::Node rootNodeGltf;
    rootNodeGltf.matrix = {1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
    gltf.nodes.emplace_back(rootNodeGltf);

    // create buffer
    size_t totalBufferSize = mesh.indices.size() * sizeof(uint32_t) + mesh.batchIDs.size() * sizeof(float)
                             + mesh.positionRTCs.size() * sizeof(glm::vec3)
                             + mesh.normals.size() * sizeof(glm::vec3) + mesh.UVs.size() * sizeof(glm::vec2);

    tinygltf::Buffer bufferGltf;
    auto &bufferData = bufferGltf.data;
    bufferData.resize(totalBufferSize);

    // add mesh
    size_t bufferOffset = 0;
    createGltfMesh(mesh, 0, gltf, bufferData, bufferOffset);

    // add material
    if (material) {
        // add texture
        if (texture && !texture->uri.empty()) {
            if (material->texture != 0) {
                throw std::invalid_argument("Material texture must have index 0 when texture is present");
            }

            createGltfTexture(*texture, gltf, nullptr);
        } else if (material->texture != -1) {
            throw std::invalid_argument("Material texture must have index -1 when no texture is used");
        }

        // add material
        createGltfMaterial(*material, gltf);
    }

    // Add Feature ID texture from RMTexture
    if (featureIdTexture) {
        createGltfTexture(*featureIdTexture, gltf, nullptr);
        
        nlohmann::json primitiveMetadataExtension = nlohmann::json::object();
        primitiveMetadataExtension["featureIdTextures"] = {
            {
                { "featureTable", CDB_MATERIAL_FEATURE_TABLE_NAME },
                { "featureIds",
                    {
                        { "texture",
                            {
                                { "texCoord", 0 },
                                { "index", gltf.textures.size() - 1 }
                            } 
                        },
                        { "channels", "r" }
                    }
                }
            }
        };
        
        tinygltf::Value primitiveMetadataExtensionValue;
        tinygltf::ParseJsonAsValue(&primitiveMetadataExtensionValue, primitiveMetadataExtension);
        gltf.meshes[0].primitives[0].extensions.insert(std::pair<std::string, tinygltf::Value>(std::string("EXT_feature_metadata"), primitiveMetadataExtensionValue));
    }

    // add buffer to the model
    gltf.buffers.emplace_back(bufferGltf);

    // create scene
    tinygltf::Scene sceneGltf;
    sceneGltf.nodes.emplace_back(0);
    gltf.scenes.emplace_back(sceneGltf);

    return gltf;
}

tinygltf::Model createGltf(const std::vector<Mesh> &meshes,
                           const std::vector<Material> &materials,
                           const std::vector<Texture> &textures)
{
    static const std::filesystem::path TEXTURE_SUB_DIR = "Textures";

    tinygltf::Model gltf;
    gltf.asset.version = "2.0";

    // create root node
    tinygltf::Node rootNodeGltf;
    rootNodeGltf.matrix = {1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
    gltf.nodes.emplace_back(rootNodeGltf);

    // create texture
    std::unordered_map<tinygltf::Sampler, unsigned> samplers;
    for (const auto &texture : textures) {
        const auto &textureFilename = texture.uri;
        if (textureFilename.empty()) {
            continue;
        }

        createGltfTexture(texture, gltf, &samplers);
    }

    // create material
    bool isUnlitMaterialUsed = false;
    for (const auto &material : materials) {
        createGltfMaterial(material, gltf);
        if (material.unlit) {
            isUnlitMaterialUsed = true;
        }
    }

    if (isUnlitMaterialUsed) {
        gltf.extensionsUsed.emplace_back("KHR_materials_unlit");
    }

    // create mesh node
    tinygltf::Buffer bufferGltf;
    size_t totalBufferSize = 0;
    for (const auto &mesh : meshes) {
        totalBufferSize += mesh.indices.size() * sizeof(uint32_t) + mesh.batchIDs.size() * sizeof(float)
                           + mesh.positionRTCs.size() * sizeof(glm::vec3)
                           + mesh.normals.size() * sizeof(glm::vec3) + mesh.UVs.size() * sizeof(glm::vec2);
    }

    auto &bufferData = bufferGltf.data;
    bufferData.resize(totalBufferSize);
    size_t bufferOffset = 0;
    for (const auto &mesh : meshes) {
        bufferOffset += createGltfMesh(mesh, 0, gltf, bufferData, bufferOffset);
    }

    // add buffer to the model
    gltf.buffers.emplace_back(bufferGltf);

    // create scene
    tinygltf::Scene sceneGltf;
    sceneGltf.nodes.emplace_back(0);
    gltf.scenes.emplace_back(sceneGltf);

    return gltf;
}

void createGltfTexture(const Texture &texture,
                       tinygltf::Model &gltf,
                       std::unordered_map<tinygltf::Sampler, unsigned> *samplerCache)
{
    tinygltf::Sampler sampler;
    sampler.minFilter = convertToGltfFilterMode(texture.minFilter);
    sampler.magFilter = convertToGltfFilterMode(texture.magFilter);
    int samplerIndex = -1;

    // search sampler in the cache first
    if (samplerCache) {
        auto existSampler = samplerCache->find(sampler);
        if (existSampler == samplerCache->end()) {
            gltf.samplers.emplace_back(sampler);
            samplerIndex = static_cast<int>(gltf.samplers.size() - 1);
            samplerCache->insert({sampler, samplerIndex});
        } else {
            samplerIndex = existSampler->second;
        }
    } else {
        gltf.samplers.emplace_back(sampler);
        samplerIndex = static_cast<int>(gltf.samplers.size() - 1);
    }

    tinygltf::Image imageGltf;
    imageGltf.uri = texture.uri;
    gltf.images.emplace_back(imageGltf);

    tinygltf::Texture textureGltf;
    textureGltf.sampler = samplerIndex;
    textureGltf.source = static_cast<int>(gltf.images.size() - 1);
    gltf.textures.emplace_back(textureGltf);
}

void createGltfMaterial(const Material &material, tinygltf::Model &gltf)
{
    glm::vec3 specularColor = material.specular;
    float specularIntensity = specularColor.r * 0.2125f + specularColor.g * 0.7154f
                              + specularColor.b * 0.0721f;

    float roughnessFactor = material.shininess;
    roughnessFactor = material.shininess / 1000.0f;
    roughnessFactor = 1.0f - roughnessFactor;
    roughnessFactor = glm::clamp(roughnessFactor, 0.0f, 1.0f);

    if (specularIntensity < 0.0) {
        roughnessFactor *= (1.0f - specularIntensity);
    }

    tinygltf::Material materialGltf;
    if (material.texture != -1) {
        materialGltf.pbrMetallicRoughness.baseColorTexture.index = material.texture;
    }

    materialGltf.doubleSided = material.doubleSided;
    materialGltf.alphaMode = "MASK";
    if (glm::equal(material.diffuse, glm::vec3(0.0f)) != glm::bvec3(true)) {
        materialGltf.pbrMetallicRoughness.baseColorFactor = {material.diffuse.r,
                                                             material.diffuse.g,
                                                             material.diffuse.b,
                                                             material.alpha};
    }
    materialGltf.pbrMetallicRoughness.roughnessFactor = roughnessFactor;
    materialGltf.pbrMetallicRoughness.metallicFactor = 0.0;

    if (material.unlit) {
        materialGltf.extensions["KHR_materials_unlit"] = {};
    }

    gltf.materials.emplace_back(materialGltf);
}

size_t createGltfMesh(const Mesh &mesh,
                      size_t rootIndex,
                      tinygltf::Model &gltf,
                      std::vector<unsigned char> &bufferData,
                      size_t offset)
{
    std::optional<AABB> aabb = mesh.aabb;
    glm::dvec3 center = aabb ? aabb->center() : glm::dvec3(0.0);
    glm::dvec3 positionMin = aabb ? aabb->min - center : glm::dvec3(0.0);
    glm::dvec3 positionMax = aabb ? aabb->max - center : glm::dvec3(0.0);

    tinygltf::Primitive primitiveGltf;
    primitiveGltf.mode = primitiveTypeToGltfMode(mesh.primitiveType);
    if (mesh.material != -1) {
        primitiveGltf.material = mesh.material;
    }

    auto bufferIndex = gltf.buffers.size();

    size_t nextSize = 0;
    size_t totalMeshSize = 0;

    // copy indices
    if (!mesh.indices.empty()) {
        nextSize = mesh.indices.size() * sizeof(uint32_t);
        createBufferAndAccessor(gltf,
                                bufferData.data() + offset,
                                mesh.indices.data(),
                                bufferIndex,
                                offset,
                                nextSize,
                                TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER,
                                mesh.indices.size(),
                                TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT,
                                TINYGLTF_TYPE_SCALAR);

        primitiveGltf.indices = static_cast<int>(gltf.accessors.size() - 1);
        offset += nextSize;
        totalMeshSize += nextSize;
    }

    // copy batchIDs
    if (!mesh.batchIDs.empty()) {
        nextSize = mesh.batchIDs.size() * sizeof(float);
        createBufferAndAccessor(gltf,
                                bufferData.data() + offset,
                                mesh.batchIDs.data(),
                                bufferIndex,
                                offset,
                                nextSize,
                                TINYGLTF_TARGET_ARRAY_BUFFER,
                                mesh.batchIDs.size(),
                                TINYGLTF_COMPONENT_TYPE_FLOAT,
                                TINYGLTF_TYPE_SCALAR);

        primitiveGltf.attributes["_BATCHID"] = static_cast<int>(gltf.accessors.size() - 1);
        offset += nextSize;
        totalMeshSize += nextSize;
    }

    // copy positions
    if (!mesh.positionRTCs.empty()) {
        nextSize = mesh.positionRTCs.size() * sizeof(glm::vec3);
        createBufferAndAccessor(gltf,
                                bufferData.data() + offset,
                                mesh.positionRTCs.data(),
                                bufferIndex,
                                offset,
                                nextSize,
                                TINYGLTF_TARGET_ARRAY_BUFFER,
                                mesh.positionRTCs.size(),
                                TINYGLTF_COMPONENT_TYPE_FLOAT,
                                TINYGLTF_TYPE_VEC3);

        auto &positionsAccessor = gltf.accessors.back();
        positionsAccessor.minValues = {positionMin.x, positionMin.y, positionMin.z};
        positionsAccessor.maxValues = {positionMax.x, positionMax.y, positionMax.z};

        primitiveGltf.attributes["POSITION"] = static_cast<int>(gltf.accessors.size() - 1);
        offset += nextSize;
        totalMeshSize += nextSize;
    }

    // copy normals
    if (!mesh.normals.empty()) {
        nextSize = mesh.normals.size() * sizeof(glm::vec3);
        createBufferAndAccessor(gltf,
                                bufferData.data() + offset,
                                mesh.normals.data(),
                                bufferIndex,
                                offset,
                                nextSize,
                                TINYGLTF_TARGET_ARRAY_BUFFER,
                                mesh.normals.size(),
                                TINYGLTF_COMPONENT_TYPE_FLOAT,
                                TINYGLTF_TYPE_VEC3);

        primitiveGltf.attributes["NORMAL"] = static_cast<int>(gltf.accessors.size() - 1);
        offset += nextSize;
        totalMeshSize += nextSize;
    }

    // copy uv
    if (!mesh.UVs.empty()) {
        nextSize = mesh.UVs.size() * sizeof(glm::vec2);
        createBufferAndAccessor(gltf,
                                bufferData.data() + offset,
                                mesh.UVs.data(),
                                bufferIndex,
                                offset,
                                nextSize,
                                TINYGLTF_TARGET_ARRAY_BUFFER,
                                mesh.UVs.size(),
                                TINYGLTF_COMPONENT_TYPE_FLOAT,
                                TINYGLTF_TYPE_VEC2);

        primitiveGltf.attributes["TEXCOORD_0"] = static_cast<int>(gltf.accessors.size() - 1);
        offset += nextSize;
        totalMeshSize += nextSize;
    }

    // add mesh
    tinygltf::Mesh meshGltf;
    meshGltf.primitives.emplace_back(primitiveGltf);
    gltf.meshes.emplace_back(meshGltf);

    // create node
    tinygltf::Node meshNode;
    meshNode.mesh = static_cast<int>(gltf.meshes.size() - 1);
    meshNode.translation = {center.x, center.y, center.z};
    gltf.nodes.emplace_back(meshNode);

    // add node to the root
    gltf.nodes[rootIndex].children.emplace_back(gltf.nodes.size() - 1);

    return totalMeshSize;
}

int primitiveTypeToGltfMode(PrimitiveType type)
{
    switch (type) {
    case PrimitiveType::Points:
        return TINYGLTF_MODE_POINTS;
    case PrimitiveType::Lines:
        return TINYGLTF_MODE_LINE;
    case PrimitiveType::LineLoop:
        return TINYGLTF_MODE_LINE_LOOP;
    case PrimitiveType::LineStrip:
        return TINYGLTF_MODE_LINE_STRIP;
    case PrimitiveType::Triangles:
        return TINYGLTF_MODE_TRIANGLES;
    case PrimitiveType::TriangleFan:
        return TINYGLTF_MODE_TRIANGLE_FAN;
    case PrimitiveType::TriangleStrip:
        return TINYGLTF_MODE_TRIANGLE_STRIP;
    default:
        assert(false && "Encountered unknown PrimitiveType");
        return TINYGLTF_MODE_POINTS;
    }
}

int convertToGltfFilterMode(TextureFilter mode)
{
    switch (mode) {
    case TextureFilter::LINEAR:
        return TINYGLTF_TEXTURE_FILTER_LINEAR;
    case TextureFilter::LINEAR_MIPMAP_LINEAR:
        return TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
    case TextureFilter::LINEAR_MIPMAP_NEAREST:
        return TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST;
    case TextureFilter::NEAREST:
        return TINYGLTF_TEXTURE_FILTER_NEAREST;
    case TextureFilter::NEAREST_MIPMAP_LINEAR:
        return TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
    case TextureFilter::NEAREST_MIPMAP_NEAREST:
        return TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST;
    default:
        assert(false && "Encounter unknown TextureFilter");
        return TINYGLTF_TEXTURE_FILTER_LINEAR;
    }
}

void createBufferAndAccessor(tinygltf::Model &modelGltf,
                             void *destBuffer,
                             const void *sourceBuffer,
                             size_t bufferIndex,
                             size_t bufferViewOffset,
                             size_t bufferViewLength,
                             int bufferViewTarget,
                             size_t accessorComponentCount,
                             int accessorComponentType,
                             int accessorType)
{
    std::memcpy(destBuffer, sourceBuffer, bufferViewLength);

    tinygltf::BufferView bufferViewGltf;
    bufferViewGltf.buffer = static_cast<int>(bufferIndex);
    bufferViewGltf.byteOffset = bufferViewOffset;
    bufferViewGltf.byteLength = bufferViewLength;
    bufferViewGltf.target = bufferViewTarget;

    tinygltf::Accessor accessorGltf;
    accessorGltf.bufferView = static_cast<int>(modelGltf.bufferViews.size());
    accessorGltf.byteOffset = 0;
    accessorGltf.count = accessorComponentCount;
    accessorGltf.componentType = accessorComponentType;
    accessorGltf.type = accessorType;

    modelGltf.bufferViews.emplace_back(bufferViewGltf);
    modelGltf.accessors.emplace_back(accessorGltf);
}
} // namespace CDBTo3DTiles
