#include "SimplexGeometry.h"

#include <iostream>

SimplexGeometry::SimplexGeometry()
{
	const uint32_t numPoints = 10000;
	_points.resize(numPoints);
	std::for_each(_points.begin(), _points.end(), [](QVector3D& point) {
		point = {5.0f*rand()/RAND_MAX,5.0f*rand()/RAND_MAX,5.0f*rand()/RAND_MAX};
	});


	connect(this, &SimplexGeometry::pointsChanged, this, &SimplexGeometry::updateData);
	connect(this, &SimplexGeometry::simplexTypeChanged, this, &SimplexGeometry::updateData);

	updateData();
}

void SimplexGeometry::updateData()
{
	clear();

	//TODO: This function may be doing too much.
	uint32_t numPoints = uint32_t(_points.size());

	const int stride = 3 * sizeof(float);

	QByteArray v;
	v.resize(numPoints * stride);
	float *p = reinterpret_cast<float *>(v.data());

	for(const QVector3D& vertex : _points)
	{
		*p++ = vertex.x();
		*p++ = vertex.y();
		*p++ = vertex.z();
	}

	setVertexData(v);
	setStride(stride);

	setPrimitiveType(mapSimplexToPrimitive(_simplexType));
	addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
				 0,
				 QQuick3DGeometry::Attribute::F32Type);

	update();
}

SimplexGeometry::SimplexType SimplexGeometry::getSimplexType() const
{
	return _simplexType;
}

void SimplexGeometry::setSimplexType(SimplexGeometry::SimplexType type)
{
	if (_simplexType == type)
		return;
	_simplexType = type;
	emit simplexTypeChanged();
}

QVector<QVector3D> SimplexGeometry::getPoints() const
{
	return _points;
}

void SimplexGeometry::setPoints(QVector<QVector3D> newPoints)
{
	std::cout << " ### " << __FUNCTION__ << " :" << "" << "," << "" << std::endl;
	_points = newPoints;
	emit pointsChanged();
}

QQuick3DGeometry::PrimitiveType SimplexGeometry::mapSimplexToPrimitive(SimplexGeometry::SimplexType type)
{
	static const QMap<SimplexType, PrimitiveType> map{{SimplexType::Points,      PrimitiveType::Points},
													  {SimplexType::Lines,       PrimitiveType::Lines},
													  {SimplexType::LineStrip,   PrimitiveType::LineStrip},
													  {SimplexType::Triangles,   PrimitiveType::Triangles}};

	return map[type];
}
