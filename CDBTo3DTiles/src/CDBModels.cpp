#include "CDBModels.h"
#include "CDB.h"
#include "Ellipsoid.h"
#include "MathHelpers.h"
#include "glm/glm.hpp"
#include "glm/gtc/epsilon.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "osg/Material"
#include "osgDB/ReadFile"
#include <unordered_set>

namespace CDBTo3DTiles {
static TextureFilter convertOsgTexFilter(osg::Texture::FilterMode);

GeometryPrimitiveFunctor::GeometryPrimitiveFunctor(Mesh &mesh)
    : osg::PrimitiveIndexFunctor()
    , m_mesh{mesh}
{
    m_indexOffset = m_mesh.positions.size();
}

void GeometryPrimitiveFunctor::writeTriangle(unsigned int i0, unsigned int i1, unsigned int i2)
{
    m_mesh.indices.emplace_back(m_indexOffset + i0);
    m_mesh.indices.emplace_back(m_indexOffset + i1);
    m_mesh.indices.emplace_back(m_indexOffset + i2);
}

void GeometryPrimitiveFunctor::drawArrays(GLenum mode, GLint first, GLsizei count)
{
    switch (mode) {
    case (GL_TRIANGLES): {
        unsigned int pos = static_cast<unsigned int>(first);
        for (GLsizei i = 2; i < count; i += 3, pos += 3) {
            writeTriangle(pos, pos + 1, pos + 2);
        }
        break;
    }
    case (GL_TRIANGLE_STRIP): {
        unsigned int pos = static_cast<unsigned int>(first);
        for (GLsizei i = 2; i < count; ++i, ++pos) {
            if ((i % 2))
                writeTriangle(pos, pos + 2, pos + 1);
            else
                writeTriangle(pos, pos + 1, pos + 2);
        }
        break;
    }
    case (GL_QUADS): {
        unsigned int pos = static_cast<unsigned int>(first);
        for (GLsizei i = 3; i < count; i += 4, pos += 4) {
            writeTriangle(pos, pos + 1, pos + 2);
            writeTriangle(pos, pos + 2, pos + 3);
        }
        break;
    }
    case (GL_QUAD_STRIP): {
        unsigned int pos = static_cast<unsigned int>(first);
        for (GLsizei i = 3; i < count; i += 2, pos += 2) {
            writeTriangle(pos, pos + 1, pos + 2);
            writeTriangle(pos + 1, pos + 3, pos + 2);
        }
        break;
    }
    case (GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
    case (GL_TRIANGLE_FAN): {
        unsigned int pos = static_cast<unsigned int>(first) + 1;
        for (GLsizei i = 2; i < count; ++i, ++pos) {
            writeTriangle(pos - 1, pos, pos + 1);
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

void GeometryPrimitiveFunctor::drawElements(GLenum mode, GLsizei count, const GLubyte *indices)
{
    drawElementsImplementation<GLubyte>(mode, count, indices);
}

void GeometryPrimitiveFunctor::drawElements(GLenum mode, GLsizei count, const GLushort *indices)
{
    drawElementsImplementation<GLushort>(mode, count, indices);
}

void GeometryPrimitiveFunctor::drawElements(GLenum mode, GLsizei count, const GLuint *indices)
{
    drawElementsImplementation<GLuint>(mode, count, indices);
}

GeometryPrimitiveFunctor &GeometryPrimitiveFunctor::operator=(const GeometryPrimitiveFunctor &)
{
    return *this;
}

CDBModel3DResult::CDBModel3DResult()
    : m_featureID{0}
    , m_transform{glm::dmat4(1.0)}
    , m_currentStateSet{new osg::StateSet()}
{
    setTraversalMode(TraversalMode::TRAVERSE_ALL_CHILDREN);
}

void CDBModel3DResult::setFeatureID(int featureID)
{
    m_featureID = featureID;
}

void CDBModel3DResult::setTransformationMatrix(glm::dmat4 transform)
{
    m_transform = transform;
}

void CDBModel3DResult::apply(osg::Geometry &geometry)
{
    osg::Matrix m = osg::computeLocalToWorld(getNodePath());
    pushStateSet(geometry.getStateSet());
    processStateSet();
    processGeometry(geometry, m);
    popStateSet();
}

void CDBModel3DResult::apply(osg::Geode &geode)
{
    pushStateSet(geode.getStateSet());
    for (unsigned i = 0; i < geode.getNumDrawables(); ++i) {
        auto drawable = geode.getDrawable(i);
        drawable->accept(*this);
    }
    popStateSet();
}

void CDBModel3DResult::apply(osg::Group &group)
{
    pushStateSet(group.getStateSet());
    traverse(group);
    popStateSet();
}

void CDBModel3DResult::finalize()
{
    for (auto &mesh : m_meshes) {
        mesh.positionRTCs.reserve(mesh.positions.size());
        auto center = mesh.aabb->center();
        for (auto pos : mesh.positions) {
            mesh.positionRTCs.emplace_back(pos - center);
        }
    }
}

void CDBModel3DResult::pushStateSet(osg::StateSet *ss)
{
    if (ss != nullptr) {
        // Save our current stateset
        m_stateSets.push(m_currentStateSet.get());

        // merge with node stateset
        m_currentStateSet = static_cast<osg::StateSet *>(m_currentStateSet->clone(osg::CopyOp::SHALLOW_COPY));
        m_currentStateSet->merge(*ss);
    }
}

void CDBModel3DResult::popStateSet()
{
    if (!m_stateSets.empty()) {
        m_currentStateSet = m_stateSets.top();
        m_stateSets.pop();
    }
}

TextureFilter convertOsgTexFilter(osg::Texture::FilterMode mode)
{
    switch (mode) {
    case osg::Texture::FilterMode::LINEAR:
        return TextureFilter::LINEAR;
    case osg::Texture::FilterMode::LINEAR_MIPMAP_LINEAR:
        return TextureFilter::LINEAR_MIPMAP_LINEAR;
    case osg::Texture::FilterMode::LINEAR_MIPMAP_NEAREST:
        return TextureFilter::LINEAR_MIPMAP_NEAREST;
    case osg::Texture::FilterMode::NEAREST:
        return TextureFilter::NEAREST;
    case osg::Texture::FilterMode::NEAREST_MIPMAP_LINEAR:
        return TextureFilter::NEAREST_MIPMAP_LINEAR;
    case osg::Texture::FilterMode::NEAREST_MIPMAP_NEAREST:
        return TextureFilter::NEAREST_MIPMAP_NEAREST;
    default:
        assert(false && "Encounter unknown osg::Texture::FilterMode");
        return TextureFilter::LINEAR;
    }
}

void CDBModel3DResult::processStateSet()
{
    if (m_currentStateSet == nullptr) {
        return;
    }

    if (m_stateSetToMesh.find(m_currentStateSet) == m_stateSetToMesh.end()) {
        Mesh mesh{};
        mesh.aabb = AABB();

        osg::Material *mat = dynamic_cast<osg::Material *>(
            m_currentStateSet->getAttribute(osg::StateAttribute::MATERIAL));
        osg::Texture *tex = dynamic_cast<osg::Texture *>(
            m_currentStateSet->getTextureAttribute(0, osg::StateAttribute::TEXTURE));

        if (mat) {
            osg::Vec4 ambient = mat->getAmbient(osg::Material::FRONT);
            osg::Vec4 diffuse = mat->getDiffuse(osg::Material::FRONT);
            osg::Vec4 specular = mat->getSpecular(osg::Material::FRONT);
            osg::Vec4 emission = mat->getEmission(osg::Material::FRONT);
            float shininess = mat->getShininess(osg::Material::FRONT);

            Material material;
            material.doubleSided = true;
            material.alpha = ambient[3];
            material.ambient = glm::vec3(ambient[0], ambient[1], ambient[2]);
            material.diffuse = glm::vec3(diffuse[0], diffuse[1], diffuse[2]);
            material.specular = glm::vec3(specular[0], specular[1], specular[2]);
            material.emission = glm::vec3(emission[0], emission[1], emission[2]);
            material.shininess = shininess;

            if (tex) {
                Texture texture;

                osg::Texture::FilterMode magFilter = tex->getFilter(osg::Texture::FilterParameter::MAG_FILTER);
                osg::Texture::FilterMode minFilter = tex->getFilter(osg::Texture::FilterParameter::MIN_FILTER);
                texture.magFilter = convertOsgTexFilter(magFilter);
                texture.minFilter = convertOsgTexFilter(minFilter);

                osg::Image *img = tex->getImage(0);
                if ((img) && (!img->getFileName().empty())) {
                    std::filesystem::path texturePath = img->getFileName();
                    texture.uri = texturePath.stem().replace_extension(".png");
                    m_images.emplace_back(img);
                    m_textures.emplace_back(texture);
                    material.texture = static_cast<int>(m_textures.size() - 1);
                }
            }

            m_materials.emplace_back(material);
            mesh.material = static_cast<int>(m_materials.size() - 1);
        } else {
            Material material;
            material.doubleSided = true;

            m_materials.emplace_back(material);
            mesh.material = static_cast<int>(m_materials.size() - 1);
        }

        m_meshes.emplace_back(mesh);
        m_stateSetToMesh.insert({m_currentStateSet, m_meshes.size() - 1});
    }
}

void CDBModel3DResult::processGeometry(osg::Geometry &geometry, osg::Matrix matrix)
{
    uint32_t meshIdx = m_stateSetToMesh[m_currentStateSet];
    auto valueVisitor = GeometryValueVisitor();

    // Positions are always required
    auto vertexArray = geometry.getVertexArray();
    if (!vertexArray) {
        return;
    }

    GeometryPrimitiveFunctor pif(m_meshes[meshIdx]);
    for (unsigned int i = 0; i < geometry.getNumPrimitiveSets(); ++i) {
        osg::PrimitiveSet *ps = geometry.getPrimitiveSet(i);
        ps->accept(pif);
    }

    // create transform matrix. Caution OSG matrix is row major
    auto OSGMatrixVal = matrix.ptr();
    glm::dmat4 transform = glm::dmat4(OSGMatrixVal[0],
                                      OSGMatrixVal[1],
                                      OSGMatrixVal[2],
                                      OSGMatrixVal[3],
                                      OSGMatrixVal[4],
                                      OSGMatrixVal[5],
                                      OSGMatrixVal[6],
                                      OSGMatrixVal[7],
                                      OSGMatrixVal[8],
                                      OSGMatrixVal[9],
                                      OSGMatrixVal[10],
                                      OSGMatrixVal[11],
                                      OSGMatrixVal[12],
                                      OSGMatrixVal[13],
                                      OSGMatrixVal[14],
                                      OSGMatrixVal[15]);
    transform = m_transform * glm::transpose(transform);

    // parse positions
    if (vertexArray->getType() == osg::Array::Type::Vec3ArrayType) {
        for (unsigned i = 0; i < vertexArray->getNumElements(); ++i) {
            vertexArray->accept(i, valueVisitor);
            osg::Vec3 pos = valueVisitor.vec3;
            glm::dvec3 glmWorldPos = transform * glm::dvec4(pos[0], pos[1], pos[2], 1.0);
            m_meshes[meshIdx].aabb->merge(glmWorldPos);
            m_meshes[meshIdx].positions.emplace_back(glmWorldPos);
            m_meshes[meshIdx].batchIDs.emplace_back(m_featureID);
        }
    }

    // parse normal
    auto normalArray = geometry.getNormalArray();
    if (normalArray && normalArray->getType() == osg::Array::Type::Vec3ArrayType) {
        glm::dmat4 normalMatrix = glm::inverse(glm::transpose(transform));
        for (unsigned i = 0; i < normalArray->getNumElements(); ++i) {
            normalArray->accept(i, valueVisitor);
            osg::Vec3 normal = valueVisitor.vec3;
            glm::vec3 glmNormal = normalMatrix * glm::vec4(normal[0], normal[1], normal[2], 0.0);
            if (!glm::epsilonEqual(glm::length(glmNormal), 0.0f, static_cast<float>(Core::Math::EPSILON7))) {
                glmNormal = glm::normalize(glmNormal);
            }

            m_meshes[meshIdx].normals.emplace_back(glmNormal);
        }
    }

    // parse texture
    // When OSG can't read texture, we have the case where mesh has UV but material doesn't have texture.
    // It will lead to size mismatch with positions and normals array since we are grouping those meshes that has UV
    // and the ones that don't together. A check for texture in material is used to prevent such case
    auto textureCoordArray = geometry.getTexCoordArray(0);
    const auto &meshMaterial = m_materials[static_cast<size_t>(m_meshes[meshIdx].material)];
    if (textureCoordArray && meshMaterial.texture != -1) {
        for (unsigned i = 0; i < textureCoordArray->getNumElements(); ++i) {
            textureCoordArray->accept(i, valueVisitor);
            valueVisitor.vec2.y() = 1.0f - valueVisitor.vec2.y();
            m_meshes[meshIdx].UVs.emplace_back(valueVisitor.vec2[0], valueVisitor.vec2[1]);
        }
    }
}

CDBGTModelCache::CDBGTModelCache(const std::filesystem::path &CDBPath)
    : m_CDBPath{CDBPath}
{}

const CDBModel3DResult *CDBGTModelCache::locateModel3D(const std::string &FACC,
                                                       const std::string &MODL,
                                                       int FSC,
                                                       std::string &modelKey) const
{
    std::string key = getModelKey(FACC, MODL, FSC);
    auto model = m_keyToModel.find(key);
    if (model != m_keyToModel.end()) {
        modelKey = key;
        return &model->second;
    }

    for (std::filesystem::directory_entry A_Cartegory : std::filesystem::directory_iterator(
             m_CDBPath / CDB::GTModel / getCDBDatasetDirectoryName(CDBDataset::GTModelGeometry_500))) {
        if (A_Cartegory.path().filename().string().front() == FACC[0]) {
            for (std::filesystem::directory_entry B_Subcartegory :
                 std::filesystem::directory_iterator(A_Cartegory)) {
                if (B_Subcartegory.path().filename().string().front() == FACC[1]) {
                    for (std::filesystem::directory_entry featureCodeDir :
                         std::filesystem::directory_iterator(B_Subcartegory)) {
                        if (featureCodeDir.path().filename().string().substr(0, 3) == FACC.substr(2, 3)) {
                            osg::ref_ptr<osg::Node> geometry = osgDB::readRefNodeFile(featureCodeDir.path()
                                                                                      / (key + ".flt"));
                            if (geometry) {
                                CDBModel3DResult model3D;
                                geometry->accept(model3D);
                                model3D.finalize();
                                m_keyToModel.insert({key, std::move(model3D)});
                                modelKey = key;
                                return &m_keyToModel[key];
                            }
                        }
                    }
                }
            }
        }
    }

    return nullptr;
}

std::string CDBGTModelCache::getModelKey(const std::string &FACC, const std::string &MODL, int FCC) const
{
    return "D500_S001_T001_" + FACC + "_" + toStringWithZeroPadding(3, FCC) + "_" + MODL;
}

CDBGTModels::CDBGTModels(CDBModelsAttributes attributes, CDBGTModelCache *cache)
    : m_cache{cache}
    , m_attributes{std::move(attributes)}
{}

const CDBModel3DResult *CDBGTModels::locateModel3D(size_t instanceIdx, std::string &modelKey) const
{
    const auto &instancesAttribs = m_attributes->getInstancesAttributes();
    const auto &stringAttribs = instancesAttribs.getStringAttribs();
    const auto &integerAttribs = instancesAttribs.getIntegerAttribs();
    auto FACCs = stringAttribs.find("FACC");
    auto MODLs = stringAttribs.find("MODL");
    auto FSCs = integerAttribs.find("FSC");

    if (FACCs != stringAttribs.end() && MODLs != stringAttribs.end() && FSCs != integerAttribs.end()) {
        size_t instanceCount = instancesAttribs.getInstancesCount();
        if (FACCs->second.size() == instanceCount && MODLs->second.size() == instanceCount
            && FSCs->second.size() == instanceCount) {
            return m_cache->locateModel3D(FACCs->second[instanceIdx],
                                          MODLs->second[instanceIdx],
                                          FSCs->second[instanceIdx],
                                          modelKey);
        }
    }

    return nullptr;
}

std::optional<CDBGTModels> CDBGTModels::createFromModelsAttributes(CDBModelsAttributes attributes,
                                                                   CDBGTModelCache *cache)
{
    const auto &instancesAttribs = attributes.getInstancesAttributes();
    const auto &stringAttribs = instancesAttribs.getStringAttribs();
    const auto &integerAttribs = instancesAttribs.getIntegerAttribs();
    auto FACCs = stringAttribs.find("FACC");
    auto MODLs = stringAttribs.find("MODL");
    auto FSCs = integerAttribs.find("FSC");
    if (FACCs == stringAttribs.end() || MODLs == stringAttribs.end() || FSCs == integerAttribs.end()) {
        return std::nullopt;
    }

    return CDBGTModels(std::move(attributes), cache);
}

CDBGSModels::CDBGSModels(CDBModelsAttributes modelsAttributes,
                         const CDBTile &GSModelTile,
                         const osg::ref_ptr<osgDB::Archive> &GSModelArchive,
                         const osg::ref_ptr<osgDB::Options> &options)
    : m_GSModelArchive{GSModelArchive}
    , m_tile{GSModelTile}
{
    m_tileFilename = GSModelTile.getRelativePath().filename().string();

    std::unordered_set<std::string> geometryFilenames;
    osgDB::Archive::FileNameList fileNameList;
    if (m_GSModelArchive->getFileNames(fileNameList)) {
        for (osgDB::Archive::FileNameList::iterator itr = fileNameList.begin(); itr != fileNameList.end();
             ++itr) {
            geometryFilenames.insert(*itr);
        }
    }

    Core::Ellipsoid ellipsoid = Core::Ellipsoid::WGS84;
    const auto &cartographicPositions = modelsAttributes.getCartographicPositions();
    const auto &orientations = modelsAttributes.getOrientations();
    const auto &scales = modelsAttributes.getScales();
    const auto &instancesAttribs = modelsAttributes.getInstancesAttributes();
    const auto &stringAttribs = instancesAttribs.getStringAttribs();
    const auto &integerAttribs = instancesAttribs.getIntegerAttribs();
    auto FACCs = stringAttribs.find("FACC");
    auto MODLs = stringAttribs.find("MODL");
    auto FSCs = integerAttribs.find("FSC");

    // extract attributes for this tile only
    size_t totalInputInstanceCount = instancesAttribs.getInstancesCount();
    std::vector<size_t> extractedInstances;
    extractedInstances.reserve(totalInputInstanceCount);
    int featureID = 0;
    for (size_t i = 0; i < totalInputInstanceCount; ++i) {
        const auto &FACC = FACCs->second[i];
        const auto &MODL = MODLs->second[i];
        int FSC = FSCs->second[i];
        std::string modelFilename = getModelFilename(FACC, MODL, FSC);
        if (geometryFilenames.find(modelFilename) != geometryFilenames.end()) {
            auto result = m_GSModelArchive->readNode(modelFilename, options.get());
            if (result.validNode()) {
                // combine mesh
                osg::ref_ptr<osg::Node> node = result.takeNode();
                glm::dvec3 worldPosition = ellipsoid.cartographicToCartesian(cartographicPositions[i]);

                double orientation = 0.0;
                if (i < orientations.size()) {
                    orientation = orientations[i];
                }

                glm::dvec3 scale(1.0f);
                if (i < scales.size()) {
                    scale = scales[i];
                }

                glm::dmat4 transform = glm::scale(calculateModelOrientation(worldPosition, orientation),
                                                  scale);

                m_model3DResult.setTransformationMatrix(transform);
                m_model3DResult.setFeatureID(featureID);
                node->accept(m_model3DResult);

                // extract input instance index
                extractedInstances.emplace_back(i);
                ++featureID;
            }
        }
    }

    extractInputInstancesAttribs(extractedInstances, instancesAttribs);

    m_model3DResult.finalize();
}

CDBGSModels::~CDBGSModels() noexcept
{
    // OSG doesn't close the archive after ref_ptr is released, so we do it ourselves
    m_GSModelArchive->close();
}

std::string CDBGSModels::getModelFilename(const std::string &FACC, const std::string &MODL, int FSC) const
{
    return "/" + m_tileFilename + "_" + FACC + "_" + toStringWithZeroPadding(3, FSC) + "_" + MODL + ".flt";
}

std::optional<CDBGSModels> CDBGSModels::createFromModelsAttributes(CDBModelsAttributes attributes,
                                                                   const std::filesystem::path &CDBPath)
{
    const auto &instancesAttribs = attributes.getInstancesAttributes();
    const auto &stringAttribs = instancesAttribs.getStringAttribs();
    const auto &integerAttribs = instancesAttribs.getIntegerAttribs();
    auto FACCs = stringAttribs.find("FACC");
    auto MODLs = stringAttribs.find("MODL");
    auto FSCs = integerAttribs.find("FSC");
    if (FACCs == stringAttribs.end() || MODLs == stringAttribs.end() || FSCs == integerAttribs.end()) {
        return std::nullopt;
    }

    // find GSModel archive
    const CDBTile &attributeTile = attributes.getTile();
    CDBTile modelTile(attributeTile.getGeoCell(),
                      CDBDataset::GSModelGeometry,
                      1,
                      1,
                      attributeTile.getLevel(),
                      attributeTile.getUREF(),
                      attributeTile.getRREF());

    std::filesystem::path GSModelZip = CDBPath / (modelTile.getRelativePath().string() + ".zip");
    if (!std::filesystem::exists(GSModelZip)) {
        return std::nullopt;
    }

    osgDB::ReaderWriter *rw = osgDB::Registry::instance()->getReaderWriterForExtension("zip");
    if (rw) {
        // set relative path for GSModel
        osg::ref_ptr<osgDB::Options> options = new osgDB::Options();
        options->setObjectCacheHint(osgDB::Options::CACHE_NONE);
        options->getDatabasePathList().push_front(GSModelZip.parent_path());

        // find GSModelTexture zip file to search for texture
        CDBTile GSModelTextureTile = CDBTile(attributeTile.getGeoCell(),
                                             CDBDataset::GSModelTexture,
                                             1,
                                             1,
                                             attributeTile.getLevel(),
                                             attributeTile.getUREF(),
                                             attributeTile.getRREF());

        std::filesystem::path GSModelTextureRelPath = GSModelTextureTile.getRelativePath();
        std::string GSModelTextureTileName = GSModelTextureRelPath.stem().string();
        std::filesystem::path GSModelTextureZip = CDBPath / (GSModelTextureRelPath.string() + ".zip");
        osgDB::ReaderWriter::ReadResult GSModelTextureRead = rw->openArchive(GSModelTextureZip,
                                                                             osgDB::Archive::READ);
        if (GSModelTextureRead.validArchive()) {
            osg::ref_ptr<osgDB::Archive> GSModelTextureArchive = GSModelTextureRead.takeArchive();
            osg::ref_ptr<FindGSModelTexture> findMissingFile = new FindGSModelTexture(GSModelTextureTileName,
                                                                                      GSModelTextureArchive);
            options->setFindFileCallback(findMissingFile);
            options->setReadFileCallback(findMissingFile);
        }

        // read GSModel geometry
        osgDB::ReaderWriter::ReadResult GSModelRead = rw->openArchive(GSModelZip, osgDB::Archive::READ);
        if (GSModelRead.validArchive()) {
            osg::ref_ptr<osgDB::Archive> archive = GSModelRead.takeArchive();
            return CDBGSModels(std::move(attributes), modelTile, archive, options);
        }
    }

    return std::nullopt;
}

void CDBGSModels::extractInputInstancesAttribs(const std::vector<size_t> &extractedInstancesIdx,
                                               const CDBInstancesAttributes &inputInstancesAttribs)
{
    size_t totalExtracted = extractedInstancesIdx.size();
    auto &modelIntegerAttribs = m_attributes.getIntegerAttribs();
    for (auto inputPair : inputInstancesAttribs.getIntegerAttribs()) {
        const auto &inputValues = inputPair.second;
        auto &modelValues = modelIntegerAttribs[inputPair.first];
        modelValues.reserve(totalExtracted);

        for (auto i : extractedInstancesIdx) {
            modelValues.emplace_back(inputValues[i]);
        }
    }

    auto &modelDoubleAttribs = m_attributes.getDoubleAttribs();
    for (auto inputPair : inputInstancesAttribs.getDoubleAttribs()) {
        const auto &inputValues = inputPair.second;
        auto &modelValues = modelDoubleAttribs[inputPair.first];
        modelValues.reserve(totalExtracted);

        for (auto i : extractedInstancesIdx) {
            modelValues.emplace_back(inputValues[i]);
        }
    }

    auto &modelStringAttribs = m_attributes.getStringAttribs();
    for (auto inputPair : inputInstancesAttribs.getStringAttribs()) {
        const auto &inputValues = inputPair.second;
        auto &modelValues = modelStringAttribs[inputPair.first];
        modelValues.reserve(totalExtracted);

        for (auto i : extractedInstancesIdx) {
            modelValues.emplace_back(inputValues[i]);
        }
    }

    const auto &inputCNAMs = inputInstancesAttribs.getCNAMs();
    auto &CNAMs = m_attributes.getCNAMs();
    CNAMs.reserve(totalExtracted);
    for (auto i : extractedInstancesIdx) {
        CNAMs.emplace_back(inputCNAMs[i]);
    }
}

CDBGSModels::FindGSModelTexture::FindGSModelTexture(const std::string &GSModelTextureTileName,
                                                    osg::ref_ptr<osgDB::Archive> archive)
    : m_archive{archive}
    , m_GSModelTextureTileName{GSModelTextureTileName}
{}

CDBGSModels::FindGSModelTexture::~FindGSModelTexture() noexcept
{
    // OSG doesn't close the archive after ref_ptr is released, so we do it ourselves
    m_archive->close();
}

std::string CDBGSModels::FindGSModelTexture::findDataFile(const std::string &filename,
                                                          const osgDB::Options *options,
                                                          osgDB::CaseSensitivity caseSensitivity)
{
    // using existing search first
    std::string fileFound = FindFileCallback::findDataFile(filename, options, caseSensitivity);

    // if not found, try to look into the zip archive and return the texture name that will
    // map to zip entry name. If archive doesn't have it, then return empty string
    if (fileFound.empty()) {
        return searchArchiveTextureName(filename);
    }

    return fileFound;
}

osgDB::ReaderWriter::ReadResult CDBGSModels::FindGSModelTexture::readImage(const std::string &filename,
                                                                           const osgDB::Options *options)
{
    // look into archive first
    auto textureFile = m_archiveEntries.find(filename);
    if (textureFile != m_archiveEntries.end()) {
        auto imageRead = m_archive->readImage(textureFile->second, options);
        if (imageRead.validImage()) {
            osg::ref_ptr<osg::Image> image = imageRead.takeImage();
            image->setFileName(textureFile->first);
            return osgDB::ReaderWriter::ReadResult(image);
        }

        return imageRead;
    }

    return ReadFileCallback::readImage(filename, options);
}

std::string CDBGSModels::FindGSModelTexture::searchArchiveTextureName(const std::string &filename)
{
    if (!m_archive) {
        return "";
    }

    if (!m_archiveEntries.empty()) {
        for (const auto &entry : m_archiveEntries) {
            if (filename.find(entry.first) != std::string::npos) {
                return entry.first;
            }
        }

        return "";
    }

    std::string fileFound = "";
    std::vector<std::string> entryList;
    if (m_archive->getFileNames(entryList)) {
        for (const auto &entry : entryList) {
            std::string textureName;

            // +2 is because osg adds separator "/" at the beginning for each entry and there is a separator "_" near the end
            if (entry.size() > m_GSModelTextureTileName.size() + 2) {
                textureName = entry.substr(m_GSModelTextureTileName.size() + 2);
                m_archiveEntries.insert({textureName, entry});
            }

            if (filename.find(textureName) != std::string::npos) {
                fileFound = textureName;
            }
        }
    }

    return fileFound;
}

} // namespace CDBTo3DTiles
