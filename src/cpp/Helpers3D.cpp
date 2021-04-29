#include "Helpers3D.h"

#include <iostream>

#include <QVector3D>
#include <QQuaternion>
#include <QVariantMap>

#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/cexport.h>
#include <assimp/SceneCombiner.h>

#include <glm/mat4x4.hpp>

#include <TriangleGeometry.h>
#include <TriangleConnectivity.h>
#include <Chronograph.h>

#define USE_CGAL
#ifdef USE_CGAL
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Alpha_shape_2.h>
#include <CGAL/Alpha_shape_vertex_base_2.h>
#include <CGAL/Alpha_shape_face_base_2.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <CGAL/algorithm.h>
#include <CGAL/assertions.h>

#include <fstream>
#include <iostream>
#include <list>
#include <vector>

//#include <CGAL/draw_triangulation_2.h>

//#include <CGAL/Simple_cartesian.h>
//#include <CGAL/Surface_mesh.h>

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Triangulation = CGAL::Triangulation_2<K>;
using Vertex_circulator = Triangulation::Vertex_circulator;
using TPoint = Triangulation::Point;

//using C = CGAL::Simple_cartesian<float>;
//using Mesh = CGAL::Surface_mesh<C::Point_3>;
//using vertex_descriptor = Mesh::Vertex_index;
//using face_descriptor = Mesh::Face_index ;


typedef K::FT                                                FT;
typedef K::Point_2                                           Point;
typedef K::Segment_2                                         Segment;
typedef CGAL::Alpha_shape_vertex_base_2<K>                   Vb;
typedef CGAL::Alpha_shape_face_base_2<K>                     Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb>          Tds;
typedef CGAL::Delaunay_triangulation_2<K,Tds>                Triangulation_2;
typedef CGAL::Alpha_shape_2<Triangulation_2>                 Alpha_shape_2;
typedef Alpha_shape_2::Alpha_shape_edges_iterator            Alpha_shape_edges_iterator;
template <class OutputIterator>
#endif

#undef NDEBUG

static void alpha_edges(const Alpha_shape_2& A, OutputIterator out)
{
  Alpha_shape_edges_iterator it = A.alpha_shape_edges_begin(),
							 end = A.alpha_shape_edges_end();
  for( ; it!=end; ++it)
	*out++ = A.segment(*it);
}

bool Helpers3D::vertexLess(const Vec3& a, const Vec3& b)
{
	if (definitelyLessThan(a.z(), b.z(), FLT_MIN))
		return true;
	if (definitelyGreaterThan(a.z(), b.z(), FLT_MIN))
		return false;

	if (definitelyLessThan(a.y(), b.y(), FLT_MIN))
		return true;
	if (definitelyGreaterThan(a.y(), b.y(), FLT_MIN))
		return false;

	if (definitelyLessThan(a.x(), b.x(), FLT_MIN))
		return true;

	return false;
}

std::vector<Vec3> Helpers3D::computeAlphaShapeSegments(const std::vector<Vec3>& points, float floorLevel)
{
	std::vector<Vec3> result;
	if (points.size() == 0)
		return result;

#ifdef USE_CGAL

	std::vector<Point> points2 = getCgalPoints2<Point>(points);
	Alpha_shape_2 A(points2.begin(), points2.end(),
					FT(0.1),
					Alpha_shape_2::GENERAL);
	const double optimalAlpha = *A.find_optimal_alpha(1);
	//Recalculate with optimal alpha
	Alpha_shape_2 B(points2.begin(), points2.end(),
					FT(optimalAlpha),
					Alpha_shape_2::GENERAL);
	std::vector<Segment> segments;
	alpha_edges(B, std::back_inserter(segments));
//	std::cout << "Alpha Shape computed" << std::endl;
//	std::cout << segments.size() << " alpha shape edges" << std::endl;

	std::for_each(segments.begin(), segments.end(), [&result, &floorLevel](const Segment& s) {
		result.push_back({static_cast<float>(s[0].x()), static_cast<float>(s[0].y()), floorLevel});
		result.push_back({static_cast<float>(s[1].x()), static_cast<float>(s[1].y()), floorLevel});
	});
//	std::for_each(segments.begin(), segments.end(), [&result](const Segment& s) {
//		result.push_back({static_cast<float>(s[0].x()), static_cast<float>(s[0].y()), 0});
//	});
#endif
	return result;
}

std::vector<Vec3> Helpers3D::computeConvexHull(const std::vector<Vec3>& points)
{
std::vector<Vec3> result;
#ifdef USE_CGAL
//  std::ifstream in("data/triangulation_prog1.cin");
//  std::istream_iterator<Point> points2.begin(in);
//  std::istream_iterator<Point> points2.end;

  std::vector<TPoint> points2 = getCgalPoints2<TPoint, Vec3>(points);

  Triangulation t;
  t.insert(points2.begin(), points2.end());
  Vertex_circulator vc = t.incident_vertices(t.infinite_vertex());
  Vertex_circulator done(vc);
  if (vc != nullptr)
  {
	  do
	  {
//		  std::cout << vc->point() << std::endl;
		  result.push_back({static_cast<float>(vc->point().x()), static_cast<float>(vc->point().y()), 0});
	  }
	  while(++vc != done);
  }
  result.push_back(result.front());
#endif
  return result;
}


int Helpers3D::createCgalMesh()
{
#ifdef USE_CGAL
//  Mesh m;
//  // Add the points as vertices
//  vertex_descriptor u = m.add_vertex(C::Point_3(0,1,0));
//  vertex_descriptor v = m.add_vertex(C::Point_3(0,0,0));
//  vertex_descriptor w = m.add_vertex(C::Point_3(1,1,0));
//  vertex_descriptor x = m.add_vertex(C::Point_3(1,0,0));
//  m.add_face(u,v,w);
//  face_descriptor f = m.add_face(u,v,x);
//  if(f == Mesh::null_face())
//  {
//	std::cerr<<"The face could not be added because of an orientation error."<<std::endl;
//	f = m.add_face(u,x,v);
//	assert(f != Mesh::null_face());
//  }
#endif
  return 0;
}

int Helpers3D::drawTriangulation(const QVector<QVector3D>& points)
{
#ifdef USE_CGAL
//  std::ifstream in((argc>1)?argv[1]:"data/triangulation_prog1.cin");
//  std::istream_iterator<Point> begin(in);
//  std::istream_iterator<Point> end;

  std::vector<Point> points2;
  std::for_each(points.begin(), points.end(), [&points2](const QVector3D& point) {
	  points2.push_back({point.x(), point.y()});
  });

  Triangulation t;
  t.insert(points2.begin(), points2.end());
  //C:\Projects\qt5-build\qtbase\include\QtGui
//  CGAL::draw(t);
#endif
  return EXIT_SUCCESS;
}

std::shared_ptr<TriangleGeometry> Helpers3D::extrudedTriangleIsland(const TriangleIsland& modelIsland,
																	const std::vector<Vec3>& modelVertices,
																	float modelFloorLevel)
{// Get (top) triangle island points, get it casted to floor, connect both.
	TriangleGeometryData returnData;

	std::vector<Vec3>& uniqueSupportPointsArray = returnData.vertices;
	std::vector<uint32_t>& supportGeometryIndices = returnData.indices;

	// These point to vertex indices in the model, for which the island was generated.
	const std::vector<uint32_t> islandTriangleIndices = modelIsland.getTriangleIndices();
	// Create a set of unique indices.
	const std::set<uint32_t> islandUniqueIndices(islandTriangleIndices.begin(), islandTriangleIndices.end());
	const uint32_t numIslandUniqueIndices = uint32_t(islandUniqueIndices.size());

	// Collect island vertices casted to floor here.
	std::vector<Vec3> floorVertices;
	floorVertices.reserve(numIslandUniqueIndices);

	// Reserve space for unique vertices: top and floor - thus factor of 2.
	uniqueSupportPointsArray.reserve(2*numIslandUniqueIndices);

	//These maps will help to match indices to vertices and vice-versa.
	IndicesToVertices indicesToUniqueVertices(Helpers3D::vertexLess);
	IndicesToVertices topIndicesToFloorVertices(Helpers3D::vertexLess);
	IndicesToVertices floorIndicesToTopVertices(Helpers3D::vertexLess);

	uint32_t currSupportIndex = 0;
	for (uint32_t index : islandUniqueIndices)
	{// Build a map of indices to top vertices and fill support array with top vertices.
		const Vec3& topVertex = modelVertices[index];

		auto it = indicesToUniqueVertices.find(topVertex);
		if (it == indicesToUniqueVertices.end())
		{// If not yet added, add an unique top vertex and a matching index from support array.
			indicesToUniqueVertices[topVertex] = currSupportIndex;
			uniqueSupportPointsArray.push_back(topVertex);

			// Match floor vertices to them by means of top index.
			const Vec3 floorVertex{topVertex.x(), topVertex.y(), modelFloorLevel};
			floorVertices.push_back(floorVertex);
			// Remember to increment the support index afterwards!
			topIndicesToFloorVertices[floorVertex] = currSupportIndex++;
		}
	}
	for (const Vec3& floorVertex : floorVertices)
	{// Extend the map with floor vertices and add floor vertices to support array (keep in mind: sometimes top==floor)
		auto it = indicesToUniqueVertices.find(floorVertex);
		if (it == indicesToUniqueVertices.end())
		{// If floor vertex was not yet added, add an unique floor vertex and a matching index from support array.
			indicesToUniqueVertices[floorVertex] = currSupportIndex;

			//========================================================================
			// TODO!: Watch out - maybe this should be somehow extracted and exposed
			Vec3 adjustedFloorVertex = floorVertex;
			adjustedFloorVertex.z() = modelFloorLevel;
//			uniqueSupportPoints.push_back(adjustedFloorVertex);
			uniqueSupportPointsArray.push_back(floorVertex);
			//========================================================================

			// Match top vertices to them by means of floor index.
			const Vec3 topVertex = uniqueSupportPointsArray[topIndicesToFloorVertices[floorVertex]];
			// Remember to increment the support index afterwards!
			floorIndicesToTopVertices[topVertex] = currSupportIndex++;
		}
	}

	// IslandAlpahShapeRing should have Z changed to floorLevel to be found in the maps.
	const std::vector<Vec3> islandAlphaShapeRing = Helpers3D::computeAlphaShapeSegments(floorVertices, modelFloorLevel);


	// Reserve for top, bottom and side triangles.
	supportGeometryIndices.reserve(2*islandTriangleIndices.size() + 6*islandAlphaShapeRing.size());
	for (uint32_t triangleVertexIndex : islandTriangleIndices)
	{// Generate triangle indices for top:
		supportGeometryIndices.push_back(indicesToUniqueVertices[modelVertices[triangleVertexIndex]]);
	}
	for (uint32_t index : islandTriangleIndices)
	{// Generate triangle indices for bottom.
		const Vec3 topVertex = modelVertices[index];
		const uint32_t floorIndex = floorIndicesToTopVertices[topVertex];
		const Vec3 floorVertex = uniqueSupportPointsArray[floorIndex];
		supportGeometryIndices.push_back(indicesToUniqueVertices[floorVertex]);
	}

	for(uint32_t i=0; i<islandAlphaShapeRing.size(); i+=2)
	{// Generate side triangles.
		const Vec3& prevAlphaPoint = islandAlphaShapeRing[i];
		const Vec3& alphaPoint = islandAlphaShapeRing[i+1];
		const uint32_t floorLeft = indicesToUniqueVertices[prevAlphaPoint];
		const uint32_t topLeft = topIndicesToFloorVertices[prevAlphaPoint];
		const uint32_t floorRight = indicesToUniqueVertices[alphaPoint];
		const uint32_t topRight = topIndicesToFloorVertices[alphaPoint];

		supportGeometryIndices.push_back(floorLeft);
		supportGeometryIndices.push_back(topLeft);
		supportGeometryIndices.push_back(floorRight);
		supportGeometryIndices.push_back(topLeft);
		supportGeometryIndices.push_back(topRight);
		supportGeometryIndices.push_back(floorRight);
	}

	return std::make_shared<TriangleGeometry>(returnData);
}

QQuaternion Helpers3D::getRotationFromDirection(const QVector3D& direction, const QVector3D& up)
{
	return QQuaternion::fromDirection(-direction, up);
}

QQuaternion Helpers3D::getRotationFromAxes(const QVector3D& axisFrom, const QVector3D& axisTo)
{
	QVector3D axis = getRotationAxis(axisFrom, axisTo);
	float angle = getSmallRotationAngle(axisFrom, axisTo);
	return getRotationFromAxisAndAngle(axis, angle);
}

QQuaternion Helpers3D::getRotationFromAxisAndAngle(const QVector3D& axis, const float angle)
{
	return QQuaternion::fromAxisAndAngle(axis, angle);
}

QQuaternion Helpers3D::getRotationFromQuaternions(const QQuaternion& current, const QQuaternion& additional)
{
	return current*additional;
}

float Helpers3D::getSmallRotationAngle(const QVector3D& from, const QVector3D& to)
{
	return 180.0/(float)M_PI*acos(QVector3D::dotProduct(from.normalized(), to.normalized()));
}

QVector3D Helpers3D::getRotationAxis(const QVector3D& from, const QVector3D& to)
{
	return QVector3D::crossProduct(from, to).normalized();
}

QVector3D Helpers3D::getRotationAxis(const QQuaternion& rotation)
{
	float angle;
	QVector3D vector;
	rotation.getAxisAndAngle(&vector, &angle);
	return vector;
}

float Helpers3D::getRotationAngle(const QQuaternion& rotation)
{
	float angle;
	QVector3D vector;
	rotation.getAxisAndAngle(&vector, &angle);
	return angle;
}

QVariantMap Helpers3D::getLinePlaneIntersection(const QVector3D& origin,
												const QVector3D& ray,
												const QVector3D& planeNormal,
												const QVector3D& planeCoord)
{

	if (qFuzzyIsNull(QVector3D::dotProduct(planeNormal, ray)))
	{// No intersection, the line is parallel to the plane
		return QVariantMap{{"intersection", QVector3D()}, {"isHit", false}};
	}

	float d = QVector3D::dotProduct(planeNormal, planeCoord);
	// Compute the parameter for the directed line ray intersecting the plane
	float lineParam = (d - QVector3D::dotProduct(planeNormal, origin))/
					  QVector3D::dotProduct(planeNormal, ray);

	return QVariantMap{{"intersection", origin + ray.normalized()*lineParam}, {"isHit", true}};
}

QVector3D Helpers3D::getRotatedVector(const QQuaternion& q, const QVector3D v)
{
	return q*v;
}

bool approximatelyEqual(float a, float b, float epsilon)
{
	return std::abs(a - b) <= ( (std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool essentiallyEqual(float a, float b, float epsilon)
{
	return std::abs(a - b) <= ( (std::abs(a) > std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool definitelyGreaterThan(float a, float b, float epsilon)
{
	return (a - b) > ( (std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool definitelyLessThan(float a, float b, float epsilon)
{
	return (b - a) > ( (std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}



void Helpers3D::countAssimpFacesAndVertices(const aiScene* _scene, uint32_t& numAssimpMeshFaces, uint32_t& numAssimpVertices)
{// NOTE: this function will accumulate in passed counters  without resetting to 0.
	for (uint32_t m=0; m<_scene->mNumMeshes; ++m)
	{
		numAssimpMeshFaces += _scene->mMeshes[m]->mNumFaces;
		numAssimpVertices  += _scene->mMeshes[m]->mNumVertices;
	}
}

void Helpers3D::getContiguousAssimpVerticesAndNormals(const aiScene* assimpScene,
													  std::vector<Vec3>& assimpVertices,
													  std::vector<Vec3>& assimpNormals)
{// NOTE: this function will update existing arrays - resize and append data to them.
	Chronograph chronograph(__FUNCTION__, false);

	uint32_t numAssimpMeshFaces = 0;
	uint32_t numAssimpVertices = 0;
	countAssimpFacesAndVertices(assimpScene, numAssimpMeshFaces, numAssimpVertices);

	const uint32_t prevAssimpVerticesSize = static_cast<uint32_t>(assimpVertices.size());
	const uint32_t prevAssimpNormalsSize = static_cast<uint32_t>(assimpNormals.size());
	assimpVertices.resize(prevAssimpVerticesSize + numAssimpVertices);
	assimpNormals.resize(prevAssimpNormalsSize + numAssimpVertices);

	Vec3* vertexPtr = &assimpVertices[prevAssimpVerticesSize];
	Vec3* normalsPtr = &assimpNormals[prevAssimpNormalsSize];

	const uint32_t numMeshes = assimpScene->mNumMeshes;
	for (uint32_t m=0; m<numMeshes; ++m)
	{
		const aiMesh* mesh = assimpScene->mMeshes[m];
		const uint32_t numVertices = mesh->mNumVertices;
		for (uint32_t v=0; v<numVertices; ++v, ++vertexPtr)
		{
			// TODO: We should probably perform subsequent mesh node transformations here.
			const aiVector3D& vertex = assimpScene->mMeshes[m]->mVertices[v];
			*vertexPtr  = *reinterpret_cast<const Vec3*>(&vertex);
		}

		if (!mesh->mNormals)
			continue;

		for (uint32_t v=0; v<numVertices; ++v, ++normalsPtr)
		{
			const aiVector3D& normal = assimpScene->mMeshes[m]->mNormals[v];
			*normalsPtr = *reinterpret_cast<const Vec3*>(&normal);
		}
	}
}

IndicesToVertices Helpers3D::mapIndicesToUniqueVerticesAndNormals(const std::vector<Vec3>& vertices,
																  const std::vector<Vec3>& normals,
																  std::vector<Vec3>& uniqueVertices,
																  std::vector<Vec3>& uniqueNormals)
{
	Chronograph chronograph(__FUNCTION__, false);

	uniqueVertices.clear();
	uniqueNormals.clear();
	IndicesToVertices indicesToUniqueVertices(vertexLess);
	uint32_t currIndex = 0;
	for (uint32_t v=0; v<vertices.size(); ++v)
	{
		const Vec3& vertex = vertices[v];
		const Vec3& normal = normals[v];
		auto it = indicesToUniqueVertices.find(vertex);
		if (it == indicesToUniqueVertices.end())
		{
			indicesToUniqueVertices[vertex] = currIndex++;
			uniqueNormals.push_back(normal);
			uniqueVertices.push_back(vertex);
		}
	}
	const uint32_t numUniqueVertices = uint32_t(indicesToUniqueVertices.size());
	std::cout << " ### " << "numUniqueVertices" << " :" << numUniqueVertices << "," << "" << std::endl;
	return indicesToUniqueVertices;
}

std::vector<uint32_t> Helpers3D::getRemappedIndices(const IndicesToVertices& indicesToUniqueVertices,
													const std::vector<Vec3>& nonUniqueVertices)
{// Remapping new indices to all unique vertices by searching map.
	Chronograph chronograph(__FUNCTION__, false);

	std::vector<uint32_t> remappedVertexIndices;
	remappedVertexIndices.reserve(nonUniqueVertices.size());
	for(const Vec3& vertex : nonUniqueVertices)
	{
		auto mapIt = indicesToUniqueVertices.find(vertex);
		assert(mapIt != indicesToUniqueVertices.end());
		remappedVertexIndices.emplace_back(mapIt->second);
	}
	return remappedVertexIndices;
}


std::vector<uint32_t> Helpers3D::calculateOverhangingTriangleIndices(const std::vector<Vec3>& vertices,
																	 const std::vector<uint32_t>& indices,
																	 float _overhangAngleMax)
{
	Chronograph chronograph(__FUNCTION__, false);

	std::vector<uint32_t> overhangingIndices;
	const uint32_t numIndices = uint32_t(indices.size());
	if (numIndices == 0)
		return overhangingIndices;

	const uint32_t* pi = &indices[0];
	for (uint32_t triangleIndex=0; triangleIndex<numIndices; triangleIndex += 3)
	{
		static const auto isTriangleOverhanging = [](const Vec3& v0, const Vec3& v1, const Vec3& v2, float maxOverhangAngle)
		{// Calculate based on vertices.
			return (((v1-v0).cross(v2-v0)).normalized()).dot(Vec3{0,0,1}) < std::cosf(float(M_PI) - maxOverhangAngle);
		};

		const uint32_t i0 = *pi++;
		const uint32_t i1 = *pi++;
		const uint32_t i2 = *pi++;

		const Vec3& v0 = vertices[i0];
		const Vec3& v1 = vertices[i1];
		const Vec3& v2 = vertices[i2];

		if (!isTriangleOverhanging(v0, v1, v2, _overhangAngleMax))
			continue;

		overhangingIndices.push_back(i0);
		overhangingIndices.push_back(i1);
		overhangingIndices.push_back(i2);
	}

	return overhangingIndices;
}
