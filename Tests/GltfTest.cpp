#include "Gltf.h"
#include <doctest/doctest.h>

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

TEST_SUITE_BEGIN("Gltf");

TEST_CASE("Test creating Gltf with one mesh")
{
    SUBCASE("Test no material and texture")
    {
        Mesh triangleMesh = createTriangleMesh();
        tinygltf::Model model = createGltf(triangleMesh, nullptr, nullptr);

        const auto &modelMeshes = model.meshes;
        const auto &modelMaterials = model.materials;
        const auto &modelTextures = model.textures;
        const auto &modelImages = model.images;
        CHECK(modelMeshes.size() == 1);
        CHECK(modelMaterials.size() == 0);
        CHECK(modelTextures.size() == 0);
        CHECK(modelImages.size() == 0);

        // check buffer view
        const auto &bufferViews = model.bufferViews;
        CHECK(bufferViews.size() == 2);

        const auto &positionBufferView = bufferViews[0];
        CHECK(positionBufferView.buffer == 0);
        CHECK(positionBufferView.byteOffset == 0);
        CHECK(positionBufferView.byteLength == triangleMesh.positionRTCs.size() * sizeof(glm::vec3));
        CHECK(positionBufferView.target == TINYGLTF_TARGET_ARRAY_BUFFER);

        const auto &normalBufferView = bufferViews[1];
        CHECK(normalBufferView.buffer == 0);
        CHECK(normalBufferView.byteOffset == triangleMesh.positionRTCs.size() * sizeof(glm::vec3));
        CHECK(normalBufferView.byteLength == triangleMesh.normals.size() * sizeof(glm::vec3));
        CHECK(normalBufferView.target == TINYGLTF_TARGET_ARRAY_BUFFER);

        // check accessor
        const auto &accessors = model.accessors;
        CHECK(accessors.size() == 2);

        const auto &positionAccessor = accessors[0];
        CHECK(positionAccessor.bufferView == 0);
        CHECK(positionAccessor.byteOffset == 0);
        CHECK(positionAccessor.count == triangleMesh.positionRTCs.size());
        CHECK(positionAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        CHECK(positionAccessor.type == TINYGLTF_TYPE_VEC3);

        const auto &normalAccessor = accessors[1];
        CHECK(normalAccessor.bufferView == 1);
        CHECK(normalAccessor.byteOffset == 0);
        CHECK(normalAccessor.count == triangleMesh.normals.size());
        CHECK(normalAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        CHECK(normalAccessor.type == TINYGLTF_TYPE_VEC3);

        // check mesh property
        const auto &mesh = modelMeshes.front();
        const auto &primitives = mesh.primitives;
        CHECK(primitives.size() == 1);

        const auto &primitive = primitives.front();
        CHECK(primitive.mode == TINYGLTF_MODE_TRIANGLES);
        CHECK(primitive.material == -1);
        CHECK(primitive.attributes.at("POSITION") == 0);
        CHECK(primitive.attributes.at("NORMAL") == 1);
        CHECK(primitive.attributes.find("TEXCOORD_0") == primitive.attributes.end());
        CHECK(primitive.attributes.find("_BATCHID") == primitive.attributes.end());
    }

    SUBCASE("Test with material and no texture")
    {
        Mesh triangleMesh = createTriangleMesh();
        Material material;
        tinygltf::Model model = createGltf(triangleMesh, &material, nullptr);

        const auto &modelMaterials = model.materials;
        CHECK(model.extensionsUsed.size() == 0); // no unlit extension is used here
        CHECK(modelMaterials.size() == 1);

        const auto &modelMaterial = modelMaterials[0];
        CHECK(modelMaterial.alphaMode == "MASK");
        CHECK(modelMaterial.doubleSided == false);
        CHECK(modelMaterial.extensions.find("KHR_materials_unlit") == modelMaterial.extensions.end());

        const auto &pbr = modelMaterial.pbrMetallicRoughness;
        CHECK(pbr.baseColorTexture.index == -1);
        CHECK(pbr.baseColorFactor[0] == doctest::Approx(material.diffuse.r));
        CHECK(pbr.baseColorFactor[1] == doctest::Approx(material.diffuse.g));
        CHECK(pbr.baseColorFactor[2] == doctest::Approx(material.diffuse.b));
        CHECK(pbr.baseColorFactor[3] == doctest::Approx(material.alpha));
        CHECK(pbr.roughnessFactor == doctest::Approx(calculateMaterialRoughness(material)));
        CHECK(pbr.metallicFactor == doctest::Approx(0.0));
    }

    SUBCASE("Test with material and texture")
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
        CHECK(modelMaterials.size() == 1);

        const auto &modelMaterial = modelMaterials[0];
        const auto &pbr = modelMaterial.pbrMetallicRoughness;
        CHECK(pbr.baseColorTexture.index == 0);

        // check texture property
        const auto &modelTextures = model.textures;
        CHECK(modelTextures.size() == 1);

        const auto &modelTexture = modelTextures.front();
        CHECK(modelTexture.sampler == 0);
        CHECK(modelTexture.source == 0);

        // check sampler
        const auto &modelSamplers = model.samplers;
        CHECK(modelSamplers.size() == 1);

        const auto &modelSampler = modelSamplers.front();
        CHECK(modelSampler.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR);
        CHECK(modelSampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR);

        // check image sources
        const auto &modelImages = model.images;
        CHECK(modelImages.size() == 1);

        const auto &modelImage = modelImages.front();
        CHECK(modelImage.uri == "textureURI");
    }

    SUBCASE("Test unlit material")
    {
        Mesh triangleMesh = createTriangleMesh();
        Material material;
        material.unlit = true;

        tinygltf::Model model = createGltf(triangleMesh, &material, nullptr);
        CHECK(model.extensionsUsed.front() == "KHR_materials_unlit");

        const auto &modelMaterials = model.materials;
        const auto &modelMaterial = modelMaterials[0];
        CHECK(modelMaterial.alphaMode == "MASK");
        CHECK(modelMaterial.doubleSided == false);
        CHECK(modelMaterial.extensions.find("KHR_materials_unlit") != modelMaterial.extensions.end());
    }

    SUBCASE("Test material with wrong texture index")
    {
        Mesh triangleMesh = createTriangleMesh();

        Material material;

        Texture texture;
        texture.uri = "textureURI";
        texture.magFilter = TextureFilter::LINEAR;
        texture.minFilter = TextureFilter::LINEAR_MIPMAP_LINEAR;

        CHECK_THROWS_AS(createGltf(triangleMesh, &material, &texture), std::invalid_argument);
    }

    SUBCASE("Test material with texture index not -1 when no texture present")
    {
        Mesh triangleMesh = createTriangleMesh();

        Material material;
        material.texture = 0;

        CHECK_THROWS_AS(createGltf(triangleMesh, &material, nullptr), std::invalid_argument);
    }
}

TEST_CASE("Test converting multiple meshes to gltf")
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
        CHECK(positionBufferView.buffer == 0);
        CHECK(positionBufferView.byteOffset == bufferOffset);
        CHECK(positionBufferView.byteLength == meshes[i].positionRTCs.size() * sizeof(glm::vec3));
        CHECK(positionBufferView.target == TINYGLTF_TARGET_ARRAY_BUFFER);

        bufferOffset += meshes[i].positionRTCs.size() * sizeof(glm::vec3);
        const auto &normalBufferView = bufferViews[i * 2 + 1];
        CHECK(normalBufferView.buffer == 0);
        CHECK(normalBufferView.byteOffset == bufferOffset);
        CHECK(normalBufferView.byteLength == meshes[i].normals.size() * sizeof(glm::vec3));
        CHECK(normalBufferView.target == TINYGLTF_TARGET_ARRAY_BUFFER);

        bufferOffset += meshes[i].normals.size() * sizeof(glm::vec3);

        // check accessor
        const auto &positionAccessor = accessors[i * 2 + 0];
        CHECK(positionAccessor.bufferView == static_cast<int>(i * 2 + 0));
        CHECK(positionAccessor.byteOffset == 0);
        CHECK(positionAccessor.count == meshes[i].positionRTCs.size());
        CHECK(positionAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        CHECK(positionAccessor.type == TINYGLTF_TYPE_VEC3);

        const auto &normalAccessor = accessors[i * 2 + 1];
        CHECK(normalAccessor.bufferView == static_cast<int>(i * 2 + 1));
        CHECK(normalAccessor.byteOffset == 0);
        CHECK(normalAccessor.count == meshes[i].normals.size());
        CHECK(normalAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        CHECK(normalAccessor.type == TINYGLTF_TYPE_VEC3);

        // check mesh property
        const auto &primitives = modelMeshes[i].primitives;
        CHECK(primitives.size() == 1);

        const auto &primitive = primitives.front();
        CHECK(primitive.mode == TINYGLTF_MODE_TRIANGLES);
        CHECK(primitive.material == meshes[i].material);
        CHECK(primitive.attributes.at("POSITION") == static_cast<int>(i * 2 + 0));
        CHECK(primitive.attributes.at("NORMAL") == static_cast<int>(i * 2 + 1));
        CHECK(primitive.attributes.find("TEXCOORD_0") == primitive.attributes.end());
        CHECK(primitive.attributes.find("_BATCHID") == primitive.attributes.end());
    }

    // check materials
    const auto &modelMaterials = model.materials;
    for (size_t i = 0; i < materials.size(); ++i) {
        CHECK(model.extensionsUsed.size() == 0); // no unlit extension is used here

        const auto &modelMaterial = modelMaterials[i];
        CHECK(modelMaterial.alphaMode == "MASK");
        CHECK(modelMaterial.doubleSided == false);
        CHECK(modelMaterial.extensions.find("KHR_materials_unlit") == modelMaterial.extensions.end());

        const auto &pbr = modelMaterial.pbrMetallicRoughness;
        CHECK(pbr.baseColorTexture.index == materials[i].texture);
        CHECK(pbr.baseColorFactor[0] == doctest::Approx(materials[i].diffuse.r));
        CHECK(pbr.baseColorFactor[1] == doctest::Approx(materials[i].diffuse.g));
        CHECK(pbr.baseColorFactor[2] == doctest::Approx(materials[i].diffuse.b));
        CHECK(pbr.baseColorFactor[3] == doctest::Approx(materials[i].alpha));
        CHECK(pbr.roughnessFactor == doctest::Approx(calculateMaterialRoughness(materials[i])));
        CHECK(pbr.metallicFactor == doctest::Approx(0.0));
    }

    // check textures
    const auto &modelTextures = model.textures;
    const auto &modelImages = model.images;
    const auto &modelTexture = modelTextures.front();
    CHECK(modelTexture.sampler == 0);
    CHECK(modelTexture.source == 0);

    // check sampler
    const auto &modelSamplers = model.samplers;
    CHECK(modelSamplers.size() == 1);

    const auto &modelSampler = modelSamplers.front();
    CHECK(modelSampler.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR);
    CHECK(modelSampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR);

    // check image sources
    const auto &modelImage = modelImages.front();
    CHECK(modelImage.uri == "textureURI");
}

TEST_SUITE_END();
