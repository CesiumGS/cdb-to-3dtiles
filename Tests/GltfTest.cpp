#include "Gltf.h"
#include "catch2/catch.hpp"

using namespace CDBTo3DTiles;

static Mesh createTriangleMesh()
{
    Mesh mesh;
    mesh.aabb = AABB();
    mesh.positions = {glm::dvec3(-0.5, 0.0, 0.0), glm::dvec3(0.0, 0.5, 0.0), glm::dvec3(0.5, 0.0, 0.0)};
    mesh.normals.resize(3, glm::vec3(0.0f, 0.0f, 1.0f));

    for (const auto &position : mesh.positions) {
        mesh.aabb->merge(position);
        mesh.positionRTCs.emplace_back(static_cast<glm::vec3>(position));
    }

    return mesh;
}

static double calculateMaterialRoughness(const Material &material)
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

    return roughnessFactor;
}

TEST_CASE("Test creating Gltf with one mesh", "[Gltf]")
{
    SECTION("Test no material and texture")
    {
        Mesh triangleMesh = createTriangleMesh();
        tinygltf::Model model = createGltf(triangleMesh, nullptr, nullptr);

        const auto &modelMeshes = model.meshes;
        const auto &modelMaterials = model.materials;
        const auto &modelTextures = model.textures;
        const auto &modelImages = model.images;
        REQUIRE(modelMeshes.size() == 1);
        REQUIRE(modelMaterials.size() == 0);
        REQUIRE(modelTextures.size() == 0);
        REQUIRE(modelImages.size() == 0);

        // check buffer view
        const auto &bufferViews = model.bufferViews;
        REQUIRE(bufferViews.size() == 2);

        const auto &positionBufferView = bufferViews[0];
        REQUIRE(positionBufferView.buffer == 0);
        REQUIRE(positionBufferView.byteOffset == 0);
        REQUIRE(positionBufferView.byteLength == triangleMesh.positionRTCs.size() * sizeof(glm::vec3));
        REQUIRE(positionBufferView.target == TINYGLTF_TARGET_ARRAY_BUFFER);

        const auto &normalBufferView = bufferViews[1];
        REQUIRE(normalBufferView.buffer == 0);
        REQUIRE(normalBufferView.byteOffset == triangleMesh.positionRTCs.size() * sizeof(glm::vec3));
        REQUIRE(normalBufferView.byteLength == triangleMesh.normals.size() * sizeof(glm::vec3));
        REQUIRE(normalBufferView.target == TINYGLTF_TARGET_ARRAY_BUFFER);

        // check accessor
        const auto &accessors = model.accessors;
        REQUIRE(accessors.size() == 2);

        const auto &positionAccessor = accessors[0];
        REQUIRE(positionAccessor.bufferView == 0);
        REQUIRE(positionAccessor.byteOffset == 0);
        REQUIRE(positionAccessor.count == triangleMesh.positionRTCs.size());
        REQUIRE(positionAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        REQUIRE(positionAccessor.type == TINYGLTF_TYPE_VEC3);

        const auto &normalAccessor = accessors[1];
        REQUIRE(normalAccessor.bufferView == 1);
        REQUIRE(normalAccessor.byteOffset == 0);
        REQUIRE(normalAccessor.count == triangleMesh.normals.size());
        REQUIRE(normalAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        REQUIRE(normalAccessor.type == TINYGLTF_TYPE_VEC3);

        // check mesh property
        const auto &mesh = modelMeshes.front();
        const auto &primitives = mesh.primitives;
        REQUIRE(primitives.size() == 1);

        const auto &primitive = primitives.front();
        REQUIRE(primitive.mode == TINYGLTF_MODE_TRIANGLES);
        REQUIRE(primitive.material == -1);
        REQUIRE(primitive.attributes.at("POSITION") == 0);
        REQUIRE(primitive.attributes.at("NORMAL") == 1);
        REQUIRE(primitive.attributes.find("TEXCOORD_0") == primitive.attributes.end());
        REQUIRE(primitive.attributes.find("_BATCHID") == primitive.attributes.end());
    }

    SECTION("Test with material and no texture")
    {
        Mesh triangleMesh = createTriangleMesh();
        Material material;
        tinygltf::Model model = createGltf(triangleMesh, &material, nullptr);

        const auto &modelMaterials = model.materials;
        REQUIRE(model.extensionsUsed.size() == 0); // no unlit extension is used here
        REQUIRE(modelMaterials.size() == 1);

        const auto &modelMaterial = modelMaterials[0];
        REQUIRE(modelMaterial.alphaMode == "MASK");
        REQUIRE(modelMaterial.doubleSided == false);
        REQUIRE(modelMaterial.extensions.find("KHR_materials_unlit") == modelMaterial.extensions.end());

        const auto &pbr = modelMaterial.pbrMetallicRoughness;
        REQUIRE(pbr.baseColorTexture.index == -1);
        REQUIRE(pbr.baseColorFactor[0] == Approx(material.diffuse.r));
        REQUIRE(pbr.baseColorFactor[1] == Approx(material.diffuse.g));
        REQUIRE(pbr.baseColorFactor[2] == Approx(material.diffuse.b));
        REQUIRE(pbr.baseColorFactor[3] == Approx(material.alpha));
        REQUIRE(pbr.roughnessFactor == Approx(calculateMaterialRoughness(material)));
        REQUIRE(pbr.metallicFactor == Approx(0.0));
    }

    SECTION("Test with material and texture")
    {
        Mesh triangleMesh = createTriangleMesh();

        Material material;
        material.texture = 0;

        Texture texture;
        texture.uri = "textureURI";
        texture.magFilter = TextureFilter::LINEAR;
        texture.minFilter = TextureFilter::LINEAR_MIPMAP_LINEAR;

        tinygltf::Model model = createGltf(triangleMesh, &material, &texture);

        // check that material has texture index
        const auto &modelMaterials = model.materials;
        REQUIRE(modelMaterials.size() == 1);

        const auto &modelMaterial = modelMaterials[0];
        const auto &pbr = modelMaterial.pbrMetallicRoughness;
        REQUIRE(pbr.baseColorTexture.index == 0);

        // check texture property
        const auto &modelTextures = model.textures;
        REQUIRE(modelTextures.size() == 1);

        const auto &modelTexture = modelTextures.front();
        REQUIRE(modelTexture.sampler == 0);
        REQUIRE(modelTexture.source == 0);

        // check sampler
        const auto &modelSamplers = model.samplers;
        REQUIRE(modelSamplers.size() == 1);

        const auto &modelSampler = modelSamplers.front();
        REQUIRE(modelSampler.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR);
        REQUIRE(modelSampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR);

        // check image sources
        const auto &modelImages = model.images;
        REQUIRE(modelImages.size() == 1);

        const auto &modelImage = modelImages.front();
        REQUIRE(modelImage.uri == "textureURI");
    }

    SECTION("Test unlit material")
    {
        Mesh triangleMesh = createTriangleMesh();
        Material material;
        material.unlit = true;

        tinygltf::Model model = createGltf(triangleMesh, &material, nullptr);
        REQUIRE(model.extensionsUsed.front() == "KHR_materials_unlit");

        const auto &modelMaterials = model.materials;
        const auto &modelMaterial = modelMaterials[0];
        REQUIRE(modelMaterial.alphaMode == "MASK");
        REQUIRE(modelMaterial.doubleSided == false);
        REQUIRE(modelMaterial.extensions.find("KHR_materials_unlit") != modelMaterial.extensions.end());
    }

    SECTION("Test material with wrong texture index")
    {
        Mesh triangleMesh = createTriangleMesh();

        Material material;

        Texture texture;
        texture.uri = "textureURI";
        texture.magFilter = TextureFilter::LINEAR;
        texture.minFilter = TextureFilter::LINEAR_MIPMAP_LINEAR;

        REQUIRE_THROWS_AS(createGltf(triangleMesh, &material, &texture), std::invalid_argument);
    }

    SECTION("Test material with texture index not -1 when no texture present")
    {
        Mesh triangleMesh = createTriangleMesh();

        Material material;
        material.texture = 0;

        REQUIRE_THROWS_AS(createGltf(triangleMesh, &material, nullptr), std::invalid_argument);
    }
}

TEST_CASE("Test converting multiple meshes to gltf", "[Gltf]")
{
    std::vector<Mesh> meshes(2, createTriangleMesh());
    meshes[0].material = 0;
    meshes[1].material = 1;

    std::vector<Material> materials(2, Material());
    materials[0].texture = -1;
    materials[1].texture = 0;

    std::vector<Texture> textures(1, Texture());
    textures[0].uri = "textureURI";
    textures[0].magFilter = TextureFilter::LINEAR;
    textures[0].minFilter = TextureFilter::LINEAR;

    auto model = createGltf(meshes, materials, textures);

    // check gltf meshes
    const auto &modelMeshes = model.meshes;
    const auto &bufferViews = model.bufferViews;
    const auto &accessors = model.accessors;
    size_t bufferOffset = 0;
    for (size_t i = 0; i < meshes.size(); ++i) {
        // check buffer view
        const auto &positionBufferView = bufferViews[i * 2 + 0];
        REQUIRE(positionBufferView.buffer == 0);
        REQUIRE(positionBufferView.byteOffset == bufferOffset);
        REQUIRE(positionBufferView.byteLength == meshes[i].positionRTCs.size() * sizeof(glm::vec3));
        REQUIRE(positionBufferView.target == TINYGLTF_TARGET_ARRAY_BUFFER);

        bufferOffset += meshes[i].positionRTCs.size() * sizeof(glm::vec3);
        const auto &normalBufferView = bufferViews[i * 2 + 1];
        REQUIRE(normalBufferView.buffer == 0);
        REQUIRE(normalBufferView.byteOffset == bufferOffset);
        REQUIRE(normalBufferView.byteLength == meshes[i].normals.size() * sizeof(glm::vec3));
        REQUIRE(normalBufferView.target == TINYGLTF_TARGET_ARRAY_BUFFER);

        bufferOffset += meshes[i].normals.size() * sizeof(glm::vec3);

        // check accessor
        const auto &positionAccessor = accessors[i * 2 + 0];
        REQUIRE(positionAccessor.bufferView == static_cast<int>(i * 2 + 0));
        REQUIRE(positionAccessor.byteOffset == 0);
        REQUIRE(positionAccessor.count == meshes[i].positionRTCs.size());
        REQUIRE(positionAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        REQUIRE(positionAccessor.type == TINYGLTF_TYPE_VEC3);

        const auto &normalAccessor = accessors[i * 2 + 1];
        REQUIRE(normalAccessor.bufferView == static_cast<int>(i * 2 + 1));
        REQUIRE(normalAccessor.byteOffset == 0);
        REQUIRE(normalAccessor.count == meshes[i].normals.size());
        REQUIRE(normalAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        REQUIRE(normalAccessor.type == TINYGLTF_TYPE_VEC3);

        // check mesh property
        const auto &primitives = modelMeshes[i].primitives;
        REQUIRE(primitives.size() == 1);

        const auto &primitive = primitives.front();
        REQUIRE(primitive.mode == TINYGLTF_MODE_TRIANGLES);
        REQUIRE(primitive.material == meshes[i].material);
        REQUIRE(primitive.attributes.at("POSITION") == static_cast<int>(i * 2 + 0));
        REQUIRE(primitive.attributes.at("NORMAL") == static_cast<int>(i * 2 + 1));
        REQUIRE(primitive.attributes.find("TEXCOORD_0") == primitive.attributes.end());
        REQUIRE(primitive.attributes.find("_BATCHID") == primitive.attributes.end());
    }

    // check materials
    const auto &modelMaterials = model.materials;
    for (size_t i = 0; i < materials.size(); ++i) {
        REQUIRE(model.extensionsUsed.size() == 0); // no unlit extension is used here

        const auto &modelMaterial = modelMaterials[i];
        REQUIRE(modelMaterial.alphaMode == "MASK");
        REQUIRE(modelMaterial.doubleSided == false);
        REQUIRE(modelMaterial.extensions.find("KHR_materials_unlit") == modelMaterial.extensions.end());

        const auto &pbr = modelMaterial.pbrMetallicRoughness;
        REQUIRE(pbr.baseColorTexture.index == materials[i].texture);
        REQUIRE(pbr.baseColorFactor[0] == Approx(materials[i].diffuse.r));
        REQUIRE(pbr.baseColorFactor[1] == Approx(materials[i].diffuse.g));
        REQUIRE(pbr.baseColorFactor[2] == Approx(materials[i].diffuse.b));
        REQUIRE(pbr.baseColorFactor[3] == Approx(materials[i].alpha));
        REQUIRE(pbr.roughnessFactor == Approx(calculateMaterialRoughness(materials[i])));
        REQUIRE(pbr.metallicFactor == Approx(0.0));
    }

    // check textures
    const auto &modelTextures = model.textures;
    const auto &modelImages = model.images;
    const auto &modelTexture = modelTextures.front();
    REQUIRE(modelTexture.sampler == 0);
    REQUIRE(modelTexture.source == 0);

    // check sampler
    const auto &modelSamplers = model.samplers;
    REQUIRE(modelSamplers.size() == 1);

    const auto &modelSampler = modelSamplers.front();
    REQUIRE(modelSampler.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR);
    REQUIRE(modelSampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR);

    // check image sources
    const auto &modelImage = modelImages.front();
    REQUIRE(modelImage.uri == "textureURI");
}
