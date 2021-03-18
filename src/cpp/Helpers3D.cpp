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

	const TriangleGeometry* stlGeometryFirst = qvariant_cast<TriangleGeometry*>(*it);
	it++;
	const TriangleGeometry* stlGeometryBack = qvariant_cast<TriangleGeometry*>(*it);

	if (!stlGeometryFirst) {
		std::cout << " ### " << __FUNCTION__ << " cannot cast to TriangleGeometry:" << "" << "," << "" << std::endl;
		return false;
	}

	aiScene* destScene = new aiScene();

	aiScene* newSceneFirst;
	aiScene* newSceneBack;

	Assimp::SceneCombiner::CopyScene(&newSceneFirst, stlGeometryFirst->getAssimpScene());
	Assimp::SceneCombiner::CopyScene(&newSceneBack, stlGeometryBack->getAssimpScene());

	std::vector<Assimp::AttachmentInfo> sceneAttachments{{newSceneFirst, newSceneFirst->mRootNode},
														 {newSceneBack,  newSceneFirst->mRootNode}};
	Assimp::SceneCombiner::MergeScenes(&destScene, newSceneFirst, sceneAttachments);
//	mergeScenes(newScene, stlGeometryBack->getAssimpScene(), glm::mat4x4());


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

void Helpers3D::mergeScenes(aiScene* scene, const aiScene* otherScene, const glm::mat4x4 transform)
{// This is a modified copy of: https://github.com/assimp/assimp/issues/203#issuecomment-293866481
	aiMesh ** new_meshes = new aiMesh*[scene->mNumMeshes + otherScene->mNumMeshes];
	memcpy(new_meshes, scene->mMeshes, scene->mNumMeshes * sizeof(aiMesh*));
	delete[] scene->mMeshes;
	scene->mMeshes = new_meshes;
	for (int i = 0, offset = scene->mNumMeshes; i < otherScene->mNumMeshes; i++, offset++)
	{
		aiMesh *mesh = otherScene->mMeshes[i];
		aiMesh *copy_mesh = scene->mMeshes[offset] = new aiMesh;

		copy_mesh->mNumVertices = mesh->mNumVertices;
		copy_mesh->mVertices = new aiVector3D[mesh->mNumVertices];
		copy_mesh->mPrimitiveTypes = mesh->mPrimitiveTypes;
		for (int j = 0; j < mesh->mNumVertices; j++) {
			glm::vec4 v(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z, 1);
			v = transform * v;
			copy_mesh->mVertices[j] = aiVector3D(v.x, v.y, v.z);
		}

		// copy_mesh->mMaterialIndex = mesh->mMaterialIndex; // FIXME
		copy_mesh->mName = mesh->mName;
		copy_mesh->mNumFaces = mesh->mNumFaces;
		if (mesh->mNormals) {
			copy_mesh->mNormals = new aiVector3D[mesh->mNumVertices];
			for (int j = 0; j < mesh->mNumVertices; j++) {
				glm::vec4 v(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z, 0);
				v = transform * v;
				copy_mesh->mNormals[j] = aiVector3D(v.x, v.y, v.z);
			}
		}

		copy_mesh->mFaces = new aiFace[mesh->mNumFaces];
		for (int j = 0; j < mesh->mNumFaces; j++) {
			copy_mesh->mFaces[j].mNumIndices = mesh->mFaces[j].mNumIndices;
			copy_mesh->mFaces[j].mIndices = new unsigned[mesh->mFaces[j].mNumIndices];
			memcpy(copy_mesh->mFaces[j].mIndices, mesh->mFaces[j].mIndices,
				   mesh->mFaces[j].mNumIndices * sizeof(unsigned));
		}
	}
	scene->mNumMeshes += otherScene->mNumMeshes;
}
