#include "FileImportExport.h"

#include <fstream>
#include <iostream>
#include <vector>

#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/cexport.h>
#include <assimp/SceneCombiner.h>

#include <TriangleGeometry.h>

struct StlTriangleData
{
	uint32_t numModelTriangles   = 0;
	uint32_t numSupportTriangles = 0;
	uint32_t numStandTriangles   = 0;
};

static StlTriangleData readStlTriangleData(const std::string& file)
{
	std::vector<uint8_t> inputBuffer(sizeof(StlTriangleData));
	std::basic_ifstream<uint8_t> stream(file);
	stream.read(&inputBuffer[0], sizeof(StlTriangleData));
	return *reinterpret_cast<StlTriangleData*>(&inputBuffer[0]);
}

static void writeStlTriangleData(const std::string& file, const StlTriangleData& data)
{
	std::basic_fstream<uint8_t> stream(file, std::ios::binary | std::ios::out | std::ios::in);

	stream.seekp(0, std::ios_base::beg);
	stream.write((const uint8_t*)&data, sizeof(data));
	stream.close();
}

static std::vector<uint8_t> readFirstFileBytes(const std::string& file, uint32_t numBytes)
{
	std::basic_ifstream<uint8_t> stream(file);
	std::vector<uint8_t> buffer(numBytes);
	stream.read(&buffer[0], numBytes);

	return buffer;
}

static void writeFirstFileBytes(const std::string& file, std::vector<uint8_t>& buffer)
{
	std::fstream stream(file, std::ios::binary | std::ios::out | std::ios::in);
	stream.seekp(0, std::ios_base::beg);
	stream.write((char*)&buffer[0], buffer.size());
	stream.close();
}

static aiScene* generateTransformedGeometryScene(const TriangleGeometry* geometry, const aiMatrix4x4& aiTransform)
{//https://github.com/assimp/assimp/issues/203
	aiScene *scene = new aiScene();                       // deleted: by us after use

	//TODO: this function may need to be defined differently, when using different primitive types, jump to "BECAUSE"
	const std::vector<Vec3>& geometryVertices = geometry->getData().vertices;
	const std::vector<uint32_t>& geometryIndices = geometry->getData().indices;

	const uint32_t destVerticesSize = static_cast<unsigned int>(geometryVertices.size());
	aiVector3D* const vertices = new aiVector3D[destVerticesSize];
	for(uint32_t i=0; i<destVerticesSize; ++i)
	{
		const Vec3& geometryVertex = geometryVertices[i];
		aiVector3D& vertex = vertices[i];

		vertex.x = geometryVertex.x();
		vertex.y = geometryVertex.y();
		vertex.z = geometryVertex.z();

		vertex *= aiTransform;
	}

	assert(0 == geometryIndices.size()%3);
	const uint32_t numFaces = static_cast<unsigned int>(geometryIndices.size()/3);
	aiFace *faces = new aiFace[numFaces];
	for(uint32_t f=0; f<numFaces; ++f)
	{
		faces[f].mNumIndices = 3;
		faces[f].mIndices = new unsigned int [3] {
			geometryIndices[3*f   ],
			geometryIndices[3*f +1],
			geometryIndices[3*f +2]
		};
	}

	aiMesh *mesh = new aiMesh();                        // deleted: Version.cpp:150
	mesh->mNumVertices = destVerticesSize;
	mesh->mNormals = nullptr;
	mesh->mVertices = vertices;
	mesh->mNumFaces = numFaces;
	mesh->mFaces = faces;

	// a valid material is needed, even if its empty

	aiMaterial *material = new aiMaterial();            // deleted: Version.cpp:155

	// a root node with the mesh list is needed; if you have multiple meshes, this must match.

	aiNode *root = new aiNode();                        // deleted: Version.cpp:143
	root->mNumMeshes = 1;
	root->mMeshes = new unsigned [] { 0 };              // deleted: scene.cpp:77

	// pack mesh(es), material, and root node into a new minimal aiScene

	scene->mNumMeshes = 1;
	scene->mMeshes = new aiMesh * [] { mesh };            // deleted: Version.cpp:151
	scene->mNumMaterials = 1;
	scene->mMaterials = new aiMaterial * [] { material }; // deleted: Version.cpp:158
	scene->mRootNode = root;

	return scene;
}

static aiScene* copyTransformedGeometryScene(const TriangleGeometry* geometry, const aiMatrix4x4& aiTransform)
{
	const aiScene* sourceScene = geometry->getAssimpScene();
	assert(sourceScene);
	aiScene* destGeometryScene;// = nullptr;
	Assimp::SceneCombiner::CopyScene(&destGeometryScene, sourceScene);

	// TODO: Remove and regenerate normals.
	//		geometryScene->mRootNode->mTransformation = aiTransform;

	for (uint32_t m=0; m<destGeometryScene->mNumMeshes; ++m)
	{// Transform all vertices according to their root node.
		const uint32_t numFacesInThisMesh = destGeometryScene->mMeshes[m]->mNumFaces;
		for (uint32_t f=0; f<numFacesInThisMesh; ++f)
		{
			aiFace& face = destGeometryScene->mMeshes[m]->mFaces[f];
			const uint32_t numFaceIndexes = face.mNumIndices;
			for (uint32_t fi=0; fi<numFaceIndexes; ++fi)
			{
				aiVector3D& vertex = destGeometryScene->mMeshes[m]->mVertices[face.mIndices[fi]];
				vertex *= aiTransform;
			}
		}
	}
	return destGeometryScene;
}

static std::vector<aiScene*> generateScenes(const QVariantList& stlExportData, StlTriangleData& data)
{
	std::vector<aiScene*> scenes;

	// deleting the scene will also take care of the vertices, faces, meshes, materials, nodes, etc.

	for (const QVariant& exportData : stlExportData)
	{
		QVariantMap map = exportData.toMap();
		const TriangleGeometry* stlGeometry = qvariant_cast<TriangleGeometry*>(map.value("geometry"));
		const QMatrix4x4 transform          = qvariant_cast<QMatrix4x4>       (map.value("transform"));
		const bool isSupportExported        = qvariant_cast<bool>             (map.value("isSupportExported"));

		if (!stlGeometry) {
			std::cout << " ### " << __FUNCTION__ << " ERROR: cannot cast export data to TriangleGeometry." << std::endl;
			return {};
		}

		aiMatrix4x4 aiTransform = *reinterpret_cast<const aiMatrix4x4*>(&transform);
		aiTransform.Transpose();

		scenes.push_back(copyTransformedGeometryScene(stlGeometry, aiTransform));

		data.numModelTriangles += scenes.back()->mMeshes[0]->mNumFaces;

		if (!isSupportExported)
			continue;

		std::cout << " ### " << __FUNCTION__ << " support is being exported for geometry file: " << stlGeometry->getInputFile().toStdString() << std::endl;

		for (const TriangleGeometry* supportGeometry : stlGeometry->getSupportGeometries())
		{
			aiScene* newScene = generateTransformedGeometryScene(supportGeometry, aiTransform);
			scenes.push_back(newScene);
			data.numSupportTriangles += scenes.back()->mMeshes[0]->mNumFaces;
		}
	}
	return scenes;
}


static aiScene* combineScenes(const std::vector<aiScene*>& scenes, aiScene* masterScene = nullptr)
{
	if (masterScene && (scenes.end() == std::find(scenes.begin(), scenes.end(), masterScene)))
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR master scene not found in scenes." << std::endl;
		return nullptr;
	}
	else
	{
		masterScene = scenes[0];
	}

	aiNode* const masterNode = masterScene->mRootNode;

	std::vector<Assimp::AttachmentInfo> sceneAttachments;
	sceneAttachments.reserve(scenes.size());
	for (aiScene* scene: scenes)
	{
		sceneAttachments.emplace_back(Assimp::AttachmentInfo{scene, masterNode});
	}

	aiScene* destReturnScene = new aiScene();
	Assimp::SceneCombiner::MergeScenes(&destReturnScene, masterScene, sceneAttachments);
	return destReturnScene;
}


bool FileImportExport::exportModelsToSTL(const QVariantList& stlExportData, const QString filePath)
{
	assert(stlExportData.size() > 0);

	if (0 == stlExportData.size())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR no export data provided for file: " << filePath.toStdString() << std::endl;
		return false;
	}

	StlTriangleData inputTriangleData;
	std::vector<aiScene*> scenes = generateScenes(stlExportData, inputTriangleData);
	std::cout << " ### " << __FUNCTION__ << " inputTriangleData.numModelTriangles:   " << inputTriangleData.numModelTriangles   << std::endl;
	std::cout << " ### " << __FUNCTION__ << " inputTriangleData.numSupportTriangles: " << inputTriangleData.numSupportTriangles << std::endl;
	std::cout << " ### " << __FUNCTION__ << " inputTriangleData.numStandTriangles:   " << inputTriangleData.numStandTriangles   << std::endl;

	if (scenes.empty())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR no geometry data provided for file: " << filePath.toStdString() << std::endl;
		return false;
	}

	std::cout << " ### " << __FUNCTION__ << " scenes.size(): " << scenes.size() << std::endl;
	std::cout << " ### " << __FUNCTION__ << " scenes.back()->mNumMeshes              : " << scenes.back()->mNumMeshes               << std::endl;
	std::cout << " ### " << __FUNCTION__ << " scenes.back()->mMeshes[0]->mNumFaces   : " << scenes.back()->mMeshes[0]->mNumFaces    << std::endl;
	std::cout << " ### " << __FUNCTION__ << " scenes.back()->mMeshes[0]->mNumVertices: " << scenes.back()->mMeshes[0]->mNumVertices << std::endl;
	const aiScene* destScene = combineScenes(scenes);
	std::cout << " ### " << __FUNCTION__ << " destScene->mNumMeshes              : " << destScene->mNumMeshes << std::endl;
	std::cout << " ### " << __FUNCTION__ << " destScene->mMeshes[0]->mNumFaces   : " << destScene->mMeshes[0]->mNumFaces << std::endl;
	std::cout << " ### " << __FUNCTION__ << " destScene->mMeshes[0]->mNumVertices: " << destScene->mMeshes[0]->mNumVertices << std::endl;

	Assimp::Exporter exporter;
	if (AI_SUCCESS == exporter.Export(destScene, "stlb", filePath.toStdString()))
	{
		std::cout << " ### " << __FUNCTION__ << " filePATH:" << filePath.toStdString() << " export OK" << std::endl;
	}
	else
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR for file:" << filePath.toStdString() << std::endl;
		return false;
	}

	writeStlTriangleData(filePath.toStdString(), inputTriangleData);
	const StlTriangleData outputTriangleData = readStlTriangleData(filePath.toStdString());
	std::cout << " ### " << __FUNCTION__ << " outputTriangleData.numModelTriangles:   " << outputTriangleData.numModelTriangles   << std::endl;
	std::cout << " ### " << __FUNCTION__ << " outputTriangleData.numSupportTriangles: " << outputTriangleData.numSupportTriangles << std::endl;
	std::cout << " ### " << __FUNCTION__ << " outputTriangleData.numStandTriangles:   " << outputTriangleData.numStandTriangles   << std::endl;

	return true;
}
