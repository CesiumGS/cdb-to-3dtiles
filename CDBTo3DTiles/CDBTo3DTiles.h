#pragma once

#include "BoundRegion.h"
#include "CDB.h"
#include "boost/filesystem.hpp"
#include "nlohmann/json.hpp"
#include "tiny_gltf.h"
#include <unordered_map>
#include <vector>

class CDBTo3DTiles
{
    struct Tile
    {
        Tile(int x,
             int y,
             int level,
             float geometricError,
             const BoundRegion &region,
             const std::string &contentUri)
            : x{x}
            , y{y}
            , level{level}
            , geometricError{geometricError}
            , region{region}
            , contentUri{contentUri}
        {}

        int x;
        int y;
        int level;
        float geometricError;
        BoundRegion region;
        std::string contentUri;
        std::vector<int> children;
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

    void ConvertTerrain(const CDBTerrain &terrain, const boost::filesystem::path &outputDirectory);

    void ConvertImagery(CDBImagery &imagery, const boost::filesystem::path &outputDirectory);

    void SaveGeoCellConversions(const CDBGeoCell &geoCell, const boost::filesystem::path &outputDirectory);

    void CreateTerrainBufferAndAccessor(tinygltf::Model &terrainModel,
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

    void WriteTerrainToB3DM(const CDBTerrain &terrain, const boost::filesystem::path &output);

    static glm::ivec2 GetQuadtreeRelativeChild(const Tile &tile, const Tile &root);

    static void InsertTerrainTileToTileset(const Tile &insert, int root, std::vector<Tile> &tiles);

    static void ConvertTerrainTileToJson(const Tile &tile,
                                         const std::vector<Tile> &tiles,
                                         nlohmann::json &json);

    std::unordered_map<CDBTile, std::string, CDBTileHash, CDBTileEqual> _imagery;
    std::unordered_map<CDBGeoCell, int, CDBGeoCellHash, CDBGeoCellEqual> _terrainRootTiles;
    std::unordered_map<CDBGeoCell, std::vector<Tile>, CDBGeoCellHash, CDBGeoCellEqual> _terrainTiles;
};
