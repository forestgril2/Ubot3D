#include "FileImportExport.h"

#include <fstream>
#include <iostream>
#include <vector>

#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/cexport.h>
#include <assimp/SceneCombiner.h>

#include <Helpers3D.h>
#include <TriangleGeometry.h>

static constexpr uint32_t kStlHeaderSize = 80u;

struct Face
{
	uint32_t indices[3] = {0u, 0u, 0u};
};

struct StlHeaderData
{
	uint32_t numModelTriangles   = 0u;
	uint32_t numSupportTriangles = 0u;
	uint32_t numStandTriangles   = 0u;
	char unused[kStlHeaderSize - (sizeof(numModelTriangles) +
								  sizeof(numSupportTriangles) +
								  sizeof(numStandTriangles))];
	uint32_t numTotalTriangles   = 0u;
};

struct StlTriangleData
{
	float normal[3] = {0.0f, 0.0f, 0.0f};
	// Vertices
	float v0[3]     = {0.0f, 0.0f, 0.0f};
	float v1[3]     = {0.0f, 0.0f, 0.0f};
	float v2[3]     = {0.0f, 0.0f, 0.0f};

	uint16_t attributes = 0u;
};

static StlHeaderData readStlTriangleData(const std::string& filePath)
{
	std::vector<uint8_t> inputBuffer(sizeof(StlHeaderData));
	std::basic_ifstream<uint8_t> stream(filePath);

	if (!stream.is_open())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR: cannot open file for reading: " << filePath << std::endl;
		return StlHeaderData();
	}

	stream.read(&inputBuffer[0], sizeof(StlHeaderData));
	return *reinterpret_cast<StlHeaderData*>(&inputBuffer[0]);
}

static bool writeStlHeaderData(const std::string& filePath, const StlHeaderData& data)
{
	std::basic_fstream<uint8_t> stream(filePath, std::ios::binary | std::ios::out | std::ios::in);

	if (!stream.is_open())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR: cannot open file for writing: " << filePath << std::endl;
		return false;
	}

	stream.seekp(0, std::ios_base::beg);
	stream.write(reinterpret_cast<const uint8_t*>(&data), sizeof(data));
	stream.close();

	return true;
}

static bool writeStlTrianglesData(const std::string& filePath, const std::vector<StlTriangleData>& data)
{
	std::basic_fstream<uint8_t> stream(filePath, std::ios::binary | std::ios::out | std::ios::in);

	if (!stream.is_open())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR: cannot open file for writing: " << filePath << std::endl;
		return false;
	}

	stream.seekp(sizeof(StlHeaderData), std::ios_base::beg);
	stream.write(reinterpret_cast<const uint8_t*>(data.data()), sizeof(data));
	stream.close();

	return true;
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

static bool generateScenes(const QVariantList& stlExportData, std::vector<aiScene*>& modelScenes,
															  std::vector<aiScene*>& supportScenes)
{
	for (const QVariant& exportData : stlExportData)
	{
		QVariantMap map = exportData.toMap();
		const TriangleGeometry* stlGeometry = qvariant_cast<TriangleGeometry*>(map.value("geometry"));
		const QMatrix4x4 transform          = qvariant_cast<QMatrix4x4>       (map.value("transform"));
		const bool isSupportExported        = qvariant_cast<bool>             (map.value("isSupportExported"));

		if (!stlGeometry) {
			std::cout << " ### " << __FUNCTION__ << " ERROR: cannot cast export data to TriangleGeometry." << std::endl;
			return false;
		}

		aiMatrix4x4 aiTransform = *reinterpret_cast<const aiMatrix4x4*>(&transform);
		aiTransform.Transpose();

		modelScenes.push_back(copyTransformedGeometryScene(stlGeometry, aiTransform));

		if (!isSupportExported)
			continue;

		std::cout << " ### " << __FUNCTION__ << " support is being exported for geometry file: " << stlGeometry->getInputFile().toStdString() << std::endl;

		for (const TriangleGeometry* supportGeometry : stlGeometry->getSupportGeometries())
		{
			aiScene* newScene = generateTransformedGeometryScene(supportGeometry, aiTransform);
			supportScenes.push_back(newScene);
		}
	}
	return true;
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

static bool assimpExportScenes(const std::vector<aiScene*>& scenes, const std::string& filePath)
{
	const aiScene* destScene = combineScenes(scenes);
	Assimp::Exporter exporter;
	if (AI_SUCCESS == exporter.Export(destScene, "stlb", filePath))
	{
		std::cout << " ### " << __FUNCTION__ << " filePATH:" << filePath << " export OK" << std::endl;
	}
	else
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR for file:" << filePath << std::endl;
		return false;
	}
}

static void getScenesFaceAndVertexCounts(const std::vector<aiScene*>& assimpScenes,
										 uint32_t& numFacesTotal,
										 uint32_t& numVerticesTotal)
{
	for (const aiScene* scene : assimpScenes)
	{
		Helpers3D::countAssimpFacesAndVertices(scene, numFacesTotal, numVerticesTotal);
	}
};

static std::vector<uint32_t>& getSceneIndices(const aiScene* scene,
											  std::vector<uint32_t>& indices,
											  const uint32_t numNewIndices = 0)
{
	if (indices.size() + numNewIndices > indices.capacity())
	{
		indices.reserve(indices.size() + numNewIndices);
	}
	const uint32_t offset = static_cast<uint32_t>(indices.size());
	for (uint32_t m=0; m<scene->mNumMeshes; ++m)
	{
		const aiMesh* mesh = scene->mMeshes[m];
		for (uint32_t f=0; f<mesh->mNumFaces; ++m)
		{
			const aiFace& face = mesh->mFaces[f];
			assert(3 == face.mNumIndices);
			indices.push_back(offset + face.mIndices[0]);
			indices.push_back(offset + face.mIndices[1]);
			indices.push_back(offset + face.mIndices[2]);
		}
	}
	return indices;
};

static TriangleGeometryData getCombinedScenesTriangleData(const std::vector<aiScene*>& scenes, const uint32_t finalIndicesSize = 0)
{
	TriangleGeometryData data;
	data.indices.reserve(finalIndicesSize);
	for (const aiScene* scene : scenes)
	{
		Helpers3D::getContiguousAssimpVerticesAndNormals(scene, data.vertices, data.normals);
		getSceneIndices(scene, data.indices);
	}

	return data;
};

static StlHeaderData getStlHeaderFromAssimpScenes(const std::vector<aiScene*>& modelScenes,
												  const std::vector<aiScene*>& supportScenes = {},
												  const std::vector<aiScene*>& standScenes = {})
{
	uint32_t numModelTriangles = 0;
	uint32_t numModelVertices = 0;
	getScenesFaceAndVertexCounts(modelScenes,
								 numModelTriangles,
								 numModelVertices);

	uint32_t numSupportTriangles = 0;
	uint32_t numSupportVertices = 0;
	getScenesFaceAndVertexCounts(supportScenes,
								 numSupportTriangles,
								 numSupportVertices);

	uint32_t numStandTriangles = 0;
	uint32_t numStandVertices = 0;
	getScenesFaceAndVertexCounts(standScenes,
								 numStandTriangles,
								 numStandVertices);

	const uint32_t numTotalTriangles = numModelTriangles + numSupportTriangles + numStandTriangles;
	return {numModelTriangles, numSupportTriangles, numStandTriangles, {}, numTotalTriangles};
}

static bool exportAssimpScenesAsStl(const std::string& filePath,
									const std::vector<aiScene*>& modelScenes,
									const std::vector<aiScene*>& supportScenes = {},
									const std::vector<aiScene*>& standScenes = {})
{
	const StlHeaderData headerData = getStlHeaderFromAssimpScenes(modelScenes, supportScenes, standScenes);

	const TriangleGeometryData modelData   = getCombinedScenesTriangleData(modelScenes, headerData.numModelTriangles);
	const TriangleGeometryData supportData = getCombinedScenesTriangleData(supportScenes, headerData.numSupportTriangles);
	const TriangleGeometryData standData   = getCombinedScenesTriangleData(standScenes, headerData.numStandTriangles);

	std::vector<StlTriangleData> triangles;
	const uint32_t numTotalIndices = static_cast<uint32_t>(modelData.indices.size() +
														   modelData.indices.size() +
														   modelData.indices.size());
	assert(0 == numTotalIndices%3);
	triangles.reserve(numTotalIndices/3);

	auto addExportTriangles = [](const TriangleGeometryData& data, std::vector<StlTriangleData>& triangles) {
		for (uint32_t i=0; i<data.indices.size(); i+=3)
		{
			const Vec3& v0 = data.vertices[i   ];
			const Vec3& v1 = data.vertices[i +1];
			const Vec3& v2 = data.vertices[i +2];
			const Vec3 n = (v1-v0).cross(v2-v0).normalized();
			triangles.emplace_back(StlTriangleData{{n .x(), n .y(), n .z()},
												   {v0.x(), v0.y(), v0.z()},
												   {v1.x(), v1.y(), v1.z()},
												   {v2.x(), v2.y(), v2.z()},
												   0});
		}
	};

	addExportTriangles(modelData,   triangles);
	addExportTriangles(supportData, triangles);
	addExportTriangles(standData,   triangles);

	if (!writeStlHeaderData(filePath, headerData) || !writeStlTrianglesData(filePath, triangles))
		return false;

	return true;
}

bool FileImportExport::exportModelsToSTL(const QVariantList& stlExportData, const QString filePath)
{
	assert(stlExportData.size() > 0);

	if (0 == stlExportData.size())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR no export data provided for file: " << filePath.toStdString() << std::endl;
		return false;
	}

	std::vector<aiScene*> modelScenes;
	std::vector<aiScene*> supportScenes;
	if (!generateScenes(stlExportData, modelScenes, supportScenes) || modelScenes.empty())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR no geometry data provided for file: " << filePath.toStdString() << std::endl;
		return false;
	}

	// TODO: Assimp exporter doesn't work correctly in release 5.0.1. It exports only master scene
	//       for app in Release  mode and twice the master scene in Debug. (https://github.com/assimp/assimp/issues/203)
//	assimpExportScenes(modelScenes, filePath.toStdString());

	if (!exportAssimpScenesAsStl(filePath.toStdString(), modelScenes, supportScenes));
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR: export failed." << std::endl;
		return false;
	}

	const StlHeaderData outputTriangleData = readStlTriangleData(filePath.toStdString());
	std::cout << " ### " << __FUNCTION__ << " outputTriangleData.numModelTriangles:   " << outputTriangleData.numModelTriangles   << std::endl;
	std::cout << " ### " << __FUNCTION__ << " outputTriangleData.numSupportTriangles: " << outputTriangleData.numSupportTriangles << std::endl;
	std::cout << " ### " << __FUNCTION__ << " outputTriangleData.numStandTriangles:   " << outputTriangleData.numStandTriangles   << std::endl;
	std::cout << " ### " << __FUNCTION__ << " outputTriangleData.numTotalTriangles:   " << outputTriangleData.numTotalTriangles   << std::endl;

	return true;
}
