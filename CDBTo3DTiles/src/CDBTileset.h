#pragma once

#include "CDBTile.h"
#include "Cartographic.h"
#include <memory>
#include <vector>

namespace CDBTo3DTiles {
class CDBTile;

class CDBTileset
{
public:
    CDBTileset();

    CDBTileset(int rootLevel, int rootUREF, int rootRREF);

    const CDBTile *getRoot() const;

    CDBTile *insertTile(const CDBTile &tile);

    const CDBTile *getFitTile(Core::Cartographic cartographic) const;

private:
    const CDBTile *getFitTile(const CDBTile *root, Core::Cartographic cartographic) const;

    CDBTile *insertTileRecursively(const CDBTile &insert, CDBTile *subTree);

    int m_rootLevel;
    int m_rootUREF;
    int m_rootRREF;
    std::vector<std::unique_ptr<CDBTile>> m_tiles;
};

} // namespace CDBTo3DTiles
