#include "GCodeGeometry.h"

#include <QRandomGenerator>
#include <QVector3D>
#include <QQuaternion>

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>


#include <gcode_program.h>
#include <parser.h>


#include <D:\Projects\qt6\qtquick3d\src\runtimerender\qssgrenderray_p.h>
#include <D:\Projects\qt6\qtquick3d\src\assetimport\qssgmeshbvhbuilder_p.h>

static aiVector3D maxFloatBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
static aiVector3D minFloatBound(FLT_MAX, FLT_MAX, FLT_MAX);

static aiVector3D maxBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
static aiVector3D minBound(FLT_MAX, FLT_MAX, FLT_MAX);

// To have QSG included
QT_BEGIN_NAMESPACE

static void updateBounds(const float* vertexMatrixXCoord)
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

static void logBounds()
{
	std::cout << " ### aiScene minBound(x,y,z): [" << minBound.x << "," << minBound.y << "," << minBound.z << "]" << std::endl;
	std::cout << " ### aiScene maxBound(x,y,z): [" << maxBound.x << "," << maxBound.y << "," << maxBound.z << "]" << std::endl;
}


static bool importGCodeFromFile(const std::string& file)
{
	std::ifstream t(file);
	std::string file_contents((std::istreambuf_iterator<char>(t)),
				  std::istreambuf_iterator<char>());

	gpr::gcode_program p = gpr::parse_gcode(file_contents);

	std::cout << p << std::endl;

	return true;
}

GCodeGeometry::GCodeGeometry()
{
	importGCodeFromFile(_inputFile.toStdString());
	updateData();
}

QQuaternion GCodeGeometry::getRotationFromDirection(const QVector3D& direction, const QVector3D& up)
{
	return QQuaternion::fromDirection(-direction, up);
}

QQuaternion GCodeGeometry::getRotationFromAxes(const QVector3D& axisFrom, const QVector3D& axisTo)
{
	QVector3D axis = getRotationAxis(axisFrom, axisTo);
	float angle = getSmallRotationAngle(axisFrom, axisTo);
	return getRotationFromAxisAndAngle(axis, angle);
}

QQuaternion GCodeGeometry::getRotationFromAxisAndAngle(const QVector3D& axis, const float angle)
{
	return QQuaternion::fromAxisAndAngle(axis, angle);
}

QQuaternion GCodeGeometry::getRotationFromQuaternions(const QQuaternion& current, const QQuaternion& additional)
{
	return current*additional;
}

float GCodeGeometry::getSmallRotationAngle(const QVector3D& from, const QVector3D& to)
{
	return 180.0f/(float)M_PI*acos(QVector3D::dotProduct(from.normalized(), to.normalized()));
}

QVector3D GCodeGeometry::getRotationAxis(const QVector3D& from, const QVector3D& to)
{
	return QVector3D::crossProduct(from, to).normalized();
}

QVector3D GCodeGeometry::getRotationAxis(const QQuaternion& rotation)
{
	float angle;
	QVector3D vector;
	rotation.getAxisAndAngle(&vector, &angle);
	return vector;
}

float GCodeGeometry::getRotationAngle(const QQuaternion& rotation)
{
	float angle;
	QVector3D vector;
	rotation.getAxisAndAngle(&vector, &angle);
	return angle;
}

GCodeGeometry::PickResult GCodeGeometry::getPick(const QVector3D& origin,
												 const QVector3D& direction,
												 const QMatrix4x4& globalTransform)
{
	QSSGRenderRay hitRay(origin, direction);

	qDebug() << " ### globalTransform: " << globalTransform;

	// From tst_picking.cpp: void picking::test_picking()
//	QSSGRenderLayer dummyLayer;
//	const QSSGRenderNode* node === ???;
//	const QSSGRenderModel &model = static_cast<const QSSGRenderModel &>(node);
//	const auto &globalTransform = model.globalTransform;

	QSSGRenderRay::RayData rayData = QSSGRenderRay::createRayData(globalTransform, hitRay);

	QSSGRef<QSSGMeshUtilities::QSSGMeshBuilder> meshBuilder = QSSGMeshUtilities::QSSGMeshBuilder::createMeshBuilder();
	QSSGMeshUtilities::MeshData meshData;

	QByteArray vertexBufferCopy;
	meshData.m_vertexBuffer.resize(vertexData().size());
	memcpy(meshData.m_vertexBuffer.data(), vertexData().data(), vertexData().size());

	QByteArray indexBufferCopy;
	meshData.m_indexBuffer.resize(indexData().size());
	memcpy(meshData.m_indexBuffer.data(), indexData().data(), indexData().size());

//	meshData.m_vertexBuffer = vertexBufferCopy;
//	meshData.m_indexBuffer = indexBuffer();
	qDebug() << " ### indexBuffer().size():" << indexData().size();
	qDebug() << " ### vertexBuffer().size():" << vertexData().size();
	qDebug() << " ### attributeCount():" << attributeCount();
	qDebug() << " ### stride():" << stride();
	meshData.m_stride = stride();
	meshData.m_attributeCount = attributeCount();
	// TODO: Hacking... do it properly.... if needed :)
	meshData.m_attributes[1].semantic = QSSGMeshUtilities::MeshData::Attribute::IndexSemantic;
	meshData.m_attributes[1].offset = 0;
	meshData.m_attributes[1].componentType = QSSGMeshUtilities::MeshData::Attribute::U32Type;

//	for (unsigned i = 0; i < meshData.m_attributeCount; ++i)
//	{
//		meshData.m_attributes[i].semantic = QSSGMeshUtilities::MeshData::Attribute::PositionSemantic;
//		meshData.m_attributes[i].offset = 0;
//		meshData.m_attributes[i].componentType = QSSGMeshUtilities::MeshData::Attribute::F32Type;
//	}
	QString error;
	QSSGMeshUtilities::Mesh* mesh = meshBuilder->buildMesh(meshData, error, QSSGBounds3(minBounds(), maxBounds()));

//	// Return the current mesh.  This is only good for this function call, item may change or be
//    // released
//    // due to any further function calls.
//    Mesh &getMesh() override

	qDebug() << " ### error:" << error;
//	mesh->m_subsets = m_subsets;
//	mesh->m_joints = m_joints;

	auto &outputMesh = meshBuilder->getMesh();
	QSSGMeshBVHBuilder meshBVHBuilder(mesh);
	static const QSSGMeshBVH* bvh = meshBVHBuilder.buildTree();

	QVector<QSSGRenderRay::IntersectionResult> intersections =
		QSSGRenderRay::intersectWithBVHTriangles(rayData, bvh->triangles, 0, bvh->triangles.size());

	if (intersections.size() > 0)
	{
		setPicked(!_isPicked);
	}

	return PickResult();
}

QString GCodeGeometry::getInputFile() const
{
	return _inputFile;
}

void GCodeGeometry::setInputFile(const QString& url)
{
	if (url == _inputFile)
		return;

	_inputFile = url;
	updateData();
	update();
}

void GCodeGeometry::setNormals(bool enable)
{
	if (m_hasNormals == enable)
		return;

    m_hasNormals = enable;
    emit normalsChanged();
    updateData();
    update();
}

void GCodeGeometry::setNormalXY(float xy)
{
    if (m_normalXY == xy)
        return;

    m_normalXY = xy;
    emit normalXYChanged();
    updateData();
    update();
}

void GCodeGeometry::setUV(bool enable)
{
    if (m_hasUV == enable)
        return;

    m_hasUV = enable;
    emit uvChanged();
    updateData();
    update();
}

void GCodeGeometry::setUVAdjust(float f)
{
    if (m_uvAdjust == f)
        return;

    m_uvAdjust = f;
    emit uvAdjustChanged();
    updateData();
    update();
}

void GCodeGeometry::setWarp(float warp)
{
	if (qFuzzyCompare(_warp, warp))
		return;

	_warp = warp;
	emit warpChanged();
	updateData();
    update();
}

void GCodeGeometry::setBounds(const QVector3D& min, const QVector3D& max)
{
	QQuick3DGeometry::setBounds(min, max);

	minBound = {min.x(), min.y(), min.z()};
	maxBound = {max.x(), max.y(), max.z()};

	emit boundsChanged();
}

void GCodeGeometry::setMinBounds(const QVector3D& minBounds)
{
	setBounds(minBounds, maxBounds());
}

void GCodeGeometry::setMaxBounds(const QVector3D& maxBounds)
{
	setBounds(minBounds(), maxBounds);
}

QVector3D GCodeGeometry::minBounds() const
{
	return QVector3D(minBound.x, minBound.y, minBound.z);
}

QVector3D GCodeGeometry::maxBounds() const
{
	return QVector3D(maxBound.x, maxBound.y, maxBound.z);
}

bool GCodeGeometry::isPicked() const
{
	return _isPicked;
}

void GCodeGeometry::updateData()
{
	setRectProfile(0.4, 0.2);
//	setPath();

    int stride = 3 * sizeof(float);
    if (m_hasNormals)
    {
        stride += 3 * sizeof(float);
    }
    if (m_hasUV)
    {
        stride += 2 * sizeof(float);
    }

	unsigned numMeshFaces = 0;//scene->mMeshes[0]->mNumFaces;

    QByteArray v;
	v.resize(3 * numMeshFaces * stride);
	float* p = reinterpret_cast<float*>(v.data());

	QByteArray indices;
	indices.resize(3 * numMeshFaces * sizeof(uint32_t));
	uint32_t* pi = reinterpret_cast<uint32_t*>(indices.data());

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
//		const aiFace& face = scene->mMeshes[0]->mFaces[i];


//		const aiVector3D boundDiff = maxBound-minBound;

//		auto setTriangleVertex = [this, &p, &pi, &boundDiff](unsigned vertexIndex) {
//			const aiVector3D vertex;// = scene->mMeshes[0]->mVertices[vertexIndex];
//			*p++ = vertex.x + _warp*boundDiff.x*sin(vertex.z/2);
//			*p++ = vertex.y;
//			*p++ = vertex.z;
//			*pi++ = vertexIndex;
//			updateBounds(p-3);
//		};

//		setTriangleVertex(face.mIndices[0]);
//		setTriangleVertex(face.mIndices[1]);
//		setTriangleVertex(face.mIndices[2]);
	}
	setBounds({minBound.x, minBound.y, minBound.z}, {maxBound.x, maxBound.y,maxBound.z});

    setVertexData(v);
	setIndexData(indices);
    setStride(stride);

    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);

	addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
				 0,
				 QQuick3DGeometry::Attribute::U32Type);

//    if (m_hasNormals) {
//        addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,
//                     3 * sizeof(float),
//                     QQuick3DGeometry::Attribute::F32Type);
//    }

//    if (m_hasUV) {
//        addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic,
//                     m_hasNormals ? 6 * sizeof(float) : 3 * sizeof(float),
//                     QQuick3DGeometry::Attribute::F32Type);
//    }

	geometryNodeDirty();
}

void GCodeGeometry::setRectProfile(const Real width, const Real height)
{
	const aiVector3D start = {-width/Real(2.0), -height/Real(2.0), Real(0.0)};
//	_profile = {start, start + aiVector3D{0.0, height, 0.0}, start + aiVector3D{width, height, 0.0}, start + aiVector3D{width, 0.0, 0.0}};
};

QT_END_NAMESPACE
