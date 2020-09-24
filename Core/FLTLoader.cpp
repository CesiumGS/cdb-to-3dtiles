#include "FLTLoader.h"
#include "ThirdParty/flt.h"
#include "glm/gtx/hash.hpp"
#include <iostream>
#include <unordered_map>

static void ParseFaceNode(FltFace *face, Mesh &mesh);
static void ParseMaterialsAndTextures(const boost::filesystem::path &parentDirectory,
                                      FltFile *file,
                                      Scene &mesh);
static void ParseMesh(FltNode *node, std::unordered_map<glm::ivec2, Mesh> &materialToMesh);
static void FreeFltFile(FltFile *file);
using FltFilePtr = std::unique_ptr<FltFile, decltype(&FreeFltFile)>;

Scene LoadFLTFile(const boost::filesystem::path &filePath)
{
    FltFilePtr fltFileWrapper = FltFilePtr(fltOpen(filePath.c_str()), FreeFltFile);
    if (!fltFileWrapper) {
        return {};
    }

    auto fltFile = fltFileWrapper.get();
    fltParse(fltFile, 0);

    Scene scene;

    auto header = fltFile->header;
    ParseMaterialsAndTextures(filePath.parent_path(), fltFile, scene);

    std::unordered_map<glm::ivec2, Mesh> materialTextureToMesh;
    ParseMesh(&header->node, materialTextureToMesh);
    for (const auto &mesh : materialTextureToMesh) {
        scene.meshes.emplace_back(mesh.second);
    }

    return scene;
}

void ParseMesh(FltNode *node, std::unordered_map<glm::ivec2, Mesh> &materialTextureToMesh)
{
    if (node->type == FLTRECORD_OBJECT) {
        for (uint32_t i = 0; i < node->numChildren; ++i) {
            if (node->child[i]->type != FLTRECORD_FACE) {
                continue;
            }

            auto face = reinterpret_cast<FltFace *>(node->child[i]);
            auto materialId = face->materialIndex;
            auto textureId = face->texturePatternIndex;
            auto &mesh = materialTextureToMesh[glm::ivec2(materialId, textureId)];
            mesh.material = materialId;
            mesh.texture = textureId;
            ParseFaceNode(face, mesh);
        }

        return;
    }

    for (uint32_t i = 0; i < node->numChildren; ++i) {
        ParseMesh(node->child[i], materialTextureToMesh);
    }
}

void ParseFaceNode(FltFace *face, Mesh &mesh)
{
    auto &node = face->node;
    for (uint32_t i = 0; i < node.numChildren; ++i) {
        if (node.child[i]->type == FLTRECORD_VERTEXLIST) {
            auto vertexList = reinterpret_cast<FltVertexList *>(node.child[i]);
            for (uint32_t j = 0; j < vertexList->numVerts; ++j) {
                auto vertex = vertexList->list[j];
                auto position = glm::vec3(vertex->x, vertex->y, vertex->z);
                mesh.positions.emplace_back(position);
                mesh.boundBox.merge(position);

                if (vertex->localFlags & FVHAS_COLOR) {
                    float red = ((vertex->packedColor & 0xFF000000) >> 24) / 255.0;
                    float green = ((vertex->packedColor & 0x00FF0000) >> 16) / 255.0;
                    float blue = ((vertex->packedColor & 0x0000FF00) >> 8) / 255.0;
                    float alpha = (vertex->packedColor & 0x000000FF) / 255.0;
                    mesh.colors.emplace_back(red, green, blue, alpha);
                }

                if (vertex->localFlags & FVHAS_NORMAL) {
                    mesh.normals.emplace_back(glm::normalize(glm::vec3(vertex->i, vertex->j, vertex->k)));
                }

                if (vertex->localFlags & FVHAS_TEXTURE) {
                    // no idea what's going on here but the origin of rgb format is different with jpg
                    mesh.uvs.emplace_back(glm::vec2(vertex->u, 1.0 - vertex->v));
                }
            }
        }
    }
}

void ParseMaterialsAndTextures(const boost::filesystem::path &parentPath, FltFile *file, Scene &mesh)
{
    auto curr = &file->header->node;
    while (curr) {
        if (curr->type == FLTRECORD_MATERIAL) {
            auto fltMaterial = reinterpret_cast<FltMaterial *>(curr);
            Material material;
            material.name = fltMaterial->ID;
            material.ambient = glm::vec3(fltMaterial->ambientRed,
                                         fltMaterial->ambientGreen,
                                         fltMaterial->ambientBlue);

            material.diffuse = glm::vec3(fltMaterial->diffuseRed,
                                         fltMaterial->diffuseGreen,
                                         fltMaterial->diffuseBlue);

            material.specular = glm::vec3(fltMaterial->specularRed,
                                          fltMaterial->specularGreen,
                                          fltMaterial->specularBlue);

            material.emissive = glm::vec3(fltMaterial->emissiveRed,
                                          fltMaterial->emissiveGreen,
                                          fltMaterial->emissiveBlue);

            material.shininess = fltMaterial->shininess;

            material.alpha = fltMaterial->alpha;

            mesh.materials.emplace_back(material);
        }

        if (curr->type == FLTRECORD_TEXTURE) {
            auto fltTexture = reinterpret_cast<FltTexture *>(curr);
            Texture texture;
            texture.path = boost::filesystem::weakly_canonical(parentPath / std::string(fltTexture->ID));
            mesh.textures.emplace_back(texture);
        }

        curr = curr->next;
    }
}

void FreeFltFile(FltFile *file)
{
    if (file) {
        fltClose(file);
        fltFileFree(file);
    }
}
