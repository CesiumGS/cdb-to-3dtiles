#include "Gltf.h"
#include "Config.h"
#include "catch2/catch.hpp"
#include <fstream>
#include <ostream>

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

TEST_CASE("Test writing GLBs with JSON chunks padded to 8 bytes", "[Gltf]")
{
    // Create sample glTF.
    Mesh triangleMesh = createTriangleMesh();
    tinygltf::Model model = createGltf(triangleMesh, nullptr, nullptr);

    // Write GLB.
    std::filesystem::path glbPath = dataPath / "test.glb";
    std::ofstream fs(glbPath, std::ios::binary);
    std::filesystem::current_path(dataPath);
    writePaddedGLB(&model, fs);
    fs.close();

    // Read GLB.
    std::ifstream inFile(glbPath, std::ios_base::binary);
    inFile.seekg(0, std::ios_base::end);
    size_t length = inFile.tellg();
    inFile.seekg(0, std::ios_base::beg);
    std::vector<char> buffer;
    buffer.reserve(length);
    std::copy(std::istreambuf_iterator<char>(inFile),
              std::istreambuf_iterator<char>(),
              std::back_inserter(buffer));

    // Read GLB length.
    uint32_t glbLength;
    std::memcpy(&glbLength, buffer.data() + 8, 4);

    // Read JSON chunk length.
    uint32_t jsonChunkLength;
    std::memcpy(&jsonChunkLength, buffer.data() + 12, 4);

    // Test padding of JSON chunk.
    REQUIRE((20 + jsonChunkLength) % 8 == 0);

    // Remove output.
    std::filesystem::remove_all(glbPath);
}

TEST_CASE("Test combining GLBs", "[Gltf]")
{
    Mesh triangleMesh = createTriangleMesh();

    SECTION("Test combining 0 glTFs")
    {
        // Create target glTF.
        tinygltf::Model gltf;
        gltf.asset.version = "2.0";
        tinygltf::Scene scene;
        scene.nodes = { 0 };
        gltf.scenes.emplace_back(scene);
        tinygltf::Buffer buffer;
        gltf.buffers.emplace_back(buffer);
        tinygltf::Node rootNode;
        rootNode.matrix = {1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
        gltf.nodes.emplace_back(rootNode);

        combineGltfs(&gltf, {});

        // Verify scenes.
        std::vector<int> expectedNodes = { 0 };
        REQUIRE(gltf.scenes.size() == 1);
        REQUIRE(std::equal(gltf.scenes[0].nodes.begin(), gltf.scenes[0].nodes.end(), expectedNodes.begin()));

        // Verify nodes.
        REQUIRE(gltf.nodes.size() == 1);
        REQUIRE(gltf.nodes[0].children.size() == 0);
    }


    SECTION("Test combining multiple glTFs")
    {
        // Create source glTFs.
        tinygltf::Model model1 = createGltf(triangleMesh, nullptr, nullptr);
        tinygltf::Model model2 = createGltf(triangleMesh, nullptr, nullptr);
        std::vector<tinygltf::Model> sourceGltfs = { model1, model2 };

        // Create target glTF.
        tinygltf::Model gltf;
        gltf.asset.version = "2.0";
        tinygltf::Scene scene;
        scene.nodes = { 0 };
        gltf.scenes.emplace_back(scene);
        tinygltf::Buffer buffer;
        gltf.buffers.emplace_back(buffer);
        tinygltf::Node rootNode;
        rootNode.matrix = {1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
        gltf.nodes.emplace_back(rootNode);

        combineGltfs(&gltf, sourceGltfs);

        // Verify scenes.
        std::vector<int> expectedNodes = { 0 };
        REQUIRE(gltf.scenes.size() == 1);
        REQUIRE(std::equal(gltf.scenes[0].nodes.begin(), gltf.scenes[0].nodes.end(), expectedNodes.begin()));

        // Verify nodes.
        std::vector<int> expectedChildren = { 1, 2 };
        REQUIRE(gltf.nodes.size() == 3);
        REQUIRE(gltf.nodes[0].children.size() == 2);
        REQUIRE(std::equal(gltf.nodes[0].children.begin(), gltf.nodes[0].children.end(), expectedChildren.begin()));
        for (size_t i = 1; i < gltf.nodes.size(); i++) {
            auto node = gltf.nodes[i];
            
            REQUIRE(node.mesh > -1);
            
            // Verify meshes.
            auto mesh = gltf.meshes[node.mesh];
            REQUIRE(mesh.primitives.size() == 1);
            
            // Verify primitives.
            auto primitive = mesh.primitives[0];
            REQUIRE(primitive.attributes["POSITION"] > -1);
            REQUIRE(primitive.attributes["NORMAL"] > -1);

            // Verify accessors.
            auto positionAccessor = gltf.accessors[primitive.attributes["POSITION"]];
            REQUIRE(positionAccessor.bufferView > -1);
        
            auto normalAccessor = gltf.accessors[primitive.attributes["NORMAL"]];
            REQUIRE(normalAccessor.bufferView > -1);

            // Verify accessor data.
            auto positionBufferView = gltf.bufferViews[positionAccessor.bufferView];
            size_t positionAccessorStartOffset = positionAccessor.byteOffset + positionBufferView.byteOffset;
            size_t positionAccessorByteLength = positionAccessor.type * positionAccessor.componentType * positionAccessor.count;
            std::vector<uint8_t> positionData(gltf.buffers[0].data[positionAccessorStartOffset], gltf.buffers[0].data[positionAccessorStartOffset + positionAccessorByteLength]);

            auto normalBufferView = gltf.bufferViews[normalAccessor.bufferView];
            size_t normalAccessorStartOffset = normalAccessor.byteOffset + normalBufferView.byteOffset;
            size_t normalAccessorByteLength = normalAccessor.type * normalAccessor.componentType * normalAccessor.count;
            std::vector<uint8_t> normalData(gltf.buffers[0].data[normalAccessorStartOffset], gltf.buffers[0].data[normalAccessorStartOffset + normalAccessorByteLength]);

            auto sourceGltf = sourceGltfs[i - 1];
            auto sourcePositionAccessor = sourceGltf.accessors[sourceGltf.meshes[0].primitives[0].attributes["POSITION"]];
            auto sourcePositionBufferView = sourceGltf.bufferViews[sourcePositionAccessor.bufferView];
            size_t sourcePositionAccessorStartOffset = sourcePositionAccessor.byteOffset + sourcePositionBufferView.byteOffset;
            size_t sourcePositionAccessorByteLength = sourcePositionAccessor.type * sourcePositionAccessor.componentType * sourcePositionAccessor.count;
            std::vector<uint8_t> sourcePositionData(sourceGltf.buffers[0].data[sourcePositionAccessorStartOffset], sourceGltf.buffers[0].data[sourcePositionAccessorStartOffset + sourcePositionAccessorByteLength]);

            auto sourceNormalAccessor = sourceGltf.accessors[sourceGltf.meshes[0].primitives[0].attributes["NORMAL"]];
            auto sourceNormalBufferView = sourceGltf.bufferViews[sourceNormalAccessor.bufferView];
            size_t sourceNormalAccessorStartOffset = sourceNormalAccessor.byteOffset + sourceNormalBufferView.byteOffset;
            size_t sourceNormalAccessorByteLength = sourceNormalAccessor.type * sourceNormalAccessor.componentType * sourceNormalAccessor.count;
            std::vector<uint8_t> sourceNormalData(sourceGltf.buffers[0].data[sourceNormalAccessorStartOffset], sourceGltf.buffers[0].data[sourceNormalAccessorStartOffset + sourceNormalAccessorByteLength]);

            REQUIRE(std::equal(
                positionData.begin(),
                positionData.end(),
                sourcePositionData.begin()
            ));

            REQUIRE(std::equal(
                normalData.begin(),
                normalData.end(),
                sourceNormalData.begin()
            ));
       
        }
    }
}