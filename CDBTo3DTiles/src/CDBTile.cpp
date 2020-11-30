#include "CDBTile.h"
#include "CDB.h"

namespace CDBTo3DTiles {


constexpr int MAX_POSITIVE_LOD_WIDTH[] = {
    1 << 0,  1 << 1,  1 << 2,  1 << 3,  1 << 4,  1 << 5,  1 << 6,  1 << 7,
    1 << 8,  1 << 9,  1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15,
    1 << 16, 1 << 17, 1 << 18, 1 << 19, 1 << 20, 1 << 21, 1 << 22, 1 << 23,
};

CDBTile::CDBTile(CDBGeoCell geoCell, CDBDataset dataset, int CS_1, int CS_2, int level, int UREF, int RREF)
{
    if (level < -10 || level > MAX_LEVEL) {
        throw std::invalid_argument("Level is out of range. Minimum is -10 and maximum is 23");
    }

    if (level < 0 && ((UREF != 0) || (RREF != 0))) {
        throw std::invalid_argument("Negative level tile only accept UREF and RREF value 0");
    }

    if (level >= 0 && level <= 23) {
        int maxWidth = MAX_POSITIVE_LOD_WIDTH[level];
        if (UREF < 0 || UREF > maxWidth - 1) {
            throw std::invalid_argument("Positive level tile has UREF out of range");
        }

        if (RREF < 0 || RREF > maxWidth - 1) {
            throw std::invalid_argument("Positive level tile has RREF out of range");
        }
    }

    m_geoCell = geoCell;
    m_dataset = dataset;
    m_CS_1 = CS_1;
    m_CS_2 = CS_2;
    m_level = level;
    m_UREF = UREF;
    m_RREF = RREF;
    m_region = calcBoundRegion(*m_geoCell, m_level, m_UREF, m_RREF);
    m_path = convertToPath();
}

CDBTile::CDBTile(const CDBTile &other)
    : m_customContentURI{other.m_customContentURI}
    , m_region{other.m_region}
    , m_path{other.m_path}
    , m_geoCell{other.m_geoCell}
    , m_dataset{other.m_dataset}
    , m_CS_1{other.m_CS_1}
    , m_CS_2{other.m_CS_2}
    , m_level{other.m_level}
    , m_UREF{other.m_UREF}
    , m_RREF{other.m_RREF}
{}

CDBTile &CDBTile::operator=(const CDBTile &other)
{
    if (&other != this) {
        m_customContentURI = other.m_customContentURI;
        m_region = other.m_region;
        m_path = other.m_path;
        m_geoCell = other.m_geoCell;
        m_dataset = other.m_dataset;
        m_CS_1 = other.m_CS_1;
        m_CS_2 = other.m_CS_2;
        m_level = other.m_level;
        m_UREF = other.m_UREF;
        m_RREF = other.m_RREF;
    }

    return *this;
}

const std::filesystem::path *CDBTile::getCustomContentURI() const noexcept
{
    return m_customContentURI ? &*m_customContentURI : nullptr;
}

void CDBTile::setCustomContentURI(const std::filesystem::path &customContentURI) noexcept
{
    m_customContentURI = customContentURI;
}

std::string CDBTile::retrieveGeoCellDatasetFromTileName(const CDBTile &tile)
{
    const auto &geoCell = tile.getGeoCell();
    std::string latitudeDir = geoCell.getLatitudeDirectoryName();
    std::string longitudeDir = geoCell.getLongitudeDirectoryName();

    auto dataset = tile.getDataset();
    std::string datasetDir = getCDBDatasetDirectoryName(dataset);
    std::string datasetInTileFilename = "D" + toStringWithZeroPadding(3, static_cast<unsigned>(dataset));

    std::string CS_1 = tile.getCS_1Name();
    std::string CS_2 = tile.getCS_2Name();

    std::string geoCellDatasetName = latitudeDir + longitudeDir + "_" + datasetInTileFilename + "_" + CS_1
                                     + "_" + CS_2;

    return geoCellDatasetName;
}

std::optional<CDBTile> CDBTile::createParentTile(const CDBTile &tile)
{
    if (tile.getLevel() == -10) {
        return std::nullopt;
    }

    int parentLevel = tile.getLevel() - 1;
    if (parentLevel < 0) {
        return CDBTile(tile.getGeoCell(), tile.getDataset(), tile.getCS_1(), tile.getCS_2(), parentLevel, 0, 0);
    } else {
        int UREF = tile.getUREF() / 2;
        int RREF = tile.getRREF() / 2;
        return CDBTile(tile.getGeoCell(),
                       tile.getDataset(),
                       tile.getCS_1(),
                       tile.getCS_2(),
                       parentLevel,
                       UREF,
                       RREF);
    }
}

CDBTile CDBTile::createChildForNegativeLOD(const CDBTile &tile)
{
    if (tile.getLevel() >= 0) {
        throw std::invalid_argument("getChildForNegativeLOD only valid for negative level");
    }

    int childLevel = tile.getLevel() + 1;
    int childUREF = tile.getUREF();
    int childRREF = tile.getRREF();

    return CDBTile(tile.getGeoCell(),
                   tile.getDataset(),
                   tile.getCS_1(),
                   tile.getCS_2(),
                   childLevel,
                   childUREF,
                   childRREF);
}

CDBTile CDBTile::createNorthWestForPositiveLOD(const CDBTile &tile)
{
    if (tile.getLevel() < 0) {
        throw std::invalid_argument("North West child only exists for positive level");
    }

    int childLevel = tile.getLevel() + 1;
    int childUREF = 1 + 2 * tile.getUREF();
    int childRREF = 2 * tile.getRREF();

    return CDBTile(tile.getGeoCell(),
                   tile.getDataset(),
                   tile.getCS_1(),
                   tile.getCS_2(),
                   childLevel,
                   childUREF,
                   childRREF);
}

CDBTile CDBTile::createNorthEastForPositiveLOD(const CDBTile &tile)
{
    if (tile.getLevel() < 0) {
        throw std::invalid_argument("North East child only exists for positive level");
    }

    int childLevel = tile.getLevel() + 1;
    int childUREF = 1 + 2 * tile.getUREF();
    int childRREF = 1 + 2 * tile.getRREF();

    return CDBTile(tile.getGeoCell(),
                   tile.getDataset(),
                   tile.getCS_1(),
                   tile.getCS_2(),
                   childLevel,
                   childUREF,
                   childRREF);
}

CDBTile CDBTile::createSouthWestForPositiveLOD(const CDBTile &tile)
{
    if (tile.getLevel() < 0) {
        throw std::invalid_argument("South West child only exists for positive level");
    }

    int childLevel = tile.getLevel() + 1;
    int childUREF = 2 * tile.getUREF();
    int childRREF = 2 * tile.getRREF();

    return CDBTile(tile.getGeoCell(),
                   tile.getDataset(),
                   tile.getCS_1(),
                   tile.getCS_2(),
                   childLevel,
                   childUREF,
                   childRREF);
}

CDBTile CDBTile::createSouthEastForPositiveLOD(const CDBTile &tile)
{
    if (tile.getLevel() < 0) {
        throw std::invalid_argument("South East child only exists for positive level");
    }

    int childLevel = tile.getLevel() + 1;
    int childUREF = 2 * tile.getUREF();
    int childRREF = 1 + 2 * tile.getRREF();

    return CDBTile(tile.getGeoCell(),
                   tile.getDataset(),
                   tile.getCS_1(),
                   tile.getCS_2(),
                   childLevel,
                   childUREF,
                   childRREF);
}

std::optional<CDBTile> CDBTile::createFromFile(const std::string &filename)
{
    char NS, WE;
    int geoCellLatitude, geoCellLongitude;
    int dataset, CS_1, CS_2;
    int level, UREF, RREF;
    char level_UREF_RREF[100] = "";
    int result = sscanf(filename.c_str(),
                        "%c%d%c%d_D%d_S%d_T%d_%s",
                        &NS,
                        &geoCellLatitude,
                        &WE,
                        &geoCellLongitude,
                        &dataset,
                        &CS_1,
                        &CS_2,
                        level_UREF_RREF);
    if (result < 8) {
        return std::nullopt;
    }

    if (level_UREF_RREF[0] == 'L' && level_UREF_RREF[1] == 'C') {
        result = sscanf(level_UREF_RREF, "LC%d_U%d_R%d", &level, &UREF, &RREF);
        if (result < 3) {
            return std::nullopt;
        }

        level = -level;
    } else {
        result = sscanf(level_UREF_RREF, "L%d_U%d_R%d", &level, &UREF, &RREF);
        if (result < 3) {
            return std::nullopt;
        }
    }

    if ((NS != 'S' && NS != 'N') || (WE != 'W' && WE != 'E')) {
        return std::nullopt;
    }

    if (!isValidDataset(dataset)) {
        return std::nullopt;
    }

    if (level < -10 || level > MAX_LEVEL) {
        return std::nullopt;
    }

    if (level < 0 && ((UREF != 0) || (RREF != 0))) {
        return std::nullopt;
    }

    if (level >= 0) {
        int maxWidth = MAX_POSITIVE_LOD_WIDTH[level];
        if (UREF < 0 || UREF > maxWidth - 1) {
            return std::nullopt;
        }

        if (RREF < 0 || RREF > maxWidth - 1) {
            return std::nullopt;
        }
    }

    geoCellLatitude = NS == 'S' ? -geoCellLatitude : geoCellLatitude;
    geoCellLongitude = WE == 'W' ? -geoCellLongitude : geoCellLongitude;
    if (geoCellLatitude < -90 || geoCellLatitude >= 90 || geoCellLongitude < -180 || geoCellLongitude > 180) {
        return std::nullopt;
    }

    return CDBTile(CDBGeoCell(geoCellLatitude, geoCellLongitude),
                   static_cast<CDBDataset>(dataset),
                   CS_1,
                   CS_2,
                   level,
                   UREF,
                   RREF);
}

std::string CDBTile::getUREFDirectoryName() const noexcept
{
    return "U" + std::to_string(m_UREF);
}

std::string CDBTile::getRREFName() const noexcept
{
    return "R" + std::to_string(m_RREF);
}

std::string CDBTile::getCS_1Name() const noexcept
{
    return "S" + toStringWithZeroPadding(3, m_CS_1);
}

std::string CDBTile::getCS_2Name() const noexcept
{
    return "T" + toStringWithZeroPadding(3, m_CS_2);
}

std::string CDBTile::getLevelInFilename() const noexcept
{
    if (m_level < 0) {
        return "LC" + toStringWithZeroPadding(2, glm::abs(m_level));
    }

    return "L" + toStringWithZeroPadding(2, m_level);
}

std::string CDBTile::getLevelDirectoryName() const noexcept
{
    if (m_level < 0) {
        return "LC";
    }

    return "L" + toStringWithZeroPadding(2, m_level);
}

std::filesystem::path CDBTile::convertToPath() const noexcept
{
    std::string latitudeDir = m_geoCell->getLatitudeDirectoryName();
    std::string longitudeDir = m_geoCell->getLongitudeDirectoryName();

    std::string datasetDir = getCDBDatasetDirectoryName(m_dataset);
    std::string datasetInTileFilename = "D" + toStringWithZeroPadding(3, static_cast<unsigned>(m_dataset));

    std::string levelDir = getLevelDirectoryName();
    std::string levelInTileFilename = getLevelInFilename();

    std::string UREFDir = getUREFDirectoryName();
    std::string RREFName = getRREFName();

    std::string CS_1 = getCS_1Name();
    std::string CS_2 = getCS_2Name();

    std::string tileFilename = latitudeDir + longitudeDir + "_" + datasetInTileFilename + "_" + CS_1 + "_"
                               + CS_2 + "_" + levelInTileFilename + "_" + UREFDir + "_" + RREFName;

    return CDB::TILES / latitudeDir / longitudeDir / datasetDir / levelDir / UREFDir / tileFilename;
}

Core::BoundingRegion CDBTile::calcBoundRegion(const CDBGeoCell &geoCell, int level, int UREF, int RREF) noexcept
{
    double distLOD = 1.0;
    if (level > 0) {
        distLOD = glm::pow(2.0, -level);
    }
    double longUnitLOD = distLOD * geoCell.getLongitudeExtentInDegree();
    double latUnitLOD = distLOD * geoCell.getLatitudeExtentInDegree();

    double minLongitudeDegree = geoCell.getLongitude() + RREF * longUnitLOD;
    double minLatitudeDegree = geoCell.getLatitude() + UREF * latUnitLOD;
    Core::Cartographic min(glm::radians(minLongitudeDegree), glm::radians(minLatitudeDegree), 0.0);
    Core::Cartographic max(glm::radians(minLongitudeDegree + longUnitLOD),
                           glm::radians(minLatitudeDegree + latUnitLOD),
                           0.0);

    Core::GlobeRectangle rectangle(min.longitude, min.latitude, max.longitude, max.latitude);
    return Core::BoundingRegion(rectangle, 0.0, 0.0);
}

bool operator==(const CDBTile &lhs, const CDBTile &rhs) noexcept
{
    return lhs.m_geoCell == rhs.m_geoCell && lhs.m_level == rhs.m_level && lhs.m_UREF == rhs.m_UREF
           && lhs.m_RREF == rhs.m_RREF && lhs.m_CS_1 == rhs.m_CS_1 && lhs.m_CS_2 == rhs.m_CS_2
           && lhs.m_dataset == rhs.m_dataset;
}

} // namespace CDBTo3DTiles
