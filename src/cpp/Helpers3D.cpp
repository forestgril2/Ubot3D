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

void alpha_edges( const Alpha_shape_2& A, OutputIterator out)
{
  Alpha_shape_edges_iterator it = A.alpha_shape_edges_begin(),
							 end = A.alpha_shape_edges_end();
  for( ; it!=end; ++it)
	*out++ = A.segment(*it);
}

QVector<QVector3D> Helpers3D::computeAlphaShape(const QVector<QVector3D>& points)
{
	if (points.size() == 0)
		return QVector<QVector3D>();

	QVector<QVector3D> result;
#ifdef USE_CGAL

	std::vector<Point> points2 = getCgalPoints2<Point>(points);
	Alpha_shape_2 A(points2.begin(), points2.end(),
					FT(160),
					Alpha_shape_2::GENERAL);
	std::vector<Segment> segments;
	alpha_edges(A, std::back_inserter(segments));
	std::cout << "Alpha Shape computed" << std::endl;
	std::cout << segments.size() << " alpha shape edges" << std::endl;
	std::cout << "Optimal alpha: " << *A.find_optimal_alpha(1)<<std::endl;

	std::for_each(segments.begin(), segments.end(), [&result](const Segment& s) {
		result.push_back({static_cast<float>(s[0].x()), static_cast<float>(s[0].y()), 0});
		result.push_back({static_cast<float>(s[1].x()), static_cast<float>(s[1].y()), 0});
	});
#endif
	return result;
}

QVector<QVector3D> Helpers3D::computeConvexHull(const QVector<QVector3D>& points)
{
QVector<QVector3D> result;
#ifdef USE_CGAL
//  std::ifstream in("data/triangulation_prog1.cin");
//  std::istream_iterator<Point> points2.begin(in);
//  std::istream_iterator<Point> points2.end;

  std::vector<TPoint> points2 = getCgalPoints2<TPoint>(points);

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

bool Helpers3D::exportModelsToSTL(const QVariantList& stlExportData, const QString filePath)
{
	assert(stlExportData.size() > 0);

//	QList<QVariantMap>::const_iterator it = stlExportData.begin();

	std::vector<Assimp::AttachmentInfo> sceneAttachments;

	aiNode* masterNode = nullptr;
	aiScene* masterScene = nullptr;
	for (const QVariant& exportData : stlExportData)
	{
		QVariantMap map = exportData.toMap();
		const TriangleGeometry* stlGeometry = qvariant_cast<TriangleGeometry*>(map.value("geometry"));
		if (!stlGeometry) {
			std::cout << " ### " << __FUNCTION__ << " cannot cast to TriangleGeometry:" << "" << "," << "" << std::endl;
			return false;
		}

		aiScene* geometryScene;// = nullptr;
		Assimp::SceneCombiner::CopyScene(&geometryScene, stlGeometry->getAssimpScene());

		const QMatrix4x4 transform = qvariant_cast<QMatrix4x4>(map.value("transform"));
		aiMatrix4x4 aiTransform = *reinterpret_cast<const aiMatrix4x4*>(&transform);
		aiTransform.Transpose();

		// TODO: Remove and regenerate normals.
//		geometryScene->mRootNode->mTransformation = aiTransform;

		for (uint32_t m=0; m<geometryScene->mNumMeshes; ++m)
		{// Transform all vertices according to their root node.
			const uint32_t numFacesInThisMesh = geometryScene->mMeshes[m]->mNumFaces;
			for (uint32_t f=0; f<numFacesInThisMesh; ++f)
			{
				aiFace& face = geometryScene->mMeshes[m]->mFaces[f];
				const uint32_t numFaceIndexes = face.mNumIndices;
				for (uint32_t fi=0; fi<numFaceIndexes; ++fi)
				{
					aiVector3D& vertex = geometryScene->mMeshes[m]->mVertices[face.mIndices[fi]];
					vertex *= aiTransform;
//					std::cout << " ### " << __FUNCTION__ << " :" << "" << "," << "" << std::endl;
				}
			}
		}

		if (!masterNode)
		{
			masterNode = geometryScene->mRootNode;
			masterScene = geometryScene;
		}

		sceneAttachments.push_back({geometryScene, masterNode});
	}

	aiScene* destScene = new aiScene();
	Assimp::SceneCombiner::MergeScenes(&destScene, masterScene, sceneAttachments);

	Assimp::Exporter exporter;

//	for (uint32_t i=0; i<exporter.GetExportFormatCount(); ++i)
//	{
//		std::string desc(exporter.GetExportFormatDescription(i)->description);
//		std::string formatIdentifier(exporter.GetExportFormatDescription(i)->id);
//		std::cout << " ### " << __FUNCTION__ << ": " << desc << "," << formatIdentifier << ", " << i << std::endl << std::endl;
//	}

	if (AI_SUCCESS == exporter.Export(destScene, "stlb", filePath.toStdString()))
	{
		std::cout << " ### " << __FUNCTION__ << " filePATH:" << filePath.toStdString() << " export OK" << std::endl;
	}
	else
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR for file:" << filePath.toStdString() << std::endl;
	}

	return true;
}

