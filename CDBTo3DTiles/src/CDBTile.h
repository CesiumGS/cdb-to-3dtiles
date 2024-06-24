#pragma once

#include "BoundingRegion.h"
#include "CDBDataset.h"
#include "CDBGeoCell.h"
#include <optional>

namespace CDBTo3DTiles {
class CDBTile
{
public:
    CDBTile(CDBGeoCell geoCell, CDBDataset dataset, int CS_1, int CS_2, int level, int UREF, int RREF);

    CDBTile(const CDBTile &);

    CDBTile(CDBTile &&) noexcept = default;

    CDBTile &operator=(const CDBTile &);

    CDBTile &operator=(CDBTile &&) noexcept = default;

    inline const std::filesystem::path &getRelativePath() const noexcept { return m_path; }

    inline const Core::BoundingRegion &getBoundRegion() const noexcept { return *m_region; }

    inline const CDBGeoCell &getGeoCell() const noexcept { return *m_geoCell; }

    inline CDBDataset getDataset() const noexcept { return m_dataset; }

    inline int getCS_1() const noexcept { return m_CS_1; }

    inline int getCS_2() const noexcept { return m_CS_2; }

    inline int getLevel() const noexcept { return m_level; }

    inline int getUREF() const noexcept { return m_UREF; }

    inline int getRREF() const noexcept { return m_RREF; }

    inline const std::vector<CDBTile *> &getChildren() const noexcept { return m_children; }

    inline std::vector<CDBTile *> &getChildren() noexcept { return m_children; }

    const std::filesystem::path *getCustomContentURI() const noexcept;

    void setCustomContentURI(const std::filesystem::path &customContentURI) noexcept;

    static std::string retrieveGeoCellDatasetFromTileName(const CDBTile &tile);

    static std::optional<CDBTile> createParentTile(const CDBTile &tile);

    static CDBTile createChildForNegativeLOD(const CDBTile &tile);

    static CDBTile createNorthWestForPositiveLOD(const CDBTile &tile);

    static CDBTile createNorthEastForPositiveLOD(const CDBTile &tile);

    static CDBTile createSouthWestForPositiveLOD(const CDBTile &tile);

    static CDBTile createSouthEastForPositiveLOD(const CDBTile &tile);

    static std::optional<CDBTile> createFromFile(const std::string &filename);

    static Core::BoundingRegion calcBoundRegion(const CDBGeoCell &geoCell,
                                                int level,
                                                int UREF,
                                                int RREF) noexcept;

private:
    static constexpr int MAX_LEVEL = 23;

    friend bool operator==(const CDBTile &lhs, const CDBTile &rhs) noexcept;

    std::string getUREFDirectoryName() const noexcept;

    std::string getLevelInFilename() const noexcept;

    std::string getRREFName() const noexcept;

    std::string getCS_1Name() const noexcept;

    std::string getCS_2Name() const noexcept;

    std::string getLevelDirectoryName() const noexcept;

    std::filesystem::path convertToPath() const noexcept;

    std::vector<CDBTile *> m_children;
    std::optional<std::filesystem::path> m_customContentURI;
    std::optional<Core::BoundingRegion> m_region;
    std::filesystem::path m_path;
    std::optional<CDBGeoCell> m_geoCell;
    CDBDataset m_dataset;
    int m_CS_1;
    int m_CS_2;
    int m_level;
    int m_UREF;
    int m_RREF;
};
} // namespace CDBTo3DTiles

namespace std {
template<>
struct hash<CDBTo3DTiles::CDBTile>
{
    size_t operator()(CDBTo3DTiles::CDBTile const &tile) const noexcept
    {
        size_t seed = 0;
        CDBTo3DTiles::hashCombine(seed, tile.getGeoCell());
        CDBTo3DTiles::hashCombine(seed, tile.getLevel());
        CDBTo3DTiles::hashCombine(seed, tile.getUREF());
        CDBTo3DTiles::hashCombine(seed, tile.getRREF());
        CDBTo3DTiles::hashCombine(seed, tile.getDataset());
        CDBTo3DTiles::hashCombine(seed, tile.getCS_1());
        CDBTo3DTiles::hashCombine(seed, tile.getCS_2());

        return seed;
    }
};
} // namespace std
