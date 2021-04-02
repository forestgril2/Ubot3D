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

static const uint32_t kNumfloatsPerPositionAttribute = 3u;

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
							  aiProcess_CalcTangentSpace |
							  aiProcess_Triangulate |
//							  aiProcess_JoinIdenticalVertices
							  aiProcess_FixInfacingNormals
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
	updateData(TriangleGeometryData());
}

TriangleGeometry::TriangleGeometry(const TriangleGeometryData& data) : TriangleGeometry()
{
	updateData(data);
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

QVector<QVector<QVector3D> > TriangleGeometry::getTriangleIslands() const
{
	return _triangleIslands;
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

	reloadAssimpScene();

	if (!_scene)
		return;

	updateData(prepareDataFromAssimpScene());
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

void TriangleGeometry::reloadAssimpScene()
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
	clear();
}
uint32_t TriangleGeometry::calculateAndSetStride()
{
	static const uint32_t numfloatsPerColorAttribute = 4u;
	uint32_t stride = kNumfloatsPerPositionAttribute * sizeof(float);
	if (_hasColors)
	{
		stride += numfloatsPerColorAttribute * sizeof(float);
	}
	setStride(int(stride));
	return stride;
}

std::vector<float> TriangleGeometry::prepareColorTrianglesVertexData()
{// Calculate triangle vertices with colors and view them as Qt array.
	Chronograph chronograph(__FUNCTION__, true);

	const uint32_t numVertices = uint32_t(_data.vertices.size());
	std::vector<float> ret(uint32_t(stride())*numVertices);
	float* p = ret.data();

	std::set<uint32_t> overhangingIndices(_overhangingTriangleIndices.begin(), _overhangingTriangleIndices.end());

	for (uint32_t vertexIndex=0; vertexIndex<numVertices; ++vertexIndex)
	{
		const Vec3& vertex = _data.vertices[vertexIndex];
		auto setTriangleVertex = [this, &p](const Vec3& vertex) {
			*p++ = vertex.x();
			*p++ = vertex.y();
			*p++ = vertex.z();
			updateBounds(p-3);
		};

		auto setTriangleColor = [this, &p](const bool isVertexOverhanging) {
			const Eigen::Vector4f color = isVertexOverhanging ? _overhangColor : _baseModelColor;
			*p++ = color.x();
			*p++ = color.y();
			*p++ = color.z();
			*p++ = color.w();
		};

		auto conditionallyPushOverhangingTriangleVertex = [this](const Vec3& vertex, const bool isVertexOverhanging) {
			if (!isVertexOverhanging)
				return;
			_overhangingPoints.push_back(*reinterpret_cast<const QVector3D*>(&vertex));
//			_overhangingTriangleVertices.push_back();
		};

		setTriangleVertex(vertex);
		const bool isOverhangingVertex = overhangingIndices.find(vertexIndex) != overhangingIndices.end();
		setTriangleColor(isOverhangingVertex);
		conditionallyPushOverhangingTriangleVertex(vertex, isOverhangingVertex);
	}
	setBounds({_minBound.x, _minBound.y, _minBound.z}, {_maxBound.x, _maxBound.y,_maxBound.z});

	return ret;
}

void TriangleGeometry::collectOverhangingData(const std::vector<uint32_t>& overhangingTriangleIndices,
											  const std::vector<Vec3>& vertices)
{
	Chronograph chronograph(__FUNCTION__, true);

	_overhangingTriangleVertices.clear();
	_overhangingPoints.clear();

	_overhangingTriangleVertices.reserve(qsizetype(overhangingTriangleIndices.size() * 2));
	_overhangingPoints.reserve(qsizetype(overhangingTriangleIndices.size()));

	for (uint32_t index=0; index<overhangingTriangleIndices.size(); index+=3)
	{// For each triangle, collect triangle side vertices pair-wise.
		const QVector3D v0 = *reinterpret_cast<const QVector3D*>(&vertices[overhangingTriangleIndices[index   ]]);
		const QVector3D v1 = *reinterpret_cast<const QVector3D*>(&vertices[overhangingTriangleIndices[index +1]]);
		const QVector3D v2 = *reinterpret_cast<const QVector3D*>(&vertices[overhangingTriangleIndices[index +2]]);

		_overhangingTriangleVertices.push_back(v0);
		_overhangingTriangleVertices.push_back(v1);
		_overhangingTriangleVertices.push_back(v1);
		_overhangingTriangleVertices.push_back(v2);
		_overhangingTriangleVertices.push_back(v2);
		_overhangingTriangleVertices.push_back(v0);

		_overhangingPoints.push_back(v0);
		_overhangingPoints.push_back(v1);
		_overhangingPoints.push_back(v2);
	}

	emit overhangingTriangleVerticesChanged();
	emit overhangingPointsChanged();
}

QVector<TriangleGeometry*> TriangleGeometry::getSupportGeometries()
{
	Chronograph chronograph(__FUNCTION__, true);
//	calculateOverhangingData();

	return QVector<TriangleGeometry*>();
}

TriangleGeometryData TriangleGeometry::prepareDataFromAssimpScene()
{// Assimp vertices from STL are not unique, lets remove duplicates and remap indices.
	std::vector<Vec3> assimpVertices;
	std::vector<Vec3> assimpNormals;

	Helpers3D::getContiguousAssimpVerticesAndNormals(_scene, assimpVertices, assimpNormals);

	TriangleGeometryData returnData;
	const IndicesToVertices indicesToUniqueVertices =
			Helpers3D::mapIndicesToUniqueVertices(_scene, assimpVertices,      assimpNormals,
														  returnData.vertices, returnData.normals);
	returnData.indices = Helpers3D::getRemappedIndices(indicesToUniqueVertices, assimpVertices);
	return returnData;
}

void TriangleGeometry::calculateOverhangingData()
{
	Chronograph chronograph(__FUNCTION__, true);

	collectOverhangingData(_overhangingTriangleIndices, _data.vertices);
	TriangleConnectivity triangleConnectivity(_overhangingTriangleIndices);
	std::vector<TriangleIsland> triangleIslands = triangleConnectivity.calculateIslands();
	std::cout << " ### " << __FUNCTION__ << " triangleIslands.size():" << triangleIslands.size() << "," << "" << std::endl;

	_triangleIslands.clear();
	for (TriangleIsland& island : triangleIslands)
	{
		std::set<uint32_t> islandUniqueIndices;
		for (TriangleShared triangle : island.getTriangles())
		{
			islandUniqueIndices.insert(triangle->getVertexIndex(0));
			islandUniqueIndices.insert(triangle->getVertexIndex(1));
			islandUniqueIndices.insert(triangle->getVertexIndex(2));
		}

		QVector<QVector3D> islandPoints;
		islandPoints.reserve(uint32_t(islandUniqueIndices.size()));
		for (uint32_t index : islandUniqueIndices)
		{
			islandPoints.emplaceBack(*reinterpret_cast<const QVector3D*>(&_data.vertices[index]));
		}

		_triangleIslands.emplaceBack(Helpers3D::computeAlphaShape(islandPoints));
	}

	emit triangleIslandsChanged();
//	emit overhangingPointsChanged();
//	emit triangulationResultChanged();
}

void TriangleGeometry::updateData(const TriangleGeometryData& data)
{
	Chronograph chronograph(__FUNCTION__, true);

	if (data.vertices.empty())
		return;

	_data = data;


	const uint32_t stride = calculateAndSetStride();

	_overhangingTriangleIndices =
			Helpers3D::calculateOverhangingTriangleIndices(_data.vertices, _data.indices, _overhangAngleMax);

	calculateOverhangingData();

	setIndexData(QByteArray(reinterpret_cast<const char*>(_data.indices.data()),
							qsizetype(_data.indices.size() * sizeof(uint32_t))));

	setVertexData(QByteArray(reinterpret_cast<const char*>(prepareColorTrianglesVertexData().data()),
							 qsizetype(_data.vertices.size() * stride * sizeof(float))));

    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);

	addAttribute(QQuick3DGeometry::Attribute::ColorSemantic,
				 kNumfloatsPerPositionAttribute * sizeof(float),
				 QQuick3DGeometry::Attribute::F32Type);

	addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
				 0,
				 QQuick3DGeometry::Attribute::U32Type);

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

QT_END_NAMESPACE
