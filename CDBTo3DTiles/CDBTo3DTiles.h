#pragma once

#include "BoundRegion.h"
#include "CDB.h"
#include "boost/filesystem.hpp"
#include "nlohmann/json.hpp"
#include "tiny_gltf.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

class CDBTo3DTiles
{
    struct MaterialTexturePair
    {
        MaterialTexturePair(const Material &material, const Texture &texture)
            : material{material}
            , texture{texture}
            , materialGltfIndex{-1}
        {}

        Material material;
        Texture texture;
        mutable std::vector<glm::vec3> positionsRTC;
        mutable std::vector<glm::vec3> normals;
        mutable std::vector<glm::vec2> uvs;
        mutable AABB boundBox;
        mutable int materialGltfIndex;
    };

    struct Tile
    {
        Tile(int x,
             int y,
             int level,
             const BoundRegion &region,
             const std::string &contentUri)
            : x{x}
            , y{y}
            , level{level}
            , region{region}
            , contentUri{contentUri}
        {}

        int x;
        int y;
        int level;
        BoundRegion region;
        std::string contentUri;
        std::vector<int> children;
    };

    struct MaterialTexturePairHash
    {
        size_t operator()(const MaterialTexturePair &key) const;
    };

    struct MaterialTexturePairEqual
    {
        bool operator()(const MaterialTexturePair &lhs, const MaterialTexturePair &rhs) const noexcept;
    };

    struct CDBTileHash
    {
        size_t operator()(const CDBTile &tile) const;
    };

    struct CDBTileEqual
    {
        bool operator()(const CDBTile &lhs, const CDBTile &rhs) const noexcept;
    };

    struct CDBGeoCellHash
    {
        size_t operator()(const CDBGeoCell &geoCell) const;
    };

    struct CDBGeoCellEqual
    {
        bool operator()(const CDBGeoCell &lhs, const CDBGeoCell &rhs) const noexcept;
    };

    struct B3dmHeader
    {
        char magic[4];
        uint32_t version;
        uint32_t byteLength;
        uint32_t featureTableJsonByteLength;
        uint32_t featureTableBinByteLength;
        uint32_t batchTableJsonByteLength;
        uint32_t batchTableBinByteLength;
    };

public:
    void Convert(const boost::filesystem::path &CDBPath, const boost::filesystem::path &output);

private:
    static const boost::filesystem::path TERRAIN_PATH;
    static const boost::filesystem::path IMAGERY_PATH;
    static const boost::filesystem::path GSMODEL_PATH;
    static const boost::filesystem::path GSMODEL_TEXTURE_PATH;

    static glm::ivec2 GetQuadtreeRelativeChild(const Tile &tile, const Tile &root);

    static void InsertTileToTileset(const CDBGeoCell &geoCell,
                                    const Tile &insert,
                                    int root,
                                    std::vector<Tile> &tiles);

    static void ConvertTilesetToJson(const Tile &tile,
                                     float geometricError,
                                     const std::vector<Tile> &tiles,
                                     nlohmann::json &json);

    void ConvertTerrain(const CDBTerrain &terrain, const boost::filesystem::path &outputDirectory);

    void ConvertImagery(CDBImagery &imagery, const boost::filesystem::path &outputDirectory);

    void ConvertGSModel(const CDBGSModel &model, const boost::filesystem::path &output);

    void SaveGeoCellConversions(const CDBGeoCell &geoCell, const boost::filesystem::path &outputDirectory);

    void CreateBufferAndAccessor(tinygltf::Model &terrainModel,
                                 void *destBuffer,
                                 const void *sourceBuffer,
                                 size_t bufferIndex,
                                 size_t bufferViewOffset,
                                 size_t bufferViewLength,
                                 int bufferViewTarget,
                                 size_t accessorComponentCount,
                                 int accessorComponentType,
                                 int accessorType);

    tinygltf::Model CreateTerrainGltf(const CDBTerrain &terrain);

    void WriteToB3DM(tinygltf::Model *gltf, const boost::filesystem::path &output);

    tinygltf::Model CreateGSModelGltf(const CDBGSModel &model,
                                      const boost::filesystem::path &textureDirectory);

    void AddCDBMaterialToGltf(
        const boost::filesystem::path &textureDirectory,
        std::unordered_set<MaterialTexturePair, MaterialTexturePairHash, MaterialTexturePairEqual>
            &materialTextures,
        std::unordered_map<std::string, int> &modelTextures,
        tinygltf::Model &GSModelGltf);

    void AddCDBSceneToGltf(
        std::unordered_set<MaterialTexturePair, MaterialTexturePairHash, MaterialTexturePairEqual>
            &materialTextures,
        tinygltf::Model &GSModelGltf);

    std::unordered_set<std::string> _writtenTextures;

    std::unordered_map<CDBTile, std::string, CDBTileHash, CDBTileEqual> _imagery;
    std::unordered_map<CDBGeoCell, int, CDBGeoCellHash, CDBGeoCellEqual> _terrainRootTiles;
    std::unordered_map<CDBGeoCell, std::vector<Tile>, CDBGeoCellHash, CDBGeoCellEqual> _terrainTiles;

    std::unordered_map<CDBGeoCell, int, CDBGeoCellHash, CDBGeoCellEqual> _modelRootTiles;
    std::unordered_map<CDBGeoCell, std::vector<Tile>, CDBGeoCellHash, CDBGeoCellEqual> _modelTiles;
};
