#include "GCodeGeometry.h"

#include <QRandomGenerator>
#include <QVector3D>
#include <QQuaternion>

#include <assert.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#include <gcode_program.h>
#include <parser.h>

#include <Eigen/Geometry>


using namespace Eigen;

static Vector3f maxFloatBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
static Vector3f minFloatBound(FLT_MAX, FLT_MAX, FLT_MAX);

static Vector3f maxBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
static Vector3f minBound(FLT_MAX, FLT_MAX, FLT_MAX);

// To have QSG included
QT_BEGIN_NAMESPACE

static const std::vector<Vector3f> squareVertices = {{-0.5, 0.0, 0.0},
													 {-0.5, 1.0, 0.0},
													 { 0.5, 1.0, 0.0},
													 { 0.5, 0.0, 0.0}};

static const std::vector<Vector3f> cubeVertices = {//bottom
												   {-0.5, 0.0, -0.5},
												   {-0.5, 1.0, -0.5},
												   { 0.5, 1.0, -0.5},
												   { 0.5, 0.0, -0.5},
												   //top
												   {-0.5, 0.0, 0.5},
												   {-0.5, 1.0, 0.5},
												   { 0.5, 1.0, 0.5},
												   { 0.5, 0.0, 0.5}};

static void updateBounds(const float* vertexMatrixXCoord)
{
	minBound.x() = (std::min(minBound.x(), *vertexMatrixXCoord));
	maxBound.x() = (std::max(maxBound.x(), *vertexMatrixXCoord));
	++vertexMatrixXCoord;
	minBound.y() = (std::min(minBound.y(), *vertexMatrixXCoord));
	maxBound.y() = (std::max(maxBound.y(), *vertexMatrixXCoord));
	++vertexMatrixXCoord;
	minBound.z() = (std::min(minBound.z(), *vertexMatrixXCoord));
	maxBound.z() = (std::max(maxBound.z(), *vertexMatrixXCoord));
}

static void logBounds()
{
	std::cout << " ### aiScene minBound(x,y,z): [" << minBound.x() << "," << minBound.y() << "," << minBound.z() << "]" << std::endl;
	std::cout << " ### aiScene maxBound(x,y,z): [" << maxBound.x() << "," << maxBound.y() << "," << maxBound.z() << "]" << std::endl;
}


static gpr::gcode_program importGCodeFromFile(const std::string& file)
{
	std::ifstream t(file);
	std::string file_contents((std::istreambuf_iterator<char>(t)),
				  std::istreambuf_iterator<char>());

	return gpr::parse_gcode(file_contents);
}

GCodeGeometry::GCodeGeometry()
{
	gpr::gcode_program gcodeProgram = importGCodeFromFile(_inputFile.toStdString());
	createExtruderPaths(gcodeProgram);

	updateData();

	static auto updateDataAndUpdate = [this](){
		updateData();
		update();
	};

	connect(this, &GCodeGeometry::numSubPathsChanged, this, updateDataAndUpdate);
	connect(this, &GCodeGeometry::numPointsInSubPathChanged, this, updateDataAndUpdate);
	connect(this, &GCodeGeometry::numPathPointsUsedChanged, this, updateDataAndUpdate);
}

void GCodeGeometry::dumpSubPath(const std::string& blockString, const std::vector<Vector3f>& subPath)
{
//	std::ofstream pathFile("path" + std::to_string(_extruderPaths.size()) + ".txt");
//	for (const Vector3f& point : subPath)
//	{
//		pathFile << "[" << point.x() << "," << point.y() << "," << point.z() << "]" << std::endl;
//	}
//	pathFile << blockString << std::endl;
//	pathFile.close();
}

void GCodeGeometry::createExtruderPaths(const gpr::gcode_program& gcodeProgram)
{
	std::vector<Vector3f> subPath;
	_extruderSubPaths.push_back(std::vector<Vector3f>());

	unsigned maxPointsInSubPath = 0;

	bool isExtruderOn = false;
	unsigned blockCount = 0;
	const unsigned blockCountLimit = 5220800;
//	const unsigned blockCountLimit = 52208;
//	const unsigned blockCountLimit = 255;
	Vector3f currentCoords(0,0,0);
	std::string blockString;
	for (auto it = gcodeProgram.begin(); it != gcodeProgram.end() && blockCount < blockCountLimit; ++it, ++blockCount)
	{
		const auto& block = *it;
		blockString = block.to_string();
//		std::cout << blockString << std::endl;

		auto setExtrusionOff = [this, &maxPointsInSubPath, &blockString, &blockCount, &subPath, &isExtruderOn]()
		{// If we are setting extrusion off, swap created path with the empty one in path vector.
			if (!isExtruderOn)
				return;
//			std::cout << " ####### setExtrusionOff : " << std::endl;
			isExtruderOn = false;

			if (subPath.empty())
				return;

//			std::cout << " #### adding subPath with: " << subPath.size() << std::endl;
			maxPointsInSubPath = std::max<unsigned>(maxPointsInSubPath, subPath.size());

			dumpSubPath(blockString, subPath);

			std::swap(_extruderSubPaths.back(), subPath);
			_extruderSubPaths.push_back(std::vector<Vector3f>());
		};
		auto setExtrusionOn = [&isExtruderOn]()
		{// If we are setting extrusion on, add new (empty) path to path vector.
			if (isExtruderOn)
				return;
//			std::cout << " ####### setExtrusionOn : " << std::endl;
			isExtruderOn = true;
//			std::cout << " ####### adding empty subPath with: " << std::endl;
		};

		Vector3i coordsSet(0,0,0);
		Vector3f absoluteCoords(0,0,0);

		Vector3f relativeCoords(0,0,0);

		Vector3f* newCoordsPtr = &absoluteCoords;

		if (blockCount > 160)
		{
//			std::cout << "### :" << "" << std::endl;
		}

		for (const gpr::chunk& chunk : block)
		{
			switch(chunk.tp())
			{
				case gpr::CHUNK_TYPE_COMMENT:
				case gpr::CHUNK_TYPE_PERCENT:
				case gpr::CHUNK_TYPE_WORD:
					break;

				case gpr::CHUNK_TYPE_WORD_ADDRESS:
				{
					const gpr::addr& ad = chunk.get_address();
					const char& word = chunk.get_word();

					switch(word)
					{
						case 'G':
							switch (ad.tp())
							{
								case gpr::ADDRESS_TYPE_INTEGER:
									switch (ad.int_value())
									{
										case 0:
											setExtrusionOff();
											break;
										case 1:
											setExtrusionOn();
											break;
										case 90:
											newCoordsPtr = &absoluteCoords;
											break;
										case 91:
											newCoordsPtr = &relativeCoords;
											break;
										default:
											break;
									}
									break;

								case gpr::ADDRESS_TYPE_DOUBLE:
									assert(false);
									break;
							}
							break;
						case 'X':
							newCoordsPtr->x() = float(ad.double_value());
							coordsSet.x() = true;
							break;
						case 'Y':
							newCoordsPtr->y() = float(ad.double_value());
							coordsSet.y() = true;
							break;
						case 'Z':
							newCoordsPtr->z() = float(ad.double_value());
							coordsSet.z() = true;
							break;
						default:
							break;
					}
					break;
				}
			}
		}

		if (coordsSet.isZero())
			continue;

		for (unsigned short i = 0; i < coordsSet.size(); ++i)
		{// If this coord is not set in this block, use the most recent.
			if (coordsSet[i] != 0)
				continue;
			(*newCoordsPtr)[i] = currentCoords[i];
		}

		if (newCoordsPtr != &absoluteCoords)
		{
			currentCoords += *newCoordsPtr;
			absoluteCoords = currentCoords;
		}
		else
		{
			currentCoords = absoluteCoords;
		}

		if (isExtruderOn)
		{
//			std::cout << " ### pushing path point: [" << absoluteCoords.x() << "," << absoluteCoords.y() << "," << absoluteCoords.z() << "]" << std::endl;
			subPath.push_back(absoluteCoords);
		}
	}

	if (!subPath.empty())
	{
		maxPointsInSubPath = std::max<unsigned>(maxPointsInSubPath, subPath.size());
		std::cout << " #### adding subPath no. " << _extruderSubPaths.size() -1<< ", maxPointsInSubPath: " << maxPointsInSubPath << std::endl;

		dumpSubPath(blockString, subPath);

		std::swap(_extruderSubPaths.back(), subPath);
	}

	setNumPointsInSubPath(maxPointsInSubPath);
	setNumSubPaths(_extruderSubPaths.size());
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

bool GCodeGeometry::normals() const { return m_hasNormals; }

void GCodeGeometry::setNormals(bool enable)
{
	if (m_hasNormals == enable)
		return;

    m_hasNormals = enable;
    emit normalsChanged();
    updateData();
    update();
}

float GCodeGeometry::normalXY() const { return m_normalXY; }

void GCodeGeometry::setNormalXY(float xy)
{
	if (m_normalXY == xy)
        return;

    m_normalXY = xy;
    emit normalXYChanged();
    updateData();
    update();
}

bool GCodeGeometry::uv() const { return m_hasUV; }

void GCodeGeometry::setUV(bool enable)
{
	if (m_hasUV == enable)
        return;

    m_hasUV = enable;
    emit uvChanged();
    updateData();
    update();
}

float GCodeGeometry::uvAdjust() const { return m_uvAdjust; }

void GCodeGeometry::setUVAdjust(float f)
{
	if (m_uvAdjust == f)
        return;

    m_uvAdjust = f;
    emit uvAdjustChanged();
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
	return QVector3D(minBound.x(), minBound.y(), minBound.z());
}

QVector3D GCodeGeometry::maxBounds() const
{
	return QVector3D(maxBound.x(), maxBound.y(), maxBound.z());
}

bool GCodeGeometry::isPicked() const
{
	return _isPicked;
}

void GCodeGeometry::setPicked(const bool isPicked)
{
	if (_isPicked == isPicked)
		return;

	_isPicked = isPicked;
	isPickedChanged();
}

void GCodeGeometry::setNumSubPaths(const unsigned num)
{
	if (_numSubPaths == num)
		return;

	_numSubPaths = num;

	emit numSubPathsChanged();
}

unsigned GCodeGeometry::getNumSubPaths() const
{
	return _numSubPaths;
}

void GCodeGeometry::setNumPointsInSubPath(const unsigned num)
{
	if (_numPointsInSubPath == num)
		return;

	_numPointsInSubPath = num;

	emit numPointsInSubPathChanged();
}

unsigned GCodeGeometry::getNumPointsInSubPath() const
{
	return _numPointsInSubPath;
}

void GCodeGeometry::setNumPathPointsUsed(const uint32_t num)
{
	if (_numPathPointsUsed == num)
		return;

	_numPathPointsUsed = num;

	emit numPathPointsUsedChanged();

}

uint32_t GCodeGeometry::getNumPathPointsUsed() const
{
	return _numPathPointsUsed;
}

void GCodeGeometry::generateTriangles()
{
	uint32_t stride = 3 * sizeof(float);
	if (m_hasNormals)
	{
		stride += 3 * sizeof(float);
	}
	if (m_hasUV)
	{
		stride += 2 * sizeof(float);
	}

	size_t numPathPoints = 0;
	size_t numSubPathUsed = std::min<uint32_t>(static_cast<uint32_t>(_extruderSubPaths.size()), _numSubPaths);

	if (numSubPathUsed == 0)
		return;

	for (uint32_t j = 0; j < numSubPathUsed; ++j)
	{
		numPathPoints += std::min<uint32_t>(_numPointsInSubPath, static_cast<uint32_t>(_extruderSubPaths[j].size()));
	}

	if (numPathPoints == 0)
		return;

	std::cout << "### updateData() numSubPathUsed:" << numSubPathUsed << std::endl;
	std::cout << "### updateData() numPathPoints:" << numPathPoints << std::endl;


	const std::vector<Vector3f>& usedStructVertices = _isUsingCubeStruct ? cubeVertices : squareVertices;
	static const ushort verticesPerPathPoint = static_cast<ushort>(usedStructVertices.size());
	_allModelVertices.resize(static_cast<int64_t>(verticesPerPathPoint * numPathPoints * stride));
	float* coordsPtr = reinterpret_cast<float*>(_allModelVertices.data());

	// One rectangle or 6 for cube.
	const ushort numRect = (_isUsingCubeStruct) ? 6u : 1u;
	const ushort numIndexPerRect = 6u;
	_allIndices.resize(static_cast<int64_t>(numPathPoints * numRect * numIndexPerRect * sizeof(uint32_t)));
	uint32_t* indicesPtr = reinterpret_cast<uint32_t*>(_allIndices.data());
	uint32_t totalPrevPointCount = 0;

	static const Vector3f profDiag = _profile[2] - _profile[0];
	Vector3f prevPoint = _extruderSubPaths[0][0];
	for (uint32_t subPathIndex = 0; subPathIndex < numSubPathUsed; ++subPathIndex)
	{
		const std::vector<Vector3f>& path = _extruderSubPaths[subPathIndex];

		if (path.empty())
		{
			std::cout << " ### WARNING empty path " << std::endl;
			continue;
		}

//		uint32_t i = ((j == 0) ? 1 : 0);
		// To get a path rectangle with known length we need the first point of subPath defined.
		// So we also start iterating from the second point.
		prevPoint = path[0];
		for (uint32_t subPathPointIndex = 1; subPathPointIndex < std::min<uint32_t>(_numPointsInSubPath, uint32_t(path.size())); ++subPathPointIndex)
		{
//			const Vector3f boundDiff = maxBound-minBound;
			const Vector3f currPoint = path[subPathPointIndex];
			const Vector3f pathStep = currPoint - prevPoint;
			const float length = pathStep.norm();
			assert(length > FLT_MIN);
			const Matrix3f scale{{profDiag.x(), 0,                 0},
								 {0,            length,            0},
								 {0,            0,      profDiag.y()}};
			Eigen::Quaternionf rotation = Quaternionf::FromTwoVectors(Vector3f{0,1,0}, pathStep);

			std::vector<Vector3f> usedVertices = usedStructVertices;

			std::for_each(usedVertices.begin(), usedVertices.end(), [&scale, &rotation, &prevPoint](Vector3f& v){
				v = rotation*(scale*v) + prevPoint;
			});

			// Set vertex indices as in a rectangle.
			assert(std::numeric_limits<uint32_t>::max() >= static_cast<uint64_t>(totalPrevPointCount + subPathPointIndex -1) * static_cast<uint64_t>(usedVertices.size()));
			const uint32_t firstStructIndexInPathStep = (totalPrevPointCount + subPathPointIndex -1) * static_cast<uint32_t>(usedVertices.size());
			auto setTriangleVertexCoords = [&coordsPtr, &usedVertices](const unsigned index) {
				const Vector3f vertex = usedVertices[index];
				*coordsPtr++ = vertex.x();
				*coordsPtr++ = vertex.y();
				*coordsPtr++ = vertex.z();
				updateBounds(coordsPtr-3);
			};

			auto setTriangleVertexIndex = [&indicesPtr, firstStructIndexInPathStep](const unsigned structIndex) {
				*indicesPtr++ = firstStructIndexInPathStep + structIndex;
			};

			auto setQuadVertexCoords = [setTriangleVertexCoords](const std::vector<ushort>&& indices) {
				std::for_each(indices.begin(), indices.end(), setTriangleVertexCoords);
			};

			auto setQuadTriangleIndices = [setTriangleVertexIndex](const std::vector<ushort>&& indices) {
				std::for_each(indices.begin(), indices.end(), setTriangleVertexIndex);
			};

			{// bottom quad - 4 vertices and 6 indices are enough
				setQuadVertexCoords({0,1,2,3});
				setQuadTriangleIndices({0,1,2,0,2,3});
			}

			if (_isUsingCubeStruct)
			{// For a cuboid we need 4 more vertices and 5 more quads
				// VERTICES
				setQuadVertexCoords({4,5,6,7});

				// QUADS
				setQuadTriangleIndices({4,5,6,6,7,4}); // top
				setQuadTriangleIndices({0,1,5,5,4,0}); // left
				setQuadTriangleIndices({3,2,6,6,7,3}); // right
				setQuadTriangleIndices({0,3,7,7,4,0}); // back
				setQuadTriangleIndices({1,2,6,6,5,1}); // front
			}

			prevPoint = path[subPathPointIndex];
		}

		totalPrevPointCount += path.size();
	}
//	std::cout << " ######### bounds x : " << minBound.x() << "," << maxBound.x() << std::endl;
//	std::cout << " ######### bounds y : " << minBound.y() << "," << maxBound.y() << std::endl;
//	std::cout << " ######### bounds z : " << minBound.z() << "," << maxBound.z() << std::endl;
	setBounds({minBound.x(), minBound.y(), minBound.z()}, {maxBound.x(), maxBound.y(),maxBound.z()});

	setStride(static_cast<int32_t>(stride));
	setNumPathPointsUsed(totalPrevPointCount);

	_areTrianglesReady = true;
}

void GCodeGeometry::updateData()
{

	clear();
	setRectProfile(0.4f, 0.28f);

	if (!_areTrianglesReady)
	{
		generateTriangles();
	}

	QByteArray usedVertices(_allModelVertices);
	QByteArray usedIndices(_allIndices);

	// TODO: Refactor - use things initialized previously in generateTriangles().
	const std::vector<Vector3f>& usedStructVertices = _isUsingCubeStruct ? cubeVertices : squareVertices;
	static const ushort verticesPerPathPoint = static_cast<ushort>(usedStructVertices.size());
	usedVertices.resize(static_cast<int64_t>(verticesPerPathPoint * _numPathPointsUsed * uint32_t(stride())));
	const ushort numRect = (_isUsingCubeStruct) ? 6u : 1u;
	const ushort numIndexPerRect = 6u;
	usedIndices.resize(static_cast<int64_t>(_numPathPointsUsed * numRect * numIndexPerRect * sizeof(uint32_t)));

	setVertexData(usedVertices);
	setIndexData(usedIndices);

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

//	geometryNodeDirty();
	emit modelLoaded();
	std::cout << __FUNCTION__ << std::endl;
}

void GCodeGeometry::setRectProfile(const Real width, const Real height)
{
	const Vector3f start = {-width/Real(2.0), -height/Real(2.0), Real(0.0)};
	_profile = {start, start + Vector3f{0.0, height, 0.0}, start + Vector3f{width, height, 0.0}, start + Vector3f{width, 0.0, 0.0}};
};

QT_END_NAMESPACE
