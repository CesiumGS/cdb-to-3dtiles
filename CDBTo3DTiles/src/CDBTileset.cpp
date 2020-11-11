#include "CDBTileset.h"
#include "CDB.h"
#include "glm/glm.hpp"

namespace CDBTo3DTiles {

static glm::ivec2 getQuadtreeRelativeChild(const CDBTile &tile, const CDBTile &root);

CDBTileset::CDBTileset()
    : m_rootLevel{-10}
    , m_rootUREF{0}
    , m_rootRREF{0}
{}

CDBTileset::CDBTileset(int rootLevel, int rootUREF, int rootRREF)
    : m_rootLevel{rootLevel}
    , m_rootUREF{rootUREF}
    , m_rootRREF{rootRREF}
{}

const CDBTile *CDBTileset::getRoot() const
{
    if (m_tiles.empty()) {
        return nullptr;
    }

    return m_tiles.front().get();
}

CDBTile *CDBTileset::insertTile(const CDBTile &tile)
{
    if (tile.getLevel() < m_rootLevel) {
        return nullptr;
    }

    if ((tile.getLevel() < 0 || tile.getLevel() == m_rootLevel)
        && (tile.getUREF() != m_rootUREF || tile.getRREF() != m_rootRREF)) {
        return nullptr;
    }

    if (tile.getLevel() > 0 && tile.getLevel() > m_rootLevel) {
        int parentUREF = tile.getUREF() >> (tile.getLevel() - m_rootLevel);
        int parentRREF = tile.getRREF() >> (tile.getLevel() - m_rootLevel);
        if (parentUREF != m_rootUREF || parentRREF != m_rootRREF) {
            return nullptr;
        }
    }

    if (m_tiles.empty()) {
        auto cdbRoot = std::make_unique<CDBTile>(tile.getGeoCell(),
                                                 tile.getDataset(),
                                                 tile.getCS_1(),
                                                 tile.getCS_2(),
                                                 m_rootLevel,
                                                 m_rootUREF,
                                                 m_rootRREF);

        m_tiles.emplace_back(std::move(cdbRoot));
    }

    return insertTileRecursively(tile, m_tiles.front().get());
}

const CDBTile *CDBTileset::getFitTile(Core::Cartographic cartographic) const
{
    const CDBTile *root = getRoot();
    if (!root) {
        return nullptr;
    }

    const auto &rectangle = root->getBoundRegion().getRectangle();
    if (!rectangle.contains(cartographic)) {
        return nullptr;
    }

    return getFitTile(root, cartographic);
}

glm::ivec2 getQuadtreeRelativeChild(const CDBTile &tile, const CDBTile &root)
{
    double powerOf2 = glm::pow(2, tile.getLevel() - root.getLevel() - 1);
    int UREF = static_cast<int>(tile.getUREF() / powerOf2 - root.getUREF() * 2);
    int RREF = static_cast<int>(tile.getRREF() / powerOf2 - root.getRREF() * 2);
    return {RREF, UREF};
}

const CDBTile *CDBTileset::getFitTile(const CDBTile *root, Core::Cartographic cartographic) const
{
    for (auto child : root->getChildren()) {
        if (child) {
            const auto &childRectangle = child->getBoundRegion().getRectangle();
            if (childRectangle.contains(cartographic)) {
                return getFitTile(child, cartographic);
            }
        }
    }

    return root;
}

CDBTile *CDBTileset::insertTileRecursively(const CDBTile &insert, CDBTile *subTree)
{
    // we are at the right level here. Just copy the data over
    if (insert.getLevel() == subTree->getLevel() && insert.getUREF() == subTree->getUREF()
        && insert.getRREF() == subTree->getRREF()) {
        auto customContentURI = insert.getCustomContentURI();
        if (customContentURI) {
            subTree->setCustomContentURI(*customContentURI);
        }

        return subTree;
    }

    // root has only one child if its level < 0
    if (subTree->getLevel() < 0) {
        if (subTree->getChildren().empty()) {
            int childUREF = subTree->getUREF();
            int childRREF = subTree->getRREF();
            int childLevel = subTree->getLevel() + 1;
            auto &children = subTree->getChildren();
            const CDBGeoCell &geoCell = insert.getGeoCell();
            auto child = std::make_unique<CDBTile>(geoCell,
                                                   insert.getDataset(),
                                                   insert.getCS_1(),
                                                   insert.getCS_2(),
                                                   childLevel,
                                                   childUREF,
                                                   childRREF);
            children.emplace_back(child.get());
            m_tiles.emplace_back(std::move(child));
        }

        return insertTileRecursively(insert, subTree->getChildren().back());
    }

    // For positive level, root is a quadtree.
    // So we determine which child we will insert the tile to
    auto &children = subTree->getChildren();
    if (children.size() < 4) {
        children.resize(4, nullptr);
    }

    glm::ivec2 relativeChild = getQuadtreeRelativeChild(insert, *subTree);
    size_t childIdx = relativeChild.y * 2 + relativeChild.x;
    if (children[childIdx] == nullptr) {
        int childRREF = relativeChild.x + 2 * subTree->getRREF();
        int childUREF = relativeChild.y + 2 * subTree->getUREF();
        int childLevel = subTree->getLevel() + 1;
        const CDBGeoCell &geoCell = insert.getGeoCell();
        auto child = std::make_unique<CDBTile>(geoCell,
                                               insert.getDataset(),
                                               insert.getCS_1(),
                                               insert.getCS_2(),
                                               childLevel,
                                               childUREF,
                                               childRREF);

        children[childIdx] = child.get();
        m_tiles.emplace_back(std::move(child));
    }

    return insertTileRecursively(insert, children[childIdx]);
}
} // namespace CDBTo3DTiles
