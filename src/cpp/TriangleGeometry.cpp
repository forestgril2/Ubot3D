#include <TriangleGeometry.h>

#include <QVector3D>
#include <QQuaternion>

#include <algorithm>
#include <cstdio>
#include <deque>
#include <iostream>
#include <iterator>
#include <stack>
#include <string>
#include <cfloat>

// ASSIMP LIBRARY INCLUDE
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>

//#include <D:\Projects\qt6-build-dev\qtbase\include\QtQuick3DUtils\6.0.0\QtQuick3DUtils\private\qssgbounds3_p.h>


//#include <D:\Projects\qt6\qtquick3d\src\runtimerender\graphobjects\qssgrendernode_p.h>
//#include <D:\Projects\qt6\qtquick3d\src\runtimerender\qssgrendermesh_p.h>
//#include <D:\Projects\qt6\qtquick3d\src\assetimport\qssgmeshutilities.cpp>

#ifdef _MSC_VER
#include <D:\Projects\qt6-a80e52\qtquick3d\src\runtimerender\qssgrenderray_p.h>
#include <D:\Projects\qt6-a80e52\qtquick3d\src\assetimport\qssgmeshbvhbuilder_p.h>
#include <QtQuick3DAssetImport/private/qssgmeshbvhbuilder_p.h>
#else
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderray_p.h>
#endif


#include <QtQuick3DUtils/private/qssgmeshbvh_p.h>
//qssgmeshbvhbuilder_p.h

//#include <D:\Projects\qt6\qtquick3d\src\runtimerender\graphobjects\qssgrendermodel_p.h>
//#include <D:\Projects\qt6\qtquick3d\src\utils\qssgoption_p.h>
//#include <D:\Projects\qt6\qtquick3d\src\runtimerender\graphobjects\qssgrenderlayer_p.h>


#include <clipper.hpp>

#include <Chronograph.h>
#include <Helpers3D.h>
#include <NaiveSlicer.h>
#include <PolygonTriangulation.h>
#include <TriangleConnectivity.h>

#include <fstream>

// To have QSG included
QT_BEGIN_NAMESPACE

#undef NDEBUG

using Matrix4 = Eigen::Matrix4f;

static const uint32_t kNumFloatsPerPositionAttribute = 3u;

static std::list<std::vector<uint32_t>> composeNodeRingsV1(const std::set<Edge>& edges)
{
	std::list<std::vector<uint32_t>> rings;
	std::set<uint32_t> nodes;
	for(auto const& e : edges)
	{
		nodes.insert(e.first);
		nodes.insert(e.second);
	}

	uint32_t node;
	while(!nodes.empty())
	{
		node = *nodes.begin();
		nodes.erase(node);

		std::stack<uint32_t> stack({node});

		std::map<uint32_t, uint32_t> path = {{node, node}};
		std::map<uint32_t, std::set<uint32_t>> visited; visited[node];

		while (!stack.empty())
		{
			const uint32_t topNode = stack.top();
			stack.pop();

			for(auto const& edge : edges)
			{
				if (edge.second != topNode && edge.first != topNode)
					continue;

				const uint32_t nbr = ((edge.first == topNode) ? edge.second : edge.first);

				if(visited.find(nbr) == visited.end())
				{
					path[nbr] = topNode;
					stack.push(nbr);
					visited[nbr] = {topNode};
				}
				else if(nbr == topNode)
				{
					rings.push_back({topNode});
				}
				else if(visited[topNode].find(nbr) == visited[topNode].end())
				{
					std::vector<uint32_t> ring({nbr, topNode});

					while(visited[nbr].find(path[topNode]) == visited[nbr].end())
					{
						if(path[topNode] == path[path[topNode]])
							break;

						ring.push_back(path[topNode]);
						path[topNode] = path[path[topNode]];
					}

					ring.push_back(path[topNode]);
					rings.push_back(ring);
					visited[nbr].insert(topNode);
				}
			}
		}

		for(const auto& p : path)
		{
			nodes.erase(p.first);
		}
	}

	return rings;
}

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

static std::vector<Edge> convertBoundaryToEdges(const std::vector<uint32_t>& indexBoundary)
{
	std::vector<Edge> converted;
	converted.reserve(indexBoundary.size());

	for (uint32_t i=1; i<indexBoundary.size(); ++i)
	{
		converted.emplace_back(Edge(indexBoundary[i-1], indexBoundary[i]));
	}
	converted.emplace_back(Edge(indexBoundary[indexBoundary.size() -1], indexBoundary[0]));
	return converted;
};

template <typename EdgeContainer>
static std::vector<Vec3> getEdgeVertices(const std::vector<Vec3>& vertices,
										 const EdgeContainer& edges,
										 const bool isDuplicatingSecondVertex)
{
	std::vector<Vec3> converted;
	converted.reserve((isDuplicatingSecondVertex ? 2 : 1) * edges.size());
	for (const Edge& edge : edges)
	{
		converted.emplace_back(vertices[edge.first]);
		if (!isDuplicatingSecondVertex)
			continue;
		converted.emplace_back(vertices[edge.second]);
	}
	return converted;
};

//template <typename FromType, typename ToType,
//		  template <typename FromType, typename ...> class FromContainer,
//		  template <typename ToType,   typename ...> class ToContainer>
//static ToContainer<ToType> map(const FromContainer<FromType>& elements,
//							   std::function<ToType(FromType)> convert,
//							   (void)(ToContainer::*inserter)(ToType))
//{
//	ToContainer<ToType> converted;
//	converted.reserve(int32_t(elements, elements.size()));

//	for (const FromType& el : elements)
//	{
//		converted.emplaceBack(*reinterpret_cast<const QVector3D*>(&v));
//	}
//	return converted;
//};
//converted.emplaceBack(*reinterpret_cast<const QVector3D*>(&v));

static QVector<QVector3D> convertToQVectors3D(const std::vector<Vec3>& vertices)
{
	QVector<QVector3D> converted;
	converted.reserve(int32_t(vertices.size()));
	std::for_each(vertices.begin(), vertices.end(), [&converted](const Vec3& v) {
		converted.emplaceBack(*reinterpret_cast<const QVector3D*>(&v));
	});
	return converted;
};

static std::vector<Vec3> generateRingEdgeVertices(const std::vector<Vec3>& vertices)
{
	std::vector<Vec3> ringEdgeVertices;
	ringEdgeVertices.reserve(2 * vertices.size());
	auto verticesIt = vertices.begin();
	ringEdgeVertices.emplace_back(*verticesIt++);
	std::for_each(verticesIt, vertices.end(), [&ringEdgeVertices](const Vec3& v) {
		ringEdgeVertices.emplace_back(v);
		ringEdgeVertices.emplace_back(ringEdgeVertices.back());
	});
	ringEdgeVertices.emplace_back(ringEdgeVertices.front());
	return ringEdgeVertices;
};

static std::list<std::vector<uint32_t>> composeNodeRingsV0(const std::set<Edge>& edges)
{//TODO: Currently will crash for non-closed boundaries.
 //      Extend to get graph rings and connections to/between rings.
	Chronograph chronograph(__FUNCTION__, true);

	// Having a set of TriangleIsland boundary Edges, defined as indices==nodes on their ends,
	// find rings by connecting edges by matching indices.
	std::list<std::vector<uint32_t>> rings;

	// Copy initial edges. We will remove each resolved/matched edge from it until empty.
	std::set<Edge> edgesLeft(edges);

	// We will also keep track of all edges left at their end-nodes.
	std::map<uint32_t, std::set<Edge>> edgesLeftAtNodes;
//	std::cout << " ### " << __FUNCTION__ << " All edges: " << std::endl;
	for (const Edge& edge : edgesLeft)
	{
//		std::cout << edge << std::endl;
		edgesLeftAtNodes[edge.first].insert(edge);
		edgesLeftAtNodes[edge.second].insert(edge);
	}

#ifndef NDEBUG
	for (const auto& [node, edges] : edgesLeftAtNodes)
	{// All these sets should start with a size of 2n, as every index is
	 // a node with as many "ingoing" as "outgoing" edges.
		if (0 != edges.size() % 2)
		{
			std::cout << " ### " << __FUNCTION__ << " ERROR! Impossible thing has happened: there are "
												 << edges.size() << " edges at one boundary node."
												 << std::endl;
			exit(-1);
		}
		assert(0 == edges.size() % 2);
	}
#endif

	// Initialize a boundary with a random starting edge and keep finding adjacent edges
	// in the same edge direction. Add back edge nodes to boundary, until the boundary is closed
	// This happens, when end-nodes match.
	while(!edgesLeft.empty())
	{
		Edge currEdge = *edgesLeft.begin();
		// The boundary at start are just front/back nodes of the starting edge.
		std::deque<uint32_t> boundary{currEdge.first, currEdge.second};
//		std::cout << " ### " << __FUNCTION__ << " Starting Edge:" << currEdge << std::endl;
		// Remove the edge from map entry for front node.
//		std::cout << " ### " << __FUNCTION__ << " edges at front node: " << std::endl;
		std::set<Edge>& edgesLeftAtFrontNode = edgesLeftAtNodes.at(boundary.front());
//		for(const Edge& edge: edgesLeftAtFrontNode)
//		{
//			std::cout << edge << std::endl;
//		}
//		std::cout << std::endl;
		edgesLeftAtFrontNode.erase(currEdge);

		while(true)
		{// Keep erasing edges from set and from maps until back boundary node matches the front boundary node.
			std::set<Edge>& edgesLeftAtCurrNode = edgesLeftAtNodes.at(boundary.back());
			const bool isBackNodeErased = edgesLeftAtCurrNode.erase(currEdge);
			assert(isBackNodeErased);

			// Consider the current edge fully visited, as it was removed from its both map-nodes.
			// Erase it from the edges left.
			const bool isCurrNodeErased = edgesLeft.erase(currEdge);
			assert(isCurrNodeErased);

			// Establish a new "current" edge.
//			std::cout << " ### " << __FUNCTION__ << " edges at current node: " << std::endl;
//			for(const Edge& edge: edgesLeftAtCurrNode)
//			{
//				std::cout << edge << std::endl;
//			}
//			std::cout << std::endl;

			Edge newEdge{0,0};
			for (const Edge& newEdgeCandidate : edgesLeftAtCurrNode)
			{// Make sure, the ordering is correct to accept a candidate.
				if (newEdgeCandidate.first != currEdge.second)
					continue;

				// Use the first, which has the same direction.
				newEdge = newEdgeCandidate;
				break;
			}
			assert(newEdge != Edge(0,0));
			assert(newEdge != currEdge);
			currEdge = newEdge;

			if (edgesLeftAtNodes.at(boundary.back()).empty())
			{// If all edges in this node have been visited, remove its whole map entry.
				edgesLeftAtNodes.erase(boundary.back());
			}


			if (currEdge.second == boundary.front())
			{// If new current edge second node matches boundary front node, remove the edge and break.
				bool wasLastEdgeRemovedFromFrontNode = edgesLeftAtFrontNode.erase(currEdge);
				assert(wasLastEdgeRemovedFromFrontNode);

				// Erase also from the set.
				bool wasLastEdgeRemovedFromSet = edgesLeft.erase(currEdge);
				assert(wasLastEdgeRemovedFromSet );

				if (edgesLeftAtFrontNode.empty())
				{// If all edges in this node have been visited, remove its whole map entry.
					edgesLeftAtNodes.erase(boundary.front());
				}

				break;
			}

			// Otherwise append a new back node to the boundary.
			boundary.push_back(currEdge.second);
		}
//		std::cout << " ### " << __FUNCTION__ << " RING CLOSED" << "" << "," << "" << std::endl;
		rings.emplace_back(std::vector<uint32_t>(boundary.begin(), boundary.end()));
	}

//	std::cout << " ### " << __FUNCTION__ << " returning ring of size: " << rings.size() << "," << "" << std::endl;
	return rings;
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
//							  aiProcess_CalcTangentSpace |
							  aiProcess_Triangulate |
//							  aiProcess_JoinIdenticalVertices
							  aiProcess_FixInfacingNormals |
							  aiProcess_ValidateDataStructure
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
	connect(this, &TriangleGeometry::zLevelChanged, this, &TriangleGeometry::onZLevelChanged);

	connect(this, &TriangleGeometry::isSupportGeneratedChanged, this, &TriangleGeometry::onIsSupportGeneratedChanged);
	connect(this, &TriangleGeometry::areRaftsGeneratedChanged, this, &TriangleGeometry::onAreRaftsGeneratedChanged);
	connect(this, &TriangleGeometry::raftOffsetChanged, this, &TriangleGeometry::onRaftOffsetChanged);

	// Re-draw debug data each time support or rafts are generated.
	connect(this, &TriangleGeometry::raftGeometriesChanged, this, &TriangleGeometry::triangleIslandBoundariesChanged);
	connect(this, &TriangleGeometry::supportGeometriesChanged, this, &TriangleGeometry::triangleIslandBoundariesChanged);
}

TriangleGeometry::TriangleGeometry(const TriangleGeometryData& data) : TriangleGeometry()
{
	updateData(data);
}

void TriangleGeometry::setSceneTransform(const QMatrix4x4& transform)
{
	if (qFuzzyCompare(_sceneTransform, transform))
		return;

	if (!qFuzzyCompare(_sceneTransform.column(3).z(), transform.column(3).z()))
	{
		emit zLevelChanged();
	}

	_sceneTransform = transform;

	emit sceneTransformChanged();
}

QVector<QVector3D> TriangleGeometry::getDebugTriangleEdges() const
{
	return _debugTriangleEdges;
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

void TriangleGeometry::onZLevelChanged()
{
	onIsSupportGeneratedChanged();
}

void TriangleGeometry::onIsSupportGeneratedChanged()
{
	if (!_isSupportGenerated && _supportGeometries.empty())
		return;

	if (_isSupportGenerated)
	{
		generateSupportGeometries();
	}
	else
	{
		clearSupportGeometries();
	}
}

void TriangleGeometry::onAreRaftsGeneratedChanged()
{
	if (!_areRaftsGenerated && _raftGeometries.empty())
		return;

	if (_areRaftsGenerated)
	{
		generateRaftGeometries();
	}
	else
	{
		clearRaftGeometries();
	}
}

void TriangleGeometry::onRaftOffsetChanged()
{
	if (!_areRaftsGenerated)
		return;

	generateRaftGeometries();
}

QVector3D TriangleGeometry::minBounds() const
{
	return QVector3D(_minBound.x, _minBound.y, _minBound.z);
}

QVector3D TriangleGeometry::maxBounds() const
{
	return QVector3D(_maxBound.x, _maxBound.y, _maxBound.z);
}

//void TriangleGeometry::setSceneTransform(const QMatrix4x4& transform)
//{
//	if (_sceneTransform == transform)
//		return;
//	_sceneTransform = transform;
//}

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
	clear();
}

std::vector<float> TriangleGeometry::prepareColorTrianglesVertexData()
{// Calculate triangle vertices with colors and view them as Qt array.
	Chronograph chronograph(__FUNCTION__, false);

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

		setTriangleVertex(vertex);

		if (!_hasColors)
			continue;

		const bool isOverhangingVertex = overhangingIndices.find(vertexIndex) != overhangingIndices.end();
		setTriangleColor(isOverhangingVertex);
	}
	setBounds({_minBound.x, _minBound.y, _minBound.z}, {_maxBound.x, _maxBound.y,_maxBound.z});

	return ret;
}

void TriangleGeometry::generateDebugTriangleEdges(const std::vector<Vec3>& vertices, const std::vector<uint32_t>& indices)
{
	Chronograph chronograph(__FUNCTION__, false);

	_debugTriangleEdges.clear();
	_debugTriangleEdges.reserve(qsizetype(indices.size() * 2));

	for (uint32_t index=0; index<indices.size(); index+=3)
	{// For each triangle, collect triangle side vertices pair-wise.
		const QVector3D v0 = *reinterpret_cast<const QVector3D*>(&vertices[indices[index   ]]);
		const QVector3D v1 = *reinterpret_cast<const QVector3D*>(&vertices[indices[index +1]]);
		const QVector3D v2 = *reinterpret_cast<const QVector3D*>(&vertices[indices[index +2]]);

		std::vector<std::reference_wrapper<const QVector3D>> verticesToPush{v0, v1, v1, v2, v2, v0};
		for(const QVector3D& v: verticesToPush)
		{
			_debugTriangleEdges.push_back(v);
		}
	}
	emit debugTriangleEdgesChanged();
}

std::vector<uint32_t> TriangleGeometry::collectFloorTriangleIndices(const float floorOffsetLimit, const Eigen::Matrix4f& transform)
{
	Chronograph chronograph(__FUNCTION__, false);
	std::vector<uint32_t> floorTriangleIndices;
	for (uint32_t triangleIndex=0; triangleIndex<_data.indices.size(); triangleIndex += 3)
	{
		auto isTransformedVertexAtFloor = [this, &transform, floorOffsetLimit](uint32_t triangleIndex) {
			const Vec3 v0 = _data.vertices[_data.indices[triangleIndex]];
			const Eigen::Vector3f transformedVertex0 = (transform * Eigen::Vector4f{v0.x(), v0.y(), v0.z(), 0}).head<3>();

			return std::abs(transformedVertex0.z() - minBounds().z()) <= floorOffsetLimit;
		};

		if (isTransformedVertexAtFloor(triangleIndex   ) ||
			isTransformedVertexAtFloor(triangleIndex +1) ||
			isTransformedVertexAtFloor(triangleIndex +2))
		{
			floorTriangleIndices.push_back(_data.indices[triangleIndex   ]);
			floorTriangleIndices.push_back(_data.indices[triangleIndex +1]);
			floorTriangleIndices.push_back(_data.indices[triangleIndex +2]);
		}
	}
	return floorTriangleIndices;
}

void TriangleGeometry::generateSupportGeometries()
{
	Chronograph chronograph(__FUNCTION__, true);
	generateDebugTriangleEdges(_data.vertices, _overhangingTriangleIndices);

	TriangleConnectivity triangleConnectivity(_overhangingTriangleIndices);
	std::vector<TriangleIsland> triangleIslands = triangleConnectivity.calculateIslands();

	_supportGeometries.clear();
	_triangleIslandBoundaries.clear();

	std::cout << " ### " << __FUNCTION__ << " triangleIslands.size():" << triangleIslands.size() << "," << "" << std::endl;

	for (const TriangleIsland& island : triangleIslands)
	{// Generate a support geometry for each overhanging triangle island.
		// TODO: Watch out! Hacking here - assuming, the model is snapped to floor.
		const std::vector<Vec3> edgeVertices = getEdgeVertices(_data.vertices,
															   island.getBoundaryEdges(),
															   /*isDuplicatingSecondVertex*/ true);
		_supportGeometries.push_back(Helpers3D::computeExtrudedPlanarMesh(island.getTriangleIndices(),
																		  _data.vertices,
																		  edgeVertices,
																		  _minBound.z));
		_triangleIslandBoundaries.emplace_back(convertToQVectors3D(edgeVertices));
	}

	emit supportGeometriesChanged();
}

void TriangleGeometry::clearSupportGeometries()
{
	_supportGeometries.clear();
	emit supportGeometriesChanged();
	emit raftGeometriesChanged();
}

Slicer::Layer TriangleGeometry::computeBottomLayer() const
{
	using namespace Slicer;
	NaiveSlicer slicer;
	const float kSliceDistFromFloor = 0.05f;
	const std::vector<Layer> layers = slicer.slice(*this, minBounds().z() + kSliceDistFromFloor);
	return !layers.empty() ? layers[0] : Slicer::Layer();
}

void TriangleGeometry::generateRaftGeometries()
{
	Chronograph chronograph(__FUNCTION__, true);
	_raftGeometries.clear();
	_triangleIslandBoundaries.clear();

	ClipperLib::Paths offsetPathsResult;
	ClipperLib::ClipperOffset clipperOffsetter;
	static const uint32_t kClipperIntMultiplier = 1000;

	for(auto const& polyline : _bottomLayer.polylines)
	{
		// Create polygon path for clipper and enlarge/shrink the boundary by a specified offset.
		ClipperLib::Path clipperBoundary;
		for (const auto& polylineVertex : polyline)
		{
			Vec3 vertex(polylineVertex.x(), polylineVertex.y(), polylineVertex.z());
			vertex *= kClipperIntMultiplier;
			ClipperLib::IntPoint clipperPoint(int32_t(std::round(vertex.x())), int32_t(std::round(vertex.y())));
			clipperBoundary << clipperPoint;
		}

		clipperOffsetter.AddPath(clipperBoundary, ClipperLib::jtRound, ClipperLib::etClosedPolygon);
	}
	clipperOffsetter.Execute(offsetPathsResult, kClipperIntMultiplier * double(_raftOffset));


	TriangleGeometryData raftData;
	std::list<std::vector<uint32_t>> offsetRings;
	uint32_t ringNodeCount = 0;

	for (const ClipperLib::Path& clipperPath : offsetPathsResult)
	{
		std::vector<Vec3> offsetBoundary;
		offsetRings.push_back({});
		offsetRings.back().reserve(clipperPath.size());
		std::vector<uint32_t>& boundaryNodes = offsetRings.back();

		offsetBoundary.reserve(clipperPath.size());
		for (const ClipperLib::IntPoint& clipperPoint : clipperPath)
		{
			offsetBoundary.emplace_back(Vec3{float(double(clipperPoint.X)/kClipperIntMultiplier),
											 float(double(clipperPoint.Y)/kClipperIntMultiplier),
											 _minBound.z + _raftHeight});

			raftData.vertices.emplace_back(offsetBoundary.back());
			boundaryNodes.push_back(ringNodeCount++);
		}

		_triangleIslandBoundaries.emplace_back(convertToQVectors3D(generateRingEdgeVertices(offsetBoundary)));
//		_raftGeometries.push_back(Helpers3D::computeExtrudedTriangleIsland(island, _data.vertices, 1, offsetBoundary));
	}

	PolygonTriangulation t(raftData.vertices, offsetRings);
	generateDebugTriangleEdges(t.getMesh().vertices, t.getMesh().indices);

	TriangleGeometryData data{t.getMesh().vertices, t.getMesh().vertices, t.getMesh().indices};

	_raftGeometries.push_back(Helpers3D::computeExtrudedPlanarMesh(t.getMesh().indices,
																   t.getMesh().vertices,
																   t.getMesh().vertices,
																   _minBound.z));

	emit raftGeometriesChanged();
}

void TriangleGeometry::clearRaftGeometries()
{
	_raftGeometries.clear();
	emit raftGeometriesChanged();
}

void TriangleGeometry::setSupportGenerated(bool isGenerated)
{
	if (isGenerated == _isSupportGenerated)
		return;

	_isSupportGenerated = isGenerated;

	emit isSupportGeneratedChanged(_isSupportGenerated);
}

bool TriangleGeometry::isSupportGenerated() const
{
	return _isSupportGenerated;
}

void TriangleGeometry::setRaftsGenerated(bool areGenerated)
{
	if (_areRaftsGenerated == areGenerated)
		return;

	_areRaftsGenerated = areGenerated;
	emit areRaftsGeneratedChanged(_areRaftsGenerated);
}

bool TriangleGeometry::areRaftsGenerated() const
{
	return _areRaftsGenerated;
}

float TriangleGeometry::getRaftOffset() const
{
	return _raftOffset;
}

void TriangleGeometry::setRaftOffset(float offset)
{
	if (approximatelyEqual(offset, _raftOffset, 0.00001f))
		return;

	_raftOffset = offset;
	emit raftOffsetChanged(_raftOffset);
}

float TriangleGeometry::getRaftHeight() const
{
	return _raftHeight;
}

void TriangleGeometry::setRaftHeight(float height)
{
	if (approximatelyEqual(height, _raftHeight, 0.00001f))
		return;

	_raftOffset = height;
	emit raftHeightChanged(_raftHeight);
}

QVector<TriangleGeometry*> TriangleGeometry::getSupportGeometries() const
{
	QVector<TriangleGeometry*> geometries;
	geometries.reserve(qsizetype(_supportGeometries.size()));
	std::for_each(_supportGeometries.begin(),
				  _supportGeometries.end(),
				  [&geometries](std::shared_ptr<TriangleGeometry> geometry) {
					  geometries.push_back(&*geometry);
				  });
	return geometries;
}

QVector<TriangleGeometry*> TriangleGeometry::getRaftGeometries() const
{
	QVector<TriangleGeometry*> geometries;
	geometries.reserve(qsizetype(_raftGeometries.size()));
	std::for_each(_raftGeometries.begin(),
				  _raftGeometries.end(),
				  [&geometries](std::shared_ptr<TriangleGeometry> geometry) {
		geometries.push_back(&*geometry);
	});
	return geometries;
}

QVector<QVector<QVector3D> > TriangleGeometry::getTriangleIslandBoundaries() const
{
	return _triangleIslandBoundaries;
}

TriangleGeometryData TriangleGeometry::prepareDataFromAssimpScene()
{// Assimp vertices from STL are not unique, lets remove duplicates and remap indices.
	Chronograph chronograph(__FUNCTION__, true);

	std::vector<Vec3> assimpVertices;
	std::vector<Vec3> assimpNormals;
	Helpers3D::getContiguousAssimpVerticesAndNormals(_scene, assimpVertices, assimpNormals);

	TriangleGeometryData returnData;
	const IndicesToVertices indicesToUniqueVertices =
			Helpers3D::mapIndicesToUniqueVerticesAndNormals(assimpVertices,      assimpNormals,
															returnData.vertices, returnData.normals);
	returnData.indices = Helpers3D::getRemappedIndices(indicesToUniqueVertices, assimpVertices);
	return returnData;
}

void TriangleGeometry::updateData(const TriangleGeometryData& data)
{
	Chronograph chronograph(__FUNCTION__, false);

	if (data.vertices.empty())
		return;
	_data = data;

	const uint32_t stride = GeometryVertexStrideData::getStride(_hasColors);
	setStride(static_cast<int>(stride));

	// TODO: These should be redetected each time scene tranform changes. Add a signal and recalculate.
	_overhangingTriangleIndices = // These are needed to add color to triangles and generate support geometries.
			Helpers3D::calculateOverhangingTriangleIndices(_data.vertices, _data.indices, _overhangAngleMax);

	generateDebugTriangleEdges(_data.vertices, _overhangingTriangleIndices);

	// Watch out! TriangleGeometryData _data.vertices has different byte size than its size multiplied by stride:
	// prepareColorTrianglesVertexData() does the job of converting simple Vec3 arrays to vertices+colors.
	setIndexData(QByteArray(reinterpret_cast<const char*>(_data.indices.data()),
							qsizetype(_data.indices.size() * sizeof(uint32_t))));
	setVertexData(QByteArray(reinterpret_cast<const char*>(prepareColorTrianglesVertexData().data()),
							 qsizetype(_data.vertices.size() * stride)));

	setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

	addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
				 0,
				 QQuick3DGeometry::Attribute::F32Type);

	addAttribute(QQuick3DGeometry::Attribute::ColorSemantic,
				 kNumFloatsPerPositionAttribute * sizeof(float),
				 QQuick3DGeometry::Attribute::F32Type);

	addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
				 0,
				 QQuick3DGeometry::Attribute::U32Type);

	buildIntersectionData();
	_bottomLayer = computeBottomLayer();
	emit modelLoaded();
}

void TriangleGeometry::buildIntersectionData()
{
	delete _intersectionData;

	QSSGRef<QSSGMeshUtilities::QSSGMeshBuilder> meshBuilder = QSSGMeshUtilities::QSSGMeshBuilder::createMeshBuilder();
	QSSGMeshUtilities::MeshData meshData;

	QByteArray vertexBufferCopy;
	meshData.m_vertexBuffer.resize(vertexData().size());
	memcpy(meshData.m_vertexBuffer.data(), vertexData().data(), size_t(vertexData().size()));

	QByteArray indexBufferCopy;
	meshData.m_indexBuffer.resize(indexData().size());
	memcpy(meshData.m_indexBuffer.data(), indexData().data(), size_t(indexData().size()));

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

//	auto &outputMesh = meshBuilder->getMesh();
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
		QSSGRenderRay::intersectWithBVHTriangles(rayData, _intersectionData->triangles, 0, int(_intersectionData->triangles.size()));

	if (intersections.size() > 0)
	{
		std::vector<float> distancesToOrigin;
		distancesToOrigin.reserve(size_t(intersections.size()));
		std::transform(intersections.begin(),
					   intersections.end(),
					   std::back_inserter(distancesToOrigin),
					   [&origin](const QSSGRenderRay::IntersectionResult& intersection) {
									   return (origin - intersection.scenePosition).lengthSquared();
								 });

		const QVector3D intersection =
				intersections[static_cast<uint32_t>(
					std::distance(distancesToOrigin.begin(),std::min_element(distancesToOrigin.begin(),distancesToOrigin.end()))
				)].scenePosition;

		return QVariantMap{{"intersection", intersection}, {"isHit", true}};
	}

	return noHit;
}

QT_END_NAMESPACE
