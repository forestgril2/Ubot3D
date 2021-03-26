#include <TriangleGeometry.h>

#include <QVector3D>
#include <QQuaternion>

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <string>

// ASSIMP LIBRARY INCLUDE
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>

//#include <D:\Projects\qt6-build-dev\qtbase\include\QtQuick3DUtils\6.0.0\QtQuick3DUtils\private\qssgbounds3_p.h>


//#include <D:\Projects\qt6\qtquick3d\src\runtimerender\graphobjects\qssgrendernode_p.h>
//#include <D:\Projects\qt6\qtquick3d\src\runtimerender\qssgrendermesh_p.h>
//#include <D:\Projects\qt6\qtquick3d\src\assetimport\qssgmeshutilities.cpp>
#include <D:\Projects\qt6-a80e52\qtquick3d\src\runtimerender\qssgrenderray_p.h>
#include <D:\Projects\qt6-a80e52\qtquick3d\src\assetimport\qssgmeshbvhbuilder_p.h>
//#include <D:\Projects\qt6\qtquick3d\src\runtimerender\graphobjects\qssgrendermodel_p.h>
//#include <D:\Projects\qt6\qtquick3d\src\utils\qssgoption_p.h>
//#include <D:\Projects\qt6\qtquick3d\src\runtimerender\graphobjects\qssgrenderlayer_p.h>

#include <Chronograph.h>
#include <Helpers3D.h>
#include <TriangleConnectivity.h>

#include <fstream>

// To have QSG included
QT_BEGIN_NAMESPACE

void assimpErrorLogging(const std::string&& pError)
{
	std::cout << __FUNCTION__ << " : " << pError << std::endl;
}

static void assimpLogScene(const aiScene* scene)
{
	std::cout << "assimpLogScene(), numMeshes:                       " << scene->mNumMeshes << std::endl;
	std::cout << "assimpLogScene(), scene->mMeshes[0]->mNumFaces:    " << scene->mMeshes[0]->mNumFaces << std::endl;
	std::cout << "assimpLogScene(), scene->mMeshes[0]->mNumVertices: " << scene->mMeshes[0]->mNumVertices << std::endl;
}

void TriangleGeometry::updateBounds(const float* vertexMatrixXCoord)
{
	_minBound.x = (std::min(_minBound.x, *vertexMatrixXCoord));
	_maxBound.x = (std::max(_maxBound.x, *vertexMatrixXCoord));
	++vertexMatrixXCoord;
	_minBound.y = (std::min(_minBound.y, *vertexMatrixXCoord));
	_maxBound.y = (std::max(_maxBound.y, *vertexMatrixXCoord));
	++vertexMatrixXCoord;
	_minBound.z = (std::min(_minBound.z, *vertexMatrixXCoord));
	_maxBound.z = (std::max(_maxBound.z, *vertexMatrixXCoord));
}

void TriangleGeometry::logBounds()
{
	std::cout << " ### aiScene minBound(x,y,z): [" << _minBound.x << "," << _minBound.y << "," << _minBound.z << "]" << std::endl;
	std::cout << " ### aiScene maxBound(x,y,z): [" << _maxBound.x << "," << _maxBound.y << "," << _maxBound.z << "]" << std::endl;
}

void TriangleGeometry::updateAllMeshBounds(const aiScene* scene, const unsigned meshIndex)
{
	const unsigned numMeshVertices = scene->mMeshes[meshIndex]->mNumVertices;
	_minBound = _minFloatBound;
	_maxBound = _maxFloatBound;
	for (unsigned i = 0; i < numMeshVertices; ++i)
	{
		updateBounds(&(scene->mMeshes[meshIndex]->mVertices[i].x));
	}
//	logBounds();
}

bool TriangleGeometry::importModelFromFile(const std::string& pFile)
{
	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	_scene = importer.ReadFile(pFile,
							  aiProcess_CalcTangentSpace
//							  aiProcess_Triangulate |
//							  aiProcess_JoinIdenticalVertices
//							  aiProcess_FixInfacingNormals |
//							  aiProcess_ImproveCacheLocality
//							  aiProcess_SortByPType
							   );
	// If the import failed, report it
	if(!_scene)
	{
		assimpErrorLogging(std::string(importer.GetErrorString()));
		return false;
	}
	assimpLogScene(_scene);

	_isAssimpReadDone = true;
	// We're done. Everything will be cleaned up by the importer destructor
	return true;
}

TriangleGeometry::TriangleGeometry() :
	_maxFloatBound(aiVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX)),
	_minFloatBound(aiVector3D(FLT_MAX, FLT_MAX, FLT_MAX)),
	_maxBound(aiVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX)),
	_minBound(aiVector3D(FLT_MAX, FLT_MAX, FLT_MAX))
{
	updateData();
}

void TriangleGeometry::exportModelToSTL(const QString& filePath)
{
//	Helpers3D::exportModelsToSTL(filePath);
}



QVariantMap TriangleGeometry::getPick(const QVector3D& origin,
									  const QVector3D& direction,
									  const QMatrix4x4& globalTransform)
{
	Chronograph chronograph(__FUNCTION__, false);
	QVariantMap noHit{{"intersection", QVector3D()}, {"isHit", false}};
	if (vertexData().size() == 0)
	{
		std::cout << " ### " << __FUNCTION__ << " WARNING vertex buffer empty, returning empty pick" << std::endl;
		return noHit;
	}

	QSSGRenderRay hitRay(origin, direction);

//	qDebug() << " ### globalTransform: " << globalTransform;

	// From tst_picking.cpp: void picking::test_picking()
//	QSSGRenderLayer dummyLayer;
//	const QSSGRenderNode* node === ???;
//	const QSSGRenderModel &model = static_cast<const QSSGRenderModel &>(node);
//	const auto &globalTransform = model.globalTransform;

	QSSGRenderRay::RayData rayData = QSSGRenderRay::createRayData(globalTransform, hitRay);



	QVector<QSSGRenderRay::IntersectionResult> intersections =
		QSSGRenderRay::intersectWithBVHTriangles(rayData, _intersectionData->triangles, 0, _intersectionData->triangles.size());

	if (intersections.size() > 0)
	{
		std::vector<float> distancesToOrigin;
		distancesToOrigin.reserve(intersections.size());
		std::transform(intersections.begin(),
					   intersections.end(),
					   std::back_inserter(distancesToOrigin),
					   [&origin](const QSSGRenderRay::IntersectionResult& intersection) {
									   return (origin - intersection.scenePosition).lengthSquared();
								 });

		const QVector3D intersection = intersections[static_cast<uint32_t>(
										   std::distance(distancesToOrigin.begin(),
														 std::min_element(distancesToOrigin.begin(),
																		  distancesToOrigin.end())))].scenePosition;

		return QVariantMap{{"intersection", intersection}, {"isHit", true}};
	}

	return noHit;
}

QVector<QVector3D> TriangleGeometry::getOverhangingTriangleVertices() const
{
	return _overhangingTriangleVertices;
}

QVector<QVector3D> TriangleGeometry::getOverhangingPoints() const
{
	return _overhangingPoints;
}

QVector<QVector3D> TriangleGeometry::getTriangulationResult() const
{
	return _triangulationResult;
}

const aiScene* TriangleGeometry::getAssimpScene() const
{
	return _scene;
}

QString TriangleGeometry::getInputFile() const
{
	return _inputFile;
}

void TriangleGeometry::setInputFile(const QString& url)
{
	if (url == _inputFile)
		return;

	_inputFile = url;
	_isAssimpReadDone = false;
	updateData();
	update();
}

void TriangleGeometry::setNormals(bool enable)
{
	if (_hasNormals == enable)
		return;

	_hasNormals = enable;
    emit normalsChanged();
    updateData();
    update();
}

void TriangleGeometry::setNormalXY(float xy)
{
	if (_normalXY == xy)
        return;

	_normalXY = xy;
    emit normalXYChanged();
    updateData();
    update();
}

void TriangleGeometry::setUV(bool enable)
{
	if (_hasUV == enable)
        return;

	_hasUV = enable;
    emit uvChanged();
    updateData();
    update();
}

void TriangleGeometry::setUVAdjust(float f)
{
	if (_uvAdjust == f)
        return;

	_uvAdjust = f;
    emit uvAdjustChanged();
    updateData();
    update();
}

void TriangleGeometry::setBounds(const QVector3D& min, const QVector3D& max)
{
	QQuick3DGeometry::setBounds(min, max);

	_minBound = {min.x(), min.y(), min.z()};
	_maxBound = {max.x(), max.y(), max.z()};

	emit boundsChanged();
}

void TriangleGeometry::setMinBounds(const QVector3D& minBounds)
{
	setBounds(minBounds, maxBounds());
}

void TriangleGeometry::setMaxBounds(const QVector3D& maxBounds)
{
	setBounds(minBounds(), maxBounds);
}

QVector3D TriangleGeometry::minBounds() const
{
	return QVector3D(_minBound.x, _minBound.y, _minBound.z);
}

QVector3D TriangleGeometry::maxBounds() const
{
	return QVector3D(_maxBound.x, _maxBound.y, _maxBound.z);
}

bool TriangleGeometry::isPicked() const
{
	return _isPicked;
}

void TriangleGeometry::setPicked(const bool isPicked)
{
	if (_isPicked == isPicked)
		return;

	_isPicked = isPicked;
	isPickedChanged();
}

void TriangleGeometry::reloadSceneIfNecessary()
{
	if (!_isAssimpReadDone)
	{
		if (_inputFile.isEmpty())
		{
			clear();
			return;
		}

		if (!importModelFromFile(_inputFile.toStdString().c_str()))
			return;

		updateAllMeshBounds(_scene);

//		assimpLogScene(scene);
		setBounds({_minBound.x, _minBound.y, _minBound.z}, {_maxBound.x, _maxBound.y,_maxBound.z});
		emit modelLoaded();
	}
	clear();
}
void TriangleGeometry::updateData()
{
	Chronograph chronograph(__FUNCTION__, false);
	reloadSceneIfNecessary();

	if (!_scene)
		return;

	_overhangingTriangleVertices.clear();
	_overhangingPoints.clear();

	const uint32_t numfloatsPerPositionAttribute = 3u;
	const uint32_t numfloatsPerColorAttribute = 4u;
	uint32_t stride = numfloatsPerPositionAttribute * sizeof(float);
	if (_hasColors)
	{
		stride += numfloatsPerColorAttribute * sizeof(float);
	}
	const uint32_t floatsPerStride = stride / sizeof(float);

//    if (m_hasNormals)
//    {
//        stride += 3 * sizeof(float);
//    }
//    if (m_hasUV)
//    {
//        stride += 2 * sizeof(float);
//    }

	uint32_t numMeshFaces = 0;
	uint32_t numVertices = 0;
	for (uint32_t m=0; m<_scene->mNumMeshes; ++m)
	{
		numMeshFaces += _scene->mMeshes[m]->mNumFaces;
		numVertices += _scene->mMeshes[m]->mNumVertices;
	}

    QByteArray v;
	v.resize(numVertices * stride);
	float* p = reinterpret_cast<float*>(v.data());

	QByteArray indices;
	indices.resize(3 * numMeshFaces * sizeof(uint32_t));
	uint32_t* pi = reinterpret_cast<uint32_t*>(indices.data());

    // a triangle, front face = counter-clockwise
//    *p++ = -1.0f; *p++ = -1.0f; *p++ = 0.0f;
//    if (m_hasNormals) {
//        *p++ = m_normalXY; *p++ = m_normalXY; *p++ = 1.0f;
//    }
//    if (m_hasUV) {
//        *p++ = 0.0f + m_uvAdjust; *p++ = 0.0f + m_uvAdjust;
//    }
//    *p++ = 1.0f; *p++ = -1.0f; *p++ = 0.0f;
//    if (m_hasNormals) {
//        *p++ = m_normalXY; *p++ = m_normalXY; *p++ = 1.0f;
//    }
//    if (m_hasUV) {
//        *p++ = 1.0f - m_uvAdjust; *p++ = 0.0f + m_uvAdjust;
//    }
//    *p++ = 0.0f; *p++ = 1.0f; *p++ = 0.0f;
//    if (m_hasNormals) {
//        *p++ = m_normalXY; *p++ = m_normalXY; *p++ = 1.0f;
//    }
//    if (m_hasUV) {
//        *p++ = 1.0f - m_uvAdjust; *p++ = 1.0f - m_uvAdjust;
//    }

// a triangle, front face = counter-clockwise

	// TODO: We should probably perform subsequent mesh node transformations here.
	for (uint32_t m=0; m<_scene->mNumMeshes; ++m)
	{
		const uint32_t numFacesInThisMesh = _scene->mMeshes[m]->mNumFaces;
		for (uint32_t f=0; f<numFacesInThisMesh; ++f)
		{
			const aiFace& face = _scene->mMeshes[m]->mFaces[f];

			if (face.mNumIndices != 3)
			{
				std::cout << "#### WARNING! face.mNumIndices != 3, but " << face.mNumIndices << std::endl;
			}

//			const aiVector3D boundDiff = _maxBound-_minBound;

			auto isTriangleNormalOverhanging = [](const Vec3 no, const Vec3 n1, const Vec3 n2, float maxOverhangAngle)
			{// Judge based on an average of three triangle vertex normals.
				Vec3 normal = (no + n1 + n2)*0.3333333;
				return normal.dot(Vec3{0,0,1}) < std::cosf(float(M_PI) - maxOverhangAngle);
			};

			auto setTriangleVertex = [this, &p, &pi, floatsPerStride, m](uint32_t vertexIndex, const bool isVertexOverhanging) {
				const aiVector3D vertex = _scene->mMeshes[m]->mVertices[vertexIndex];
				*p++ = vertex.x;
				*p++ = vertex.y;
				*p++ = vertex.z;

				// Set color
				const Eigen::Vector4f color = isVertexOverhanging ? _overhangColor : _baseModelColor;
				if (isVertexOverhanging)
				{
					_overhangingTriangleVertices.push_back(*reinterpret_cast<const QVector3D*>(&vertex));
					_overhangingPoints.push_back(_overhangingTriangleVertices.back());
				}
				*p++ = color.x();
				*p++ = color.y();
				*p++ = color.z();
				*p++ = color.w();

				*pi++ = vertexIndex;

				updateBounds(p-floatsPerStride);
			};

			auto setTriangleVertexesAndOverhangingTriangles = [this, setTriangleVertex, isTriangleNormalOverhanging, &face, m](){
				//TODO: All this is a little messy, come back here and make it better.
				const Vec3 n0 = *reinterpret_cast<Vec3*>(&(_scene->mMeshes[m]->mNormals[face.mIndices[0]]));
				const Vec3 n1 = *reinterpret_cast<Vec3*>(&(_scene->mMeshes[m]->mNormals[face.mIndices[1]]));
				const Vec3 n2 = *reinterpret_cast<Vec3*>(&(_scene->mMeshes[m]->mNormals[face.mIndices[2]]));

				const bool isTriangleOverhanging = isTriangleNormalOverhanging(n0, n1, n2, _overhangAngleMax);

				//TODO: I mean this: _overhangingTriangleVertices are push_back'ed inside setTriangleVertex(), which looks nice - not.
				// First side: vertex0 to vertex1
				setTriangleVertex(face.mIndices[0], isTriangleOverhanging);
				setTriangleVertex(face.mIndices[1], isTriangleOverhanging);

				// Second side: vertex1 to vertex2
				if (isTriangleOverhanging)
				{
					_overhangingTriangleVertices.push_back(_overhangingTriangleVertices.back());
				}
				setTriangleVertex(face.mIndices[2], isTriangleOverhanging);

				if (!isTriangleOverhanging)
					return;

				// Third side: vertex2 to vertex0
				_overhangingTriangleVertices.push_back(_overhangingTriangleVertices.back());
				_overhangingTriangleVertices.push_back(*reinterpret_cast<QVector3D*>(&(_scene->mMeshes[m]->mVertices[face.mIndices[0]])));

				//TODO: BTW, this should be the main way to define/collect triangles - almost always by indices (sometimes by stripes, fans, maybe).
				_overhangingTriangleIndices.push_back(face.mIndices[0]);
				_overhangingTriangleIndices.push_back(face.mIndices[1]);
				_overhangingTriangleIndices.push_back(face.mIndices[2]);
			};

			setTriangleVertexesAndOverhangingTriangles();
		}
	}
	setBounds({_minBound.x, _minBound.y, _minBound.z}, {_maxBound.x, _maxBound.y,_maxBound.z});

	std::vector<TriangleIsland> triangleIslands = TriangleConnectivity(_overhangingTriangleIndices).calculateIslands();
	std::cout << " ### " << __FUNCTION__ << " triangleIslands.size():" << triangleIslands.size() << "," << "" << std::endl;
//	for (TriangleIsland& island : triangleIslands)
//	{
//		std::cout << " ### " << __FUNCTION__ << " island size:" << island.getTriangles().size() << "," << "" << std::endl;
////		for (auto triangleSetIt : island.getTriangles())
////		{
////			std::cout << " ### " << __FUNCTION__ << " :" << (*triangleSetIt).getNeighbourCount() << "," << "" << std::endl;
////		}
//	}


	_triangulationResult = Helpers3D::computeAlphaShape(_overhangingPoints);

	// Inform, that overhangings data was modified.
	emit overhangingTriangleVerticesChanged();
	emit overhangingPointsChanged();
	emit triangulationResultChanged();

    setVertexData(v);
	setIndexData(indices);
	setStride(int(stride));

    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);

	addAttribute(QQuick3DGeometry::Attribute::ColorSemantic,
				 numfloatsPerPositionAttribute * sizeof(float),
				 QQuick3DGeometry::Attribute::F32Type);

	addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
				 0,
				 QQuick3DGeometry::Attribute::U32Type);

//    if (m_hasNormals) {
//        addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,
//                     3 * sizeof(float),
//                     QQuick3DGeometry::Attribute::F32Type);
//    }

//    if (m_hasUV) {
//        addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic,
//                     m_hasNormals ? 6 * sizeof(float) : 3 * sizeof(float),
//                     QQuick3DGeometry::Attribute::F32Type);
//    }

	buildIntersectionData();
}

void TriangleGeometry::buildIntersectionData()
{
	delete _intersectionData;

	QSSGRef<QSSGMeshUtilities::QSSGMeshBuilder> meshBuilder = QSSGMeshUtilities::QSSGMeshBuilder::createMeshBuilder();
	QSSGMeshUtilities::MeshData meshData;

	QByteArray vertexBufferCopy;
	meshData.m_vertexBuffer.resize(vertexData().size());
	memcpy(meshData.m_vertexBuffer.data(), vertexData().data(), vertexData().size());

	QByteArray indexBufferCopy;
	meshData.m_indexBuffer.resize(indexData().size());
	memcpy(meshData.m_indexBuffer.data(), indexData().data(), indexData().size());

//	meshData.m_vertexBuffer = vertexBufferCopy;
//	meshData.m_indexBuffer = indexBuffer();
//	qDebug() << " ### indexBuffer().size():" << indexData().size();
//	qDebug() << " ### vertexBuffer().size():" << vertexData().size();
//	qDebug() << " ### attributeCount():" << attributeCount();
//	qDebug() << " ### stride():" << stride();
	meshData.m_stride = stride();
	meshData.m_attributeCount = attributeCount();
	// TODO: Hacking... do it properly.... if needed :)
	meshData.m_attributes[_indexAttributeIndex].semantic = QSSGMeshUtilities::MeshData::Attribute::IndexSemantic;
	meshData.m_attributes[_indexAttributeIndex].offset = 0;
	meshData.m_attributes[_indexAttributeIndex].componentType = QSSGMeshUtilities::MeshData::Attribute::U32Type;

//	for (unsigned i = 0; i < meshData.m_attributeCount; ++i)
//	{
//		meshData.m_attributes[i].semantic = QSSGMeshUtilities::MeshData::Attribute::PositionSemantic;
//		meshData.m_attributes[i].offset = 0;
//		meshData.m_attributes[i].componentType = QSSGMeshUtilities::MeshData::Attribute::F32Type;
//	}
	QString error;
	QSSGMeshUtilities::Mesh* mesh = meshBuilder->buildMesh(meshData, error, QSSGBounds3(minBounds(), maxBounds()));

//	// Return the current mesh.  This is only good for this function call, item may change or be
//    // released
//    // due to any further function calls.
//    Mesh &getMesh() override

	if (!error.isEmpty())
	{
		qDebug() << " ### error:" << error;
	}
//	mesh->m_subsets = m_subsets;
//	mesh->m_joints = m_joints;

	auto &outputMesh = meshBuilder->getMesh();
	QSSGMeshBVHBuilder meshBVHBuilder(mesh);
	_intersectionData = meshBVHBuilder.buildTree();
}

QT_END_NAMESPACE
