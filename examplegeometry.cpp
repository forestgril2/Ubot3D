#include "examplegeometry.h"

#include <QRandomGenerator>
#include <QVector3D>
#include <QQuaternion>

#include <cstdio>
#include <iostream>
#include <string>

// ASSIMP LIBRARY INCLUDE
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>

//#include "assimphelpers.h"

static bool isAssimpReadDone = false;
// Create an instance of the Importer class
static Assimp::Importer importer;
static const aiScene* scene;
static aiVector3D maxBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
static aiVector3D minBound(FLT_MAX, FLT_MAX, FLT_MAX);

void assimpErrorLogging(const std::string&& pError)
{
	std::cout << __FUNCTION__ << " : " << pError << std::endl;
}

static void assimpLogScene(const aiScene* scene)
{
	std::cout << "DoTheSceneProcessing(), numMeshes:                       " << scene->mNumMeshes << std::endl;
	std::cout << "DoTheSceneProcessing(), scene->mMeshes[0]->mNumFaces:    " << scene->mMeshes[0]->mNumFaces << std::endl;
	std::cout << "DoTheSceneProcessing(), scene->mMeshes[0]->mNumVertices: " << scene->mMeshes[0]->mNumVertices << std::endl;
}

void updateBounds(const float* vertexMatrixXCoord)
{
	minBound.x = (std::min(minBound.x, *vertexMatrixXCoord));
	maxBound.x = (std::max(maxBound.x, *vertexMatrixXCoord));
	++vertexMatrixXCoord;
	minBound.y = (std::min(minBound.y, *vertexMatrixXCoord));
	maxBound.y = (std::max(maxBound.y, *vertexMatrixXCoord));
	++vertexMatrixXCoord;
	minBound.z = (std::min(minBound.z, *vertexMatrixXCoord));
	maxBound.z = (std::max(maxBound.z, *vertexMatrixXCoord));
}

void logBounds()
{
	std::cout << " ### aiScene minBound(x,y,z): [" << minBound.x << "," << minBound.y << "," << minBound.z << "]" << std::endl;
	std::cout << " ### aiScene maxBound(x,y,z): [" << maxBound.x << "," << maxBound.y << "," << maxBound.z << "]" << std::endl;
}

static void assimpReadMeshBounds(const aiScene* scene, const unsigned meshIndex = 0u)
{
	const unsigned numMeshVertices = scene->mMeshes[meshIndex]->mNumVertices;
	for (unsigned i = 0; i < numMeshVertices; ++i)
	{
		updateBounds(&(scene->mMeshes[meshIndex]->mVertices[i].x));
	}
	logBounds();
}

bool DoTheImportThing(const std::string& pFile)
{
	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	scene = importer.ReadFile(pFile,
							  aiProcess_CalcTangentSpace |
							  aiProcess_Triangulate |
							  aiProcess_JoinIdenticalVertices |
							  aiProcess_SortByPType);
	// If the import failed, report it
	if(!scene)
	{
		assimpErrorLogging(std::string(importer.GetErrorString()));
		return false;
	}

	assimpReadMeshBounds(scene);
	assimpLogScene(scene);

	isAssimpReadDone = true;
	// We're done. Everything will be cleaned up by the importer destructor
	return true;
}

ExampleTriangleGeometry::ExampleTriangleGeometry()
{
    updateData();
}

void ExampleTriangleGeometry::setNormals(bool enable)
{
    if (m_hasNormals == enable)
        return;

    m_hasNormals = enable;
    emit normalsChanged();
    updateData();
    update();
}

void ExampleTriangleGeometry::setNormalXY(float xy)
{
    if (m_normalXY == xy)
        return;

    m_normalXY = xy;
    emit normalXYChanged();
    updateData();
    update();
}

void ExampleTriangleGeometry::setUV(bool enable)
{
    if (m_hasUV == enable)
        return;

    m_hasUV = enable;
    emit uvChanged();
    updateData();
    update();
}

void ExampleTriangleGeometry::setUVAdjust(float f)
{
    if (m_uvAdjust == f)
        return;

    m_uvAdjust = f;
    emit uvAdjustChanged();
    updateData();
    update();
}

void ExampleTriangleGeometry::setWarp(float warp)
{
	if (qFuzzyCompare(_warp, warp))
		return;

	_warp = warp;
	emit warpChanged();
	updateData();
    update();
}

void ExampleTriangleGeometry::setBounds(const QVector3D& min, const QVector3D& max)
{
	QQuick3DGeometry::setBounds(min, max);

	minBound = {min.x(), min.y(), min.z()};
	maxBound = {max.x(), max.y(), max.z()};

	emit boundsChanged();
}

void ExampleTriangleGeometry::setMinBounds(const QVector3D& minBounds)
{
	setBounds(minBounds, maxBounds());
}

void ExampleTriangleGeometry::setMaxBounds(const QVector3D& maxBounds)
{
	setBounds(minBounds(), maxBounds);
}

QVector3D ExampleTriangleGeometry::minBounds() const
{
	return QVector3D(minBound.x, minBound.y, minBound.z);
}

QVector3D ExampleTriangleGeometry::maxBounds() const
{
	return QVector3D(maxBound.x, maxBound.y, maxBound.z);
}

void ExampleTriangleGeometry::updateData()
{
	if (!isAssimpReadDone)
	{
		if (!DoTheImportThing("C:/ProjectsData/stl_files/mandorifle.stl"))
			return;

		assimpLogScene(scene);
		setBounds({minBound.x, minBound.y, minBound.z}, {maxBound.x, maxBound.y,maxBound.z});
	}

	clear();

    int stride = 3 * sizeof(float);
    if (m_hasNormals)
    {
        stride += 3 * sizeof(float);
    }
    if (m_hasUV)
    {
        stride += 2 * sizeof(float);
    }

	unsigned numMeshFaces = scene->mMeshes[0]->mNumFaces;

    QByteArray v;
    v.resize(3 * numMeshFaces * stride);
    float *p = reinterpret_cast<float *>(v.data());

    // a triangle, front face = counter-clockwise
//    *p++ = -1.0f; *p++ = -1.0f; *p++ = 0.0f;
//    if (m_hasNormals) {
//        *p++ = m_normalXY; *p++ = m_normalXY; *p++ = 1.0f;
//    }
//    if (m_hasUV) {
//        *p++ = 0.0f + m_uvAdjust; *p++ = 0.0f + m_uvAdjust;
//    }
//    *p++ = 1.0f; *p++ = -1.0f; *p++ = 0.0f;
//    if (m_hasNormals) {
//        *p++ = m_normalXY; *p++ = m_normalXY; *p++ = 1.0f;
//    }
//    if (m_hasUV) {
//        *p++ = 1.0f - m_uvAdjust; *p++ = 0.0f + m_uvAdjust;
//    }
//    *p++ = 0.0f; *p++ = 1.0f; *p++ = 0.0f;
//    if (m_hasNormals) {
//        *p++ = m_normalXY; *p++ = m_normalXY; *p++ = 1.0f;
//    }
//    if (m_hasUV) {
//        *p++ = 1.0f - m_uvAdjust; *p++ = 1.0f - m_uvAdjust;
//    }

// a triangle, front face = counter-clockwise

	for (unsigned i = 0; i < numMeshFaces; ++i)
	{
		const aiFace& face = scene->mMeshes[0]->mFaces[i];

		if (face.mNumIndices != 3)
		{
			std::cout << "#### WARNING! face.mNumIndices != 3, but " << face.mNumIndices << std::endl;
		}

		const aiVector3D boundDiff = maxBound-minBound;

		auto setTriangleVertex = [this, &p, &boundDiff](unsigned vertexIndex) {
			const aiVector3D vertex = scene->mMeshes[0]->mVertices[vertexIndex];
			*p++ = vertex.x + _warp*boundDiff.x*sin(vertex.z/2);
			*p++ = vertex.y;
			*p++ = vertex.z;
			updateBounds(p-3);
		};

		setTriangleVertex(face.mIndices[0]);
		setTriangleVertex(face.mIndices[1]);
		setTriangleVertex(face.mIndices[2]);
	}
	setBounds({minBound.x, minBound.y, minBound.z}, {maxBound.x, maxBound.y,maxBound.z});

    setVertexData(v);
    setStride(stride);

    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);

    if (m_hasNormals) {
        addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,
                     3 * sizeof(float),
                     QQuick3DGeometry::Attribute::F32Type);
    }

    if (m_hasUV) {
        addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic,
                     m_hasNormals ? 6 * sizeof(float) : 3 * sizeof(float),
                     QQuick3DGeometry::Attribute::F32Type);
    }
}

ExamplePointGeometry::ExamplePointGeometry()
{
    updateData();
}

void ExamplePointGeometry::updateData()
{
    clear();

    const int N = 2000;

    const int stride = 3 * sizeof(float);

    QByteArray v;
    v.resize(N * stride);
    float *p = reinterpret_cast<float *>(v.data());

    for (int i = 0; i < N; ++i) {
        const float x = float(QRandomGenerator::global()->bounded(200.0f) - 100.0f) / 20.0f;
        const float y = float(QRandomGenerator::global()->bounded(200.0f) - 100.0f) / 20.0f;
        *p++ = x;
        *p++ = y;
        *p++ = 0.0f;
    }

    setVertexData(v);
    setStride(stride);

    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Points);

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);
}
