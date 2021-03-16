#pragma once

#include <string>

namespace CDBTo3DTiles {
enum class CDBDataset
{
    MultipleContents = 0,
    Elevation = 1,
    MinMaxElevation = 2,
    MaxCulture = 3,
    Imagery = 4,
    RMTexture = 5,
    RMDescriptor = 6,
    GSFeature = 100,
    GTFeature = 101,
    GeoPolitical = 102,
    VectorMaterial = 200,
    RoadNetwork = 201,
    RailRoadNetwork = 202,
    PowerlineNetwork = 203,
    HydrographyNetwork = 204,
    GSModelGeometry = 300,
    GSModelTexture = 301,
    GSModelSignature = 302,
    GSModelDescriptor = 303,
    GSModelMaterial = 304,
    GSModelInteriorGeometry = 305,
    GSModelInteriorTexture = 306,
    GSModelInteriorDescriptor = 307,
    GSModelInteriorMaterial = 308,
    GSModelCMT = 309,
    T2DModelGeometry = 310,
    GSModelInteriorCMT = 311,
    T2DModelCMT = 312,
    NavData = 400,
    Navigation = 401,
    GTModelGeometry_500 = 500,
    GTModelGeometry_510 = 510,
    GTModelTexture = 511,
    GTModelSignature = 512,
    GTModelDescriptor = 503,
    GTModelMaterial = 504,
    GTModelCMT = 505,
    GTModelInteriorGeometry = 506,
    GTModelInteriorTexture = 507,
    GTModelInteriorDescriptor = 508,
    GTModelInteriorMaterial = 509,
    GTModelInteriorCMT = 513,
    MModelGeometry = 600,
    MModelTexture = 601,
    MModelSignature = 606,
    MModelDescriptor = 603,
    MModelMaterial = 604,
    MModelCMT = 605,
    Metadata = 700,
    ClientSpecific = 701,
};

std::string getCDBDatasetDirectoryName(CDBDataset dataset) noexcept;

bool isValidDataset(int value) noexcept;
} // namespace CDBTo3DTiles
