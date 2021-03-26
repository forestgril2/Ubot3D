#pragma once

#include <qqml.h>

#include <map>
#include <vector>

#include <QObject>
#include <QVector3D>
#include <QQuaternion>

#include <glm/mat4x4.hpp>
#include <glm/mat4x4.hpp>


#include <Eigen/Geometry>
using Vec3 = Eigen::Vector3f;

struct aiScene;

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
	static Q_INVOKABLE bool exportModelsToSTL(const QVariantList& stlExportData, const QString filePath);

	// TODO: CGAL related - extract to CGAL class or whatever.
	static QVector<QVector3D> computeConvexHull(const QVector<QVector3D>& points);
	static QVector<QVector3D> computeAlphaShape(const QVector<QVector3D>& points);
	static int createCgalMesh();
	static int drawTriangulation(const QVector<QVector3D>& points);

	static std::map<Vec3, uint32_t> vertexIndexing(const std::vector<Vec3>& vertices);

private:
	template<class P>
	static std::vector<P> getCgalPoints2(const QVector<QVector3D>& points)
	{
		std::vector<P> points2;
		std::for_each(points.begin(), points.end(), [&points2](const QVector3D& point) {
			points2.push_back({point.x(), point.y()});
		});
		return points2;
	}

};

bool approximatelyEqual(float a, float b, float epsilon);
bool essentiallyEqual(float a, float b, float epsilon);
bool definitelyGreaterThan(float a, float b, float epsilon);
bool definitelyLessThan(float a, float b, float epsilon);
