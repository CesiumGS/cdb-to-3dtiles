#include "CDBAttributes.h"
#include "Transforms.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ogrsf_frmts.h"

namespace CDBTo3DTiles {

glm::dmat4 calculateModelOrientation(glm::dvec3 worldPosition, double orientation)
{
    glm::dmat4 ENU = Core::Transforms::eastNorthUpToFixedFrame(worldPosition);
    glm::dvec3 up = glm::column(ENU, 2);
    glm::dmat4 rotMat = glm::rotate(glm::dmat4(1.0), -orientation, up) * glm::dmat4(glm::dmat3(ENU));
    return glm::translate(glm::dmat4(1.0), worldPosition) * rotMat;
}

void CDBInstancesAttributes::addInstanceFeature(const OGRFeature &feature)
{
    if (feature.GetFieldCount() == 0) {
        return;
    }

    for (int i = 0; i < feature.GetFieldCount(); ++i) {
        auto fieldDef = feature.GetFieldDefnRef(i);
        if (fieldDef->GetType() == OGRFieldType::OFTInteger) {
            auto &values = m_integerAttribs[fieldDef->GetNameRef()];
            values.emplace_back(feature.GetFieldAsInteger(i));
        } else if (fieldDef->GetType() == OGRFieldType::OFTReal) {
            auto &values = m_doubleAttribs[fieldDef->GetNameRef()];
            values.emplace_back(feature.GetFieldAsDouble(i));
        } else if (fieldDef->GetType() == OGRFieldType::OFTString) {
            if (strcmp(fieldDef->GetNameRef(), "CNAM") == 0) {
                m_CNAMs.emplace_back(feature.GetFieldAsString(i));
            } else {
                auto &values = m_stringAttribs[fieldDef->GetNameRef()];
                values.emplace_back(feature.GetFieldAsString(i));
            }
        }
    }
}

void CDBInstancesAttributes::mergeClassesAttributes(const CDBClassesAttributes &classVectors) noexcept
{
    const auto &classCNAMs = classVectors.getCNAMs();
    size_t instancesCount = getInstancesCount();
    for (size_t i = 0; i < instancesCount; ++i) {
        const std::string &instanceCNAM = m_CNAMs[i];
        auto classCNAM = classCNAMs.find(instanceCNAM);
        if (classCNAM != classCNAMs.end()) {
            auto classIndex = classCNAM->second;
            for (const auto &keyValue : classVectors.getIntegerAttribs()) {
                auto &instanceValues = m_integerAttribs[keyValue.first];
                if (instanceValues.empty()) {
                    instanceValues.resize(instancesCount);
                }

                instanceValues[i] = keyValue.second[classIndex];
            }

            for (const auto &keyValue : classVectors.getDoubleAttribs()) {
                auto &instanceValues = m_doubleAttribs[keyValue.first];
                if (instanceValues.empty()) {
                    instanceValues.resize(instancesCount);
                }

                instanceValues[i] = keyValue.second[classIndex];
            }

            for (const auto &keyValue : classVectors.getStringAttribs()) {
                auto &instanceValues = m_stringAttribs[keyValue.first];
                if (instanceValues.empty()) {
                    instanceValues.resize(instancesCount);
                }

                instanceValues[i] = keyValue.second[classIndex];
            }
        }
    }
}

CDBClassesAttributes::CDBClassesAttributes(GDALDatasetUniquePtr vectorDataset, CDBTile tile)
    : m_tile{std::move(tile)}
{
    for (int i = 0; i < vectorDataset->GetLayerCount(); ++i) {
        OGRLayer *layer = vectorDataset->GetLayer(i);

        for (const auto &feature : *layer) {
            addClassFeaturesAttribs(*feature);
        }
    }
}

void CDBClassesAttributes::addClassFeaturesAttribs(const OGRFeature &feature)
{
    for (int i = 0; i < feature.GetFieldCount(); ++i) {
        auto fieldDef = feature.GetFieldDefnRef(i);
        if (fieldDef->GetType() == OGRFieldType::OFTInteger) {
            auto &values = m_integerAttribs[fieldDef->GetNameRef()];
            values.emplace_back(feature.GetFieldAsInteger(i));
        } else if (fieldDef->GetType() == OGRFieldType::OFTReal) {
            auto &values = m_doubleAttribs[fieldDef->GetNameRef()];
            values.emplace_back(feature.GetFieldAsDouble(i));
        } else if (fieldDef->GetType() == OGRFieldType::OFTString) {
            if (strcmp(fieldDef->GetNameRef(), "CNAM") == 0) {
                auto CNAM = feature.GetFieldAsString(i);
                m_CNAMs[CNAM] = m_CNAMs.size();
            } else {
                auto &values = m_stringAttribs[fieldDef->GetNameRef()];
                values.emplace_back(feature.GetFieldAsString(i));
            }
        }
    }
}

CDBModelsAttributes::CDBModelsAttributes(GDALDatasetUniquePtr featureDataset,
                                         CDBTile tile,
                                         const std::filesystem::path &CDBPath)
    : m_tile{std::move(tile)}
{
    // find position
    for (int i = 0; i < featureDataset->GetLayerCount(); ++i) {
        OGRLayer *layer = featureDataset->GetLayer(i);
        for (const auto &feature : *layer) {
            m_instancesAttribs.addInstanceFeature(*feature);

            const OGRGeometry *geometry = feature->GetGeometryRef();
            if (geometry != nullptr && wkbFlatten(geometry->getGeometryType()) == wkbPoint) {
                const OGRPoint *p = geometry->toPoint();
                m_cartographicPositions.emplace_back(glm::radians(p->getX()),
                                                     glm::radians(p->getY()),
                                                     p->getZ());
            }
        }
    }

    // add class attributes first
    auto classAttribues = createClassesAttributes(tile, CDBPath);
    if (classAttribues) {
        m_instancesAttribs.mergeClassesAttributes(*classAttribues);
    }

    // find orientations
    const auto &doubleAttribs = m_instancesAttribs.getDoubleAttribs();
    auto orientationAttribs = doubleAttribs.find("AO1");
    if (orientationAttribs != doubleAttribs.end()) {
        m_orientations.reserve(orientationAttribs->second.size());
        for (size_t i = 0; i < orientationAttribs->second.size(); ++i) {
            m_orientations.emplace_back(glm::radians(orientationAttribs->second[i]));
        }
    }

    // find scale
    auto scaleXAttribs = doubleAttribs.find("SCALx");
    auto scaleYAttribs = doubleAttribs.find("SCALy");
    auto scaleZAttribs = doubleAttribs.find("SCALz");
    if (scaleXAttribs != doubleAttribs.end() && scaleYAttribs != doubleAttribs.end()
        && scaleZAttribs != doubleAttribs.end()) {
        m_scales.reserve(scaleXAttribs->second.size());
        for (size_t i = 0; i < scaleXAttribs->second.size(); ++i) {
            m_scales.emplace_back(scaleXAttribs->second[i],
                                  scaleYAttribs->second[i],
                                  scaleZAttribs->second[i]);
        }
    }
}

std::optional<CDBClassesAttributes> CDBModelsAttributes::createClassesAttributes(
    const CDBTile &instancesTile, const std::filesystem::path &CDBPath)
{
    int CS_2 = instancesTile.getCS_2();
    if (CS_2 == static_cast<int>(CDBVectorCS2::PointFeature)) {
        CS_2 = static_cast<int>(CDBVectorCS2::PointFeatureClassLevel);
    } else {
        return std::nullopt;
    }

    CDBTile classVectorsTile = CDBTile(instancesTile.getGeoCell(),
                                       instancesTile.getDataset(),
                                       instancesTile.getCS_1(),
                                       CS_2,
                                       instancesTile.getLevel(),
                                       instancesTile.getUREF(),
                                       instancesTile.getRREF());

    auto classLevelPath = CDBPath / (classVectorsTile.getRelativePath().string() + ".dbf");
    if (!std::filesystem::exists(classLevelPath)) {
        return std::nullopt;
    }

    GDALDatasetUniquePtr dataset = GDALDatasetUniquePtr(
        (GDALDataset *) GDALOpenEx(classLevelPath.string().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    if (!dataset) {
        return std::nullopt;
    }

    return CDBClassesAttributes(std::move(dataset), std::move(classVectorsTile));
}
} // namespace CDBTo3DTiles
