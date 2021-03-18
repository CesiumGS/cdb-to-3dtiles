#include "CDBTilesetBuilder.h"
#include "catch2/catch.hpp"
#include "morton.h"
#include "Config.h"

using namespace CDBTo3DTiles;
using namespace Core;

TEST_CASE("Test setting parent bits recursively.", "[CDBTilesetBuilder]")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    std::filesystem::path output = "CombineTilesets";
    std::unique_ptr<CDBTilesetBuilder> builder = std::make_unique<CDBTilesetBuilder>(input, output);

    SECTION("Test that parents of level 6 tile are set within one subtree.")
    {
        int subtreeLevels = 7;
        builder->subtreeLevels = subtreeLevels;
        uint64_t subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
        uint64_t childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
        uint64_t availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
        uint64_t childSubtreeAvailabilityByteLength = static_cast<int>(ceil(static_cast<double>(childSubtreeCount) / 8.0));
        builder->nodeAvailabilityByteLengthWithPadding = availabilityByteLength;
        builder->childSubtreeAvailabilityByteLengthWithPadding = childSubtreeAvailabilityByteLength;


        std::map<std::string, subtreeAvailability> tileAndChildAvailabilities;
        int level = 6, x = 47, y = 61;
        builder->setParentBitsRecursively(tileAndChildAvailabilities, level, x, y, 0, 0, 0);

        while(level != 0)
        {
            level -= 1;
            x /= 2;
            y /= 2;

            int64_t mortonIndex = libmorton::morton2D_64_encode(x, y);
            int levelWithinSubtree = level;
            const uint64_t nodeCountUpToThisLevel = ((1 << (2 * levelWithinSubtree)) - 1) / 3;

            const uint64_t index = nodeCountUpToThisLevel + mortonIndex;
            uint64_t byte = index / 8;
            uint64_t bit = index % 8;
            // Check the bit is set
            int mask  = (1 << bit);
            REQUIRE((tileAndChildAvailabilities["0_0_0"].nodeBuffer[byte] & mask) >> bit == 1);
        }
    }

    SECTION("Test that parents of level 6 tile are set, multi subtree.")
    {
        int subtreeLevels = 6;
        builder->subtreeLevels = subtreeLevels;
        uint64_t subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
        uint64_t childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
        uint64_t availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
        uint64_t childSubtreeAvailabilityByteLength = static_cast<int>(ceil(static_cast<double>(childSubtreeCount) / 8.0));
        builder->nodeAvailabilityByteLengthWithPadding = availabilityByteLength;
        builder->childSubtreeAvailabilityByteLengthWithPadding = childSubtreeAvailabilityByteLength;


        std::map<std::string, subtreeAvailability> tileAndChildAvailabilities;
        int level = 6, x = 47, y = 61;
        builder->setParentBitsRecursively(tileAndChildAvailabilities, level, x, y, 0, 0, 0);
        
        uint64_t childIndex = libmorton::morton2D_64_encode(x, y);
        uint64_t childByte = childIndex / 8;
        uint64_t childBit = childIndex % 8;
        int mask  = (1 << childBit);
        REQUIRE((tileAndChildAvailabilities["0_0_0"].childBuffer[childByte] & mask) >> childBit == 1);
        while(level != 0)
        {
            level -= 1;
            x /= 2;
            y /= 2;

            int64_t mortonIndex = libmorton::morton2D_64_encode(x, y);
            int levelWithinSubtree = level;
            const uint64_t nodeCountUpToThisLevel = ((1 << (2 * levelWithinSubtree)) - 1) / 3;

            const uint64_t index = nodeCountUpToThisLevel + mortonIndex;
            uint64_t byte = index / 8;
            uint64_t bit = index % 8;
            mask = (1 << bit);
            REQUIRE((tileAndChildAvailabilities["0_0_0"].nodeBuffer[byte] & mask) >> bit == 1);
        }
    }
}