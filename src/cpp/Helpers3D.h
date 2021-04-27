#pragma once

#include <qqml.h>

#include <map>
#include <vector>

#include <QObject>
#include <QVector3D>
#include <QQuaternion>

#include <glm/mat4x4.hpp>

#include <Eigen/Geometry>
using Vec3 = Eigen::Vector3f;
using IndicesToVertices = std::map<Vec3, uint32_t, bool(*)(const Vec3& a, const Vec3& b)>;

struct aiScene;
class TriangleGeometry;
class TriangleIsland;

class Helpers3D : public QObject
{
	Q_OBJECT
	QML_NAMED_ELEMENT(Helpers3D)

public:
	static Q_INVOKABLE QQuaternion getRotationFromDirection(const QVector3D& direction, const QVector3D& up);
	static Q_INVOKABLE QQuaternion getRotationFromAxes(const QVector3D& axisFrom, const QVector3D& axisTo);
	static Q_INVOKABLE QQuaternion getRotationFromAxisAndAngle(const QVector3D& axis, const float angle);
	static Q_INVOKABLE QQuaternion getRotationFromQuaternions(const QQuaternion& current, const QQuaternion& additional);
	static Q_INVOKABLE float getSmallRotationAngle(const QVector3D& from, const QVector3D& to);
	static Q_INVOKABLE QVector3D getRotationAxis(const QVector3D& from, const QVector3D& to);
	static Q_INVOKABLE QVector3D getRotationAxis(const QQuaternion& rotation);
	static Q_INVOKABLE float getRotationAngle(const QQuaternion& rotation);

	static Q_INVOKABLE QVariantMap getLinePlaneIntersection(const QVector3D& origin,
															const QVector3D& ray,
															const QVector3D& planeNormal,
															const QVector3D& planeCoord);
	static Q_INVOKABLE QVector3D getRotatedVector(const QQuaternion& q, const QVector3D v);

	// TODO: CGAL related - extract to CGAL class or whatever.
	static std::shared_ptr<TriangleGeometry> extrudedTriangleIsland(const TriangleIsland& modelIsland,
																	const std::vector<Vec3>& modelVertices,
																	float modelFloorLevel = 0);
	static std::vector<Vec3> computeConvexHull(const std::vector<Vec3>& points);
	static std::vector<Vec3> computeAlphaShapeSegments(const std::vector<Vec3>& points, float floorLevel);
	static int createCgalMesh();
	static int drawTriangulation(const QVector<QVector3D>& points);

	// TODO: Assimp related - extract to AssimpSceneProcessor or whatever.
	static void countAssimpFacesAndVertices(const aiScene* _scene, uint32_t& numAssimpMeshFaces, uint32_t& numAssimpVertices);
	static void getContiguousAssimpVerticesAndNormals(const aiScene* _scene,
													  std::vector<Vec3>& assimpVertices,
													  std::vector<Vec3>& assimpNormals);
	static IndicesToVertices mapIndicesToUniqueVerticesAndNormals(const std::vector<Vec3>& vertices,
																  const std::vector<Vec3>& normals,
																  std::vector<Vec3>& uniqueVertices,
																  std::vector<Vec3>& uniqueNormals);
	static std::vector<uint32_t> getRemappedIndices(const IndicesToVertices& indicesToUniqueVertices,
														  const std::vector<Vec3>& nonUniqueVertices);
	static std::vector<uint32_t> calculateOverhangingTriangleIndices(const std::vector<Vec3>& vertices,
															  const std::vector<uint32_t>& indices, float _overhangAngleMax);

	static bool vertexLess(const Vec3& a, const Vec3& b);

private:
	template<class P2, class V3>
	static std::vector<P2> getCgalPoints2(const std::vector<V3>& points)
	{
		std::vector<P2> points2;
		points2.reserve(points.size());
		std::for_each(points.begin(), points.end(), [&points2](const V3& point) {
			points2.push_back({point.x(), point.y()});
		});
		return points2;
	}
};

bool approximatelyEqual(float a, float b, float epsilon);
bool essentiallyEqual(float a, float b, float epsilon);
bool definitelyGreaterThan(float a, float b, float epsilon);
bool definitelyLessThan(float a, float b, float epsilon);
