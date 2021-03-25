#include "CDBDataset.h"
#include "Utility.h"
#include <cassert>

namespace CDBTo3DTiles {
std::string getCDBDatasetDirectoryName(CDBDataset dataset) noexcept
{
    std::string name;
    switch (dataset) {
    case CDBDataset::Elevation:
        name = "Elevation";
        break;
    case CDBDataset::MinMaxElevation:
        name = "MinMaxElevation";
        break;
    case CDBDataset::MaxCulture:
        name = "MaxCulture";
        break;
    case CDBDataset::Imagery:
        name = "Imagery";
        break;
    case CDBDataset::RMTexture:
        name = "RMTexture";
        break;
    case CDBDataset::RMDescriptor:
        name = "RMDescriptor";
        break;
    case CDBDataset::GSFeature:
        name = "GSFeature";
        break;
    case CDBDataset::GTFeature:
        name = "GTFeature";
        break;
    case CDBDataset::GeoPolitical:
        name = "GeoPolitical";
        break;
    case CDBDataset::VectorMaterial:
        name = "VectorMaterial";
        break;
    case CDBDataset::RoadNetwork:
        name = "RoadNetwork";
        break;
    case CDBDataset::RailRoadNetwork:
        name = "RailRoadNetwork";
        break;
    case CDBDataset::PowerlineNetwork:
        name = "PowerLineNetwork";
        break;
    case CDBDataset::HydrographyNetwork:
        name = "HydrographyNetwork";
        break;
    case CDBDataset::GSModelGeometry:
        name = "GSModelGeometry";
        break;
    case CDBDataset::GSModelTexture:
        name = "GSModelTexture";
        break;
    case CDBDataset::GSModelSignature:
        name = "GSModelSignature";
        break;
    case CDBDataset::GSModelDescriptor:
        name = "GSModelDescriptor";
        break;
    case CDBDataset::GSModelMaterial:
        name = "GSModelMaterial";
        break;
    case CDBDataset::GSModelInteriorGeometry:
        name = "GSModelInteriorGeometry";
        break;
    case CDBDataset::GSModelInteriorTexture:
        name = "GSModelInteriorTexture";
        break;
    case CDBDataset::GSModelInteriorDescriptor:
        name = "GSModelInteriorDescriptor";
        break;
    case CDBDataset::GSModelInteriorMaterial:
        name = "GSModelInteriorMaterial";
        break;
    case CDBDataset::GSModelCMT:
        name = "GSModelCMT";
        break;
    case CDBDataset::T2DModelGeometry:
        name = "T2DModelGeometry";
        break;
    case CDBDataset::GSModelInteriorCMT:
        name = "GSModelInteriorCMT";
        break;
    case CDBDataset::T2DModelCMT:
        name = "T2DModelCMT";
        break;
    case CDBDataset::NavData:
        name = "NavData";
        break;
    case CDBDataset::Navigation:
        name = "Navigation";
        break;
    case CDBDataset::GTModelGeometry_500:
    case CDBDataset::GTModelGeometry_510:
        name = "GTModelGeometry";
        break;
    case CDBDataset::GTModelTexture:
        name = "GTModelTexture";
        break;
    case CDBDataset::GTModelSignature:
        name = "GTModelSignature";
        break;
    case CDBDataset::GTModelDescriptor:
        name = "GTModelDescriptor";
        break;
    case CDBDataset::GTModelMaterial:
        name = "GTModelMaterial";
        break;
    case CDBDataset::GTModelCMT:
        name = "GTModelCMT";
        break;
    case CDBDataset::GTModelInteriorGeometry:
        name = "GTModelInteriorGeometry";
        break;
    case CDBDataset::GTModelInteriorTexture:
        name = "GTModelInteriorTexture";
        break;
    case CDBDataset::GTModelInteriorDescriptor:
        name = "GTModelInteriorDescriptor";
        break;
    case CDBDataset::GTModelInteriorMaterial:
        name = "GTModelInteriorMaterial";
        break;
    case CDBDataset::GTModelInteriorCMT:
        name = "GTModelInteriorCMT";
        break;
    case CDBDataset::MModelGeometry:
        name = "MModelGeometry";
        break;
    case CDBDataset::MModelTexture:
        name = "MModelTexture";
        break;
    case CDBDataset::MModelSignature:
        name = "MModelSignature";
        break;
    case CDBDataset::MModelDescriptor:
        name = "MModelDescriptor";
        break;
    case CDBDataset::MModelMaterial:
        name = "MModelMaterial";
        break;
    case CDBDataset::MModelCMT:
        name = "MModelCMT";
        break;
    case CDBDataset::Metadata:
        name = "Metadata";
        break;
    case CDBDataset::ClientSpecific:
        name = "ClientSpecific";
        break;
    default:
        assert(false && "Encounter unknown CDBDataset");
        break;
    }

    return toStringWithZeroPadding(3, static_cast<unsigned>(dataset)) + "_" + name;
}

bool isValidDataset(int value) noexcept
{
    switch (value) {
    case static_cast<int>(CDBDataset::Elevation):
    case static_cast<int>(CDBDataset::MinMaxElevation):
    case static_cast<int>(CDBDataset::MaxCulture):
    case static_cast<int>(CDBDataset::Imagery):
    case static_cast<int>(CDBDataset::RMTexture):
    case static_cast<int>(CDBDataset::RMDescriptor):
    case static_cast<int>(CDBDataset::GSFeature):
    case static_cast<int>(CDBDataset::GTFeature):
    case static_cast<int>(CDBDataset::GeoPolitical):
    case static_cast<int>(CDBDataset::VectorMaterial):
    case static_cast<int>(CDBDataset::RoadNetwork):
    case static_cast<int>(CDBDataset::RailRoadNetwork):
    case static_cast<int>(CDBDataset::PowerlineNetwork):
    case static_cast<int>(CDBDataset::HydrographyNetwork):
    case static_cast<int>(CDBDataset::GSModelGeometry):
    case static_cast<int>(CDBDataset::GSModelTexture):
    case static_cast<int>(CDBDataset::GSModelSignature):
    case static_cast<int>(CDBDataset::GSModelDescriptor):
    case static_cast<int>(CDBDataset::GSModelMaterial):
    case static_cast<int>(CDBDataset::GSModelInteriorGeometry):
    case static_cast<int>(CDBDataset::GSModelInteriorTexture):
    case static_cast<int>(CDBDataset::GSModelInteriorDescriptor):
    case static_cast<int>(CDBDataset::GSModelInteriorMaterial):
    case static_cast<int>(CDBDataset::GSModelCMT):
    case static_cast<int>(CDBDataset::T2DModelGeometry):
    case static_cast<int>(CDBDataset::GSModelInteriorCMT):
    case static_cast<int>(CDBDataset::T2DModelCMT):
    case static_cast<int>(CDBDataset::NavData):
    case static_cast<int>(CDBDataset::Navigation):
    case static_cast<int>(CDBDataset::GTModelGeometry_500):
    case static_cast<int>(CDBDataset::GTModelGeometry_510):
    case static_cast<int>(CDBDataset::GTModelTexture):
    case static_cast<int>(CDBDataset::GTModelSignature):
    case static_cast<int>(CDBDataset::GTModelDescriptor):
    case static_cast<int>(CDBDataset::GTModelMaterial):
    case static_cast<int>(CDBDataset::GTModelCMT):
    case static_cast<int>(CDBDataset::GTModelInteriorGeometry):
    case static_cast<int>(CDBDataset::GTModelInteriorTexture):
    case static_cast<int>(CDBDataset::GTModelInteriorDescriptor):
    case static_cast<int>(CDBDataset::GTModelInteriorMaterial):
    case static_cast<int>(CDBDataset::GTModelInteriorCMT):
    case static_cast<int>(CDBDataset::MModelGeometry):
    case static_cast<int>(CDBDataset::MModelTexture):
    case static_cast<int>(CDBDataset::MModelSignature):
    case static_cast<int>(CDBDataset::MModelDescriptor):
    case static_cast<int>(CDBDataset::MModelMaterial):
    case static_cast<int>(CDBDataset::MModelCMT):
    case static_cast<int>(CDBDataset::Metadata):
    case static_cast<int>(CDBDataset::ClientSpecific):
        return true;
    default:
        return false;
    }
}

} // namespace CDBTo3DTiles
