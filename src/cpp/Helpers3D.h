#pragma once

#include <qqml.h>

#include <QObject>
#include <QVector3D>
#include <QQuaternion>

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

	static Q_INVOKABLE QVariantMap getLinePlaneIntersection(const QVector3D& ray,
															const QVector3D& origin,
															const QVector3D& planeNormal,
															const QVector3D& planeCoord);
};
