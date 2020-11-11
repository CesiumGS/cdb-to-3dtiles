#pragma once

#include "CDBAttributes.h"
#include "Scene.h"
#include "osg/NodeVisitor"
#include "osg/StateSet"
#include "osgDB/Archive"
#include <map>
#include <stack>

namespace CDBTo3DTiles {
class GeometryValueVisitor : public osg::ValueVisitor
{
public:
    void apply(osg::Vec2 &inv) override { vec2 = inv; }

    void apply(osg::Vec3 &inv) override { vec3 = inv; }

    void apply(osg::Vec3d &inv) override { dvec3 = inv; }

    osg::Vec3 vec3;
    osg::Vec2 vec2;
    osg::Vec3d dvec3;
};

class GeometryPrimitiveFunctor : public osg::PrimitiveIndexFunctor
{
public:
    GeometryPrimitiveFunctor(Mesh &mesh);

    void setVertexArray(unsigned int, const osg::Vec2 *) override {}

    void setVertexArray(unsigned int, const osg::Vec3 *) override {}

    void setVertexArray(unsigned int, const osg::Vec4 *) override {}

    void setVertexArray(unsigned int, const osg::Vec2d *) override {}

    void setVertexArray(unsigned int, const osg::Vec3d *) override {}

    void setVertexArray(unsigned int, const osg::Vec4d *) override {}

    void writeTriangle(unsigned int i0, unsigned int i1, unsigned int i2);

    void drawArrays(GLenum mode, GLint first, GLsizei count) override;

    void drawElements(GLenum mode, GLsizei count, const GLubyte *indices) override;

    void drawElements(GLenum mode, GLsizei count, const GLushort *indices) override;

    void drawElements(GLenum mode, GLsizei count, const GLuint *indices) override;

protected:
    template<typename T>
    void drawElementsImplementation(GLenum mode, GLsizei count, const T *indices)
    {
        if (indices == 0 || count == 0)
            return;

        typedef const T *IndexPointer;

        switch (mode) {
        case (GL_TRIANGLES): {
            IndexPointer ilast = &indices[count];
            for (IndexPointer iptr = indices; iptr < ilast; iptr += 3)
                writeTriangle(*iptr, *(iptr + 1), *(iptr + 2));

            break;
        }
        case (GL_TRIANGLE_STRIP): {
            IndexPointer iptr = indices;
            for (GLsizei i = 2; i < count; ++i, ++iptr) {
                if ((i % 2))
                    writeTriangle(*(iptr), *(iptr + 2), *(iptr + 1));
                else
                    writeTriangle(*(iptr), *(iptr + 1), *(iptr + 2));
            }
            break;
        }
        case (GL_QUADS): {
            IndexPointer iptr = indices;
            for (GLsizei i = 3; i < count; i += 4, iptr += 4) {
                writeTriangle(*(iptr), *(iptr + 1), *(iptr + 2));
                writeTriangle(*(iptr), *(iptr + 2), *(iptr + 3));
            }
            break;
        }
        case (GL_QUAD_STRIP): {
            IndexPointer iptr = indices;
            for (GLsizei i = 3; i < count; i += 2, iptr += 2) {
                writeTriangle(*(iptr), *(iptr + 1), *(iptr + 2));
                writeTriangle(*(iptr + 1), *(iptr + 3), *(iptr + 2));
            }
            break;
        }
        case (GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
        case (GL_TRIANGLE_FAN): {
            IndexPointer iptr = indices;
            unsigned int first = *iptr;
            ++iptr;
            for (GLsizei i = 2; i < count; ++i, ++iptr) {
                writeTriangle(first, *(iptr), *(iptr + 1));
            }
            break;
        }
        default:
            // there are other primitives like GL_POINTS, GL_LINES, and so on, but we don't have any OpenFlight samples for them
            // so we won't support it for now
            OSG_WARN << "Geometry converter cannot handle primitive mode " << mode << std::endl;
            break;
        }
    }

private:
    GeometryPrimitiveFunctor &operator=(const GeometryPrimitiveFunctor &);
    size_t m_indexOffset;
    Mesh &m_mesh;
};

class CDBModel3DResult : public osg::NodeVisitor
{
public:
    CDBModel3DResult();

    void setFeatureID(int featureID);

    void setTransformationMatrix(glm::dmat4 transform);

    void apply(osg::Geometry &geometry) override;

    void apply(osg::Geode &geode) override;

    void apply(osg::Group &group) override;

    void finalize();

    inline const std::vector<Mesh> &getMeshes() const noexcept { return m_meshes; }

    inline const std::vector<Material> &getMaterials() const noexcept { return m_materials; }

    inline const std::vector<Texture> &getTextures() const noexcept { return m_textures; }

    inline const std::vector<osg::ref_ptr<osg::Image>> &getImages() const noexcept { return m_images; }

private:
    struct CompareStateSet
    {
        bool operator()(const osg::ref_ptr<osg::StateSet> &ss1, const osg::ref_ptr<osg::StateSet> &ss2) const
        {
            return ss1->compare(*ss2, true) < 0;
        }
    };

    void pushStateSet(osg::StateSet *ss);

    void popStateSet();

    void processStateSet();

    void processGeometry(osg::Geometry &geometry, osg::Matrix matrix = osg::Matrix::identity());

    int m_featureID;
    glm::dmat4 m_transform;
    osg::ref_ptr<osg::StateSet> m_currentStateSet;
    std::stack<osg::ref_ptr<osg::StateSet>> m_stateSets;
    std::map<osg::ref_ptr<osg::StateSet>, uint32_t, CompareStateSet> m_stateSetToMesh;
    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;
    std::vector<Texture> m_textures;
    std::vector<osg::ref_ptr<osg::Image>> m_images;
};

class CDBGTModelCache
{
public:
    CDBGTModelCache(const std::filesystem::path &CDBPath);

    const CDBModel3DResult *locateModel3D(const std::string &FACC,
                                          const std::string &MODL,
                                          int FSC,
                                          std::string &modelKey) const;

private:
    std::string getModelKey(const std::string &FACC, const std::string &MODL, int FCC) const;

    std::filesystem::path m_CDBPath;
    mutable std::map<std::string, CDBModel3DResult> m_keyToModel;
};

class CDBGTModels
{
public:
    explicit CDBGTModels(CDBModelsAttributes attributes, CDBGTModelCache *cache);

    inline const CDBModelsAttributes &getModelsAttributes() const noexcept { return *m_attributes; }

    const CDBModel3DResult *locateModel3D(size_t instanceIdx, std::string &modelKey) const;

    static std::optional<CDBGTModels> createFromModelsAttributes(CDBModelsAttributes attributes,
                                                                 CDBGTModelCache *cache);

private:
    CDBGTModelCache *m_cache;
    std::optional<CDBModelsAttributes> m_attributes;
};

class CDBGSModels
{
public:
    explicit CDBGSModels(CDBModelsAttributes modelsAttributes,
                         const CDBTile &tile,
                         const osgDB::Archive &GSModelZip,
                         const osgDB::Options &options);

    inline const CDBInstancesAttributes &getInstancesAttributes() const noexcept { return m_attributes; }

    inline const CDBTile &getTile() const noexcept { return *m_tile; }

    inline const CDBModel3DResult &getModel3D() const noexcept { return m_model3DResult; }

    static std::optional<CDBGSModels> createFromModelsAttributes(CDBModelsAttributes attributes,
                                                                 const std::filesystem::path &CDBPath);

private:
    class FindGSModelTexture : public osgDB::FindFileCallback, public osgDB::ReadFileCallback
    {
    public:
        FindGSModelTexture(const std::string &GSModelTextureTileName, osg::ref_ptr<osgDB::Archive> archive);

        std::string findDataFile(const std::string &filename,
                                 const osgDB::Options *options,
                                 osgDB::CaseSensitivity caseSensitivity) override;

        osgDB::ReaderWriter::ReadResult readImage(const std::string &filename,
                                                  const osgDB::Options *options) override;

    private:
        std::string searchArchiveTextureName(const std::string &filename);

        osg::ref_ptr<osgDB::Archive> m_archive;
        std::map<std::string, std::string> m_archiveEntries;
        std::string m_GSModelTextureTileName;
    };

    void extractInputInstancesAttribs(const std::vector<size_t> &extractedInstancesIdx,
                                      const CDBInstancesAttributes &instancesAttribs);

    std::string getModelFilename(const std::string &FACC, const std::string &MODL, int FSC) const;

    std::string m_tileFilename;
    CDBModel3DResult m_model3DResult;
    std::optional<CDBTile> m_tile;
    CDBInstancesAttributes m_attributes;
};
} // namespace CDBTo3DTiles
