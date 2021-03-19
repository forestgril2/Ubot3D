#pragma once

#include <QObject>
#include <QQuick3DGeometry>
#include <QVector>
#include <QVector3D>


class SimplexGeometry : public QQuick3DGeometry
{
	Q_OBJECT
	QML_NAMED_ELEMENT(SimplexGeometry)
	Q_PROPERTY(QVector<QVector3D> points READ getPoints WRITE setPoints NOTIFY pointsChanged)
	Q_PROPERTY(SimplexType simplexType READ getSimplexType WRITE setSimplexType NOTIFY simplexTypeChanged)

public:

	enum SimplexType
	{
		Points,
		Lines,
		LineStrip,
		Triangles
	};
	Q_ENUM(SimplexType)

	SimplexGeometry();

	Q_INVOKABLE void updateData();

	SimplexType getSimplexType() const;
	void setSimplexType(SimplexType type);

	QVector<QVector3D> getPoints() const;
	void setPoints(QVector<QVector3D> newPoints);

signals:
	void pointsChanged();
	void simplexTypeChanged();

private:
	PrimitiveType mapSimplexToPrimitive(SimplexType type);

	QVector<QVector3D> _points;
	SimplexType _simplexType;
};
