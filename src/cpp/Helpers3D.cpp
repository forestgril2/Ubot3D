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

	QList<QVariant>::const_iterator it = stlExportData.begin();

	std::vector<Assimp::AttachmentInfo> sceneAttachments;

	aiNode* masterNode = nullptr;
	aiScene* masterScene = nullptr;
	for (const QVariant& data : stlExportData)
	{
		const TriangleGeometry* stlGeometry = qvariant_cast<TriangleGeometry*>(data);
		if (!stlGeometry) {
			std::cout << " ### " << __FUNCTION__ << " cannot cast to TriangleGeometry:" << "" << "," << "" << std::endl;
			return false;
		}

		aiScene* geometryScene;// = nullptr;
		Assimp::SceneCombiner::CopyScene(&geometryScene, stlGeometry->getAssimpScene());

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
	if (AI_SUCCESS == exporter.Export(destScene, "stl", filePath.toStdString()))
	{
		std::cout << " ### " << __FUNCTION__ << " filePATH:" << filePath.toStdString() << " export OK" << std::endl;
	}
	else
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR for file:" << filePath.toStdString() << std::endl;
	}

	return true;
}
