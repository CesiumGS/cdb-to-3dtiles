#pragma once

#include "CDBTile.h"
#include "Cartographic.h"
#include "gdal_priv.h"
#include <glm/glm.hpp>

namespace CDBTo3DTiles {
class CDBClassesAttributes;

glm::dmat4 calculateModelOrientation(glm::dvec3 worldPosition, double orientation);

enum class CDBVectorCS2
{
    PointFeature = 1,
    PointFeatureClassLevel = 2,
    LinealFeature = 3,
    LinealFeatureClassLevel = 4,
    PolygonFeature = 5,
    PolygonFeatureClassLevel = 6,
    LinealFigurePointFeature = 7,
    LinealFigurePointFeatureClassLevel = 8,
    PolygonFigurePointFeature = 9,
    PolygonFigurePointFeatureClassLevel = 10,
    Relationship2DTileConnection = 11,
    Relationship2DDatasetConnection = 15,
    PointFeatureExtendedLevel = 16,
    LinealFeatureExtendedLevel = 17,
    PolygonFeatureExtendedLevel = 18,
    LinealFigurePointExtendedLevel = 19,
    PolygonFigurePointExtendedLevel = 20
};

struct CDBAttributes
{
    std::map<std::string, std::string> names;
    std::map<std::string, std::string> descriptions;

    CDBAttributes()
    {
        names["AO1"] = "Angle of Orientation";
        names["BBH"] = "Bounding Box Height";
        names["BBL"] = "Bounding Box Length";
        names["BBW"] = "Bounding Box Width";
        names["BSR"] = "Bounding Sphere Radius";
        names["CMIX"] = "Composite Material Index";
        names["FSC"] = "Feature Classification Code";
        names["HGT"] = "Height above surface level";
        names["MLOD"] = "Model Level Of Detail";
        names["NIS"] = "Number of Instances";
        names["NIX"] = "Number of Indices";
        names["NNL"] = "Number of Normals";
        names["NTC"] = "Number of Texture Coordinates";
        names["NTX"] = "Number of Texels";
        names["NVT"] = "Number of Vertices";
        names["RTAI"] = "Relative Tactical Importance";
        names["SSC"] = "Structure Shape Category";
        names["SSR"] = "Structure Shape of Roof";

        descriptions["AO1"] = "The angular distance measured from true north (0 deg) clockwise to the major (Y) axis of the feature. If the feature is square, the axis 0 through 89.999 deg shall be recorded. If the feature is circular, 360.000 deg shall be recorded. Recommended Usage. CDB readers should default to a value of 0.000 if AO1 is missing. Applicable to Point, Light Point, Moving Model Location and Figure Point features. When used in conjunction with the PowerLine dataset, AO1 corresponds to the orientation of the Y-axis of the modeled pylon. The modeled pylon should be oriented (in its local Cartesian space) so that the wires nominally attach along the Y-axis.";
        descriptions["BBH"] = "The Height/Width/Length of the Bounding Box of the 3D model associated with a point feature. It is the dimension of the box centered at the model origin and that bounds the portion of the model above its XY plane, including the envelopes of all articulated parts. BBH refers to height of the box above the XY plane of the model, BBW refers to the width of the box along the X-axis, and BBL refers to the length of the box along the Y-axis. Note that for 3D models used as cultural features, the XY plane of the model corresponds to its ground reference plane. The value of BBH, BBW and BBL should be accounted for by client-devices (in combination with other information) to determine the appropriate distance at which the model should be paged-in, rendered or processed. BBH, BBW and BBL are usually generated through database authoring tool automation. Optional on features for which a MODL has been assigned. When missing, CDB readers should default BBH to the value of BSR, and BBW and BBL to twice the value of BSR. The dimension of the bounding box is intrinsic to the model and identical for all LOD representations.";
        descriptions["BBL"] = "The length of a feature.";
        descriptions["BBW"] = "The width of a feature.";
        descriptions["BSR"] = "The radius of a feature. In the case where a feature references an associated 3D model, it is the radius of the hemisphere centered at the model origin and that bounds the portion of the model above its XY plane, including the envelopes of all articulated parts. Note that for 3D models used as cultural features, the XY plane of the model corresponds to its ground reference plane. The value of BSR should be accounted for by client-devices (in combination with other information) to determine the appropriate distance at which the model should be paged-in, rendered or processed. When the feature does not reference a 3D model, BSR is the radius of the abstract point representing the feature (e.g., a city). ";
        descriptions["CMIX"] = "Index into the Composite Material Table is used to determine the Base Materials composition of the associated feature.";
        descriptions["FSC"] = "This code, in conjunction with the FACC is used to distinguish and categorize features within a dataset.";
        descriptions["HGT"] = "Distance measured from the lowest point of the base at ground (non-floating objects) or water level (floating objects downhill side/downstream side) to the tallest point of the feature above the surface. Recorded values are positive numbers. In the case of roads and railroads, HGT corresponds to the elevation of the road/railroad wrt terrain in its immediate vicinity.";
        descriptions["MLOD"] = "The level of detail of the 3D model associated with the point feature. When used in conjunction with MODL, the MLOD attribute indicates the LOD where the corresponding MODL is found. In this case, the value of MLOD can never be larger than the LOD of the Vector Tile-LOD that contains it. When used in the context of Airport and Environmental Light Point features, the value of MLOD, if present, indicates that this light point also exist in a 3D model found at the specified LOD. In such case, the value of MLOD is not constrained and can indicate any LOD.";
        descriptions["NIS"] = "Number of instances found in the 3D model associated with the cultural point feature.";
        descriptions["NIX"] = "Number of indices found in the 3D model associated with the cultural point feature.";
        descriptions["NNL"] = "Number of normal vectors found in the 3D model associated with the cultural point feature.";
        descriptions["NTC"] = "Number of texture coordinates found in the 3D model associated with the cultural point feature.";
        descriptions["NTX"] = "Number of texels found in the 3D model associated with the cultural point feature.";
        descriptions["NVT"] = "Number of vertices of the 3D model associated with a point feature.";
        descriptions["RTAI"] = "Provides the Relative TActical Importance of moving models or cultural features relative to other features for the purpose of client-device scene/load management. A value of 100% corresponds to the highest importance; a value of 0% corresponds to the lowest importance. When confronted with otherwise identical objects that differ only wrt to their RelativeTActical Importance, client-devices should always discard features with lower importance before those of higher importance in the course of performing their scene / load management function. As a result, a value of zero gives complete freedom to client-devices to discard the feature as soon as the load of the client-device is exceeded. The effectiveness of scene / load management functions can be severely hampered if large quantities of features are assigned the same Relative TActical Importance by the modeler. In effect, if all models are assigned the same value, the client-devices have no means to distinguish tactically important objects from each other. Assigning a value of 1% to all objects is equivalent to assigning them all a value of 99%. Ideally, the assignment of tactical importance to features should be in accordance to a histogram similar to the one shown here. The shape of the curve is not critical, however the proportion of models tagged with a high importance compared to those with low importance is critical in achieving effective scene/load management schemes. It is illustrated here to show that few models should have an importance of 100 with progressively more models with lower importance. The assignment of the RTAI to each feature lends itself to database tools automation. For instance, RTAI could be based on a look-up function which factors the feature’s type (FACC or MMDC). The value of Relative TActical Importance should be accounted for by client-devices (in combination with other information) to determine the appropriate distance at which the model should be rendered or processed. Relative TActical Importance is mandatory. It has no default value.";
        descriptions["SSC"] = "Describes the Geometric form, appearance, or configuration of the feature.";
        descriptions["SSR"] = "Describes the roof shape.";
    }
};

class CDBInstancesAttributes
{
public:
    void addInstanceFeature(const OGRFeature &feature);

    void mergeClassesAttributes(const CDBClassesAttributes &classVectors) noexcept;

    inline size_t getInstancesCount() const noexcept { return m_CNAMs.size(); }

    inline const std::vector<std::string> &getCNAMs() const noexcept { return m_CNAMs; }

    inline const std::map<std::string, std::vector<int>> &getIntegerAttribs() const noexcept
    {
        return m_integerAttribs;
    }

    inline const std::map<std::string, std::vector<double>> &getDoubleAttribs() const noexcept
    {
        return m_doubleAttribs;
    }

    inline const std::map<std::string, std::vector<std::string>> &getStringAttribs() const noexcept
    {
        return m_stringAttribs;
    }

    inline std::vector<std::string> &getCNAMs() noexcept { return m_CNAMs; }

    inline std::map<std::string, std::vector<int>> &getIntegerAttribs() noexcept { return m_integerAttribs; }

    inline std::map<std::string, std::vector<double>> &getDoubleAttribs() noexcept { return m_doubleAttribs; }

    inline std::map<std::string, std::vector<std::string>> &getStringAttribs() noexcept
    {
        return m_stringAttribs;
    }

private:
    std::vector<std::string> m_CNAMs;
    std::map<std::string, std::vector<int>> m_integerAttribs;
    std::map<std::string, std::vector<double>> m_doubleAttribs;
    std::map<std::string, std::vector<std::string>> m_stringAttribs;
};

class CDBClassesAttributes
{
public:
    CDBClassesAttributes(GDALDatasetUniquePtr dataset, CDBTile tile);

    inline const CDBTile &getTile() const noexcept { return *m_tile; }

    inline const std::map<std::string, size_t> &getCNAMs() const noexcept { return m_CNAMs; }

    inline const std::map<std::string, std::vector<int>> &getIntegerAttribs() const noexcept
    {
        return m_integerAttribs;
    }

    inline const std::map<std::string, std::vector<double>> &getDoubleAttribs() const noexcept
    {
        return m_doubleAttribs;
    }

    inline const std::map<std::string, std::vector<std::string>> &getStringAttribs() const noexcept
    {
        return m_stringAttribs;
    }

private:
    void addClassFeaturesAttribs(const OGRFeature &feature);

    std::optional<CDBTile> m_tile;
    std::map<std::string, size_t> m_CNAMs;
    std::map<std::string, std::vector<int>> m_integerAttribs;
    std::map<std::string, std::vector<double>> m_doubleAttribs;
    std::map<std::string, std::vector<std::string>> m_stringAttribs;
};

class CDBModelsAttributes
{
public:
    CDBModelsAttributes(GDALDatasetUniquePtr dataset, CDBTile tile, const std::filesystem::path &CDBPath);

    inline const std::vector<Core::Cartographic> &getCartographicPositions() const noexcept
    {
        return m_cartographicPositions;
    }

    inline std::vector<Core::Cartographic> &getCartographicPositions() noexcept
    {
        return m_cartographicPositions;
    }

    inline const std::vector<double> &getOrientations() const noexcept { return m_orientations; }

    inline const std::vector<glm::vec3> &getScales() const noexcept { return m_scales; }

    inline const CDBTile &getTile() const noexcept { return *m_tile; }

    inline const CDBInstancesAttributes &getInstancesAttributes() const noexcept
    {
        return m_instancesAttribs;
    }

private:
    std::optional<CDBClassesAttributes> createClassesAttributes(const CDBTile &instancesTile,
                                                                const std::filesystem::path &CDBPath);

    std::vector<glm::vec3> m_scales;
    std::vector<double> m_orientations;
    std::vector<Core::Cartographic> m_cartographicPositions;
    CDBInstancesAttributes m_instancesAttribs;
    std::optional<CDBTile> m_tile;
};
} // namespace CDBTo3DTiles
