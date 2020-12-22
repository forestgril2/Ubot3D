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
#include <chrono>

#include <Chronograph.h>

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

template <typename T>
static std::vector<T> range(const unsigned N) {
	std::vector<T> out(N);
	std::iota(out.begin(), out.end(), 0);
	return out;
};

template <typename T>
static std::vector<T> range(const T start, const unsigned N) {
	std::vector<T> out(N);
	std::iota(out.begin(), out.end(), start);
	return out;
};

template <typename T>
static void range_test()
{
	assert(range<T>(0).size() == 0);
	assert(range<T>(1).size() == 1);
	assert(range<T>(100).size() == 100);
	assert(range<T>(100)[0] == 0);
	assert(range<T>(100)[99] == 99);

	assert(range<T>(1, 0).size() == 0);
	assert(range<T>(1, 1).size() == 1);
	assert(range<T>(1, 100).size() == 100);
	assert(range<T>(1, 100)[0] == 1);
	assert(range<T>(1, 100)[99] == 100);
//    std::cout << " ### " << __FUNCTION__  << ": OK!" << std::endl;
}


static const Vertices _squareVertices = {{-0.5, 0.0, 0.0},
										 {-0.5, 1.0, 0.0},
										 { 0.5, 1.0, 0.0},
										 { 0.5, 0.0, 0.0}};

static const Vertices _cubeVertices = {//bottom
									   {-0.5, 0.0, -0.5},
									   {-0.5, 1.0, -0.5},
									   { 0.5, 1.0, -0.5},
									   { 0.5, 0.0, -0.5},
									   //top
									   {-0.5, 0.0, 0.5},
									   {-0.5, 1.0, 0.5},
									   { 0.5, 1.0, 0.5},
									   { 0.5, 0.0, 0.5}};

static Vertices _cylinderVertices;
static void generateCylinderVertices(const float radius, const float height, const unsigned short numCirclePoints)
{

}

static std::pair<Vertices, Indices> generateCirclePies(const Point& center,
													   const Vector3f& radiusStart,
													   const Vector3f& radiusEnd,
													   const float height,
													   unsigned short numCirclePoints = 20)
{
	Vertices vert{center};
	Indices ind;
	const float angle = atan2(radiusEnd.y(), radiusEnd.x()) - atan2(radiusStart.y(), radiusStart.x());
	const float angleStep = angle / float(numCirclePoints);
	for(float p : range<float>(numCirclePoints+1))
	{
		const float cos = std::cosf(p*angleStep);
		const float sin = std::sinf(p*angleStep);
		vert.push_back(center + Matrix3f{{cos , -sin, 0},
										 {sin,   cos, 0},
										 {0,       0, 1}} * radiusStart);

		if (unsigned(p) == numCirclePoints)
		{// We only need the last vertex from the final iteration.
			break;
		}

		ind.push_back(0);
		ind.push_back(uint32_t(p)+1);
		ind.push_back(uint32_t(p)+2);
	}

	// Copy thecircle  pie shifted by height in z-direction.
	const Vector3f heightShift{0,0,height};
	const uint32_t upperCircleCenterIndex = uint32_t(vert.size());
	const uint32_t firstIndicesSize = uint32_t(ind.size());
	vert.resize(2*upperCircleCenterIndex);
	ind.resize(2*firstIndicesSize);
	for (uint32_t i = 0, j = upperCircleCenterIndex; i < upperCircleCenterIndex; ++i, ++j)
	{
		vert[j] = vert[i] + heightShift;
	}
	for (uint32_t i = 0, j = firstIndicesSize; i < firstIndicesSize; ++i, ++j)
	{
		ind[j] = ind[i] + upperCircleCenterIndex;
	}

	// Setup pie cylinder side part indices, 1 quad == 2 triangles per each point
	const uint32_t secondIndSize = uint32_t(ind.size());
	ind.resize(ind.size() + (6 * numCirclePoints));
	uint32_t* triangleIndexPtr = &ind[secondIndSize];
	for (uint32_t i = 1, j = firstIndicesSize +1; i < 3*numCirclePoints; i+=3, j+=3)
	{
		*triangleIndexPtr++ = ind[i];
		*triangleIndexPtr++ = ind[j];
		*triangleIndexPtr++ = ind[i+1];
		*triangleIndexPtr++ = ind[i+1];
		*triangleIndexPtr++ = ind[j+1];
		*triangleIndexPtr++ = ind[j];
	}

	return {vert, ind};
}

static const bool _isUsingCubeStruct = true;

static const Vertices& usedStructVertices = _isUsingCubeStruct ? _cubeVertices : _squareVertices;


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
	Chronograph chronograph("Read gcode file contents", true);
	std::ifstream t(file);
	std::string file_contents((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	chronograph.log();

	chronograph.start("Parse gcode");
	return gpr::parse_gcode(file_contents);
}

void GCodeGeometry::loadGCodeProgram()
{
	gpr::gcode_program gcodeProgram = importGCodeFromFile(_inputFile.toStdString());
	createExtruderPaths(gcodeProgram);
}

GCodeGeometry::GCodeGeometry()
{
	range_test<unsigned>();
	range_test<float>();

	setRectProfile(0.4f, 0.28f);
	const short unsigned numOfPointsInCylinderCircleBase = 20;
	static const Vector3f profileDiag = _profile[2] - _profile[0];
	const float height = profileDiag.y();
	const float radius = 0.5f * profileDiag.x();
	generateCylinderVertices(radius, height, numOfPointsInCylinderCircleBase);

	loadGCodeProgram();
	updateData();
	update();
	emit modelLoaded();

	static auto updateDataAndUpdate = [this](){
		updateData();
		update();
	};

	connect(this, &GCodeGeometry::numSubPathsChanged, this, updateDataAndUpdate);
	connect(this, &GCodeGeometry::numPointsInSubPathChanged, this, updateDataAndUpdate);
	connect(this, &GCodeGeometry::numPathPointsUsedChanged, this, updateDataAndUpdate);
}

void GCodeGeometry::dumpSubPath(const std::string& blockString, const Points& subPath)
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
	Chronograph chronograph(__FUNCTION__, true);

	Points subPath;
	_extruderSubPaths.clear();
	_extruderSubPaths.push_back(Points());

	unsigned maxPointsInSubPath = 0;

	bool isExtruderOn = false;
	bool isAbsoluteMode = true;
	unsigned blockCount = 0;
	const unsigned blockCountLimit = 5220800;
//	const unsigned blockCountLimit = 52208;
//	const unsigned blockCountLimit = 255;
	std::string blockString;

	Point lastAbsCoords(0,0,0);
	Point blockAbsCoords(0,0,0);
	Point* newCoordsPtr = &blockAbsCoords;
	for (auto it = gcodeProgram.begin(); it != gcodeProgram.end() && blockCount < blockCountLimit; ++it, ++blockCount)
	{
		const auto& block = *it;
		blockString = block.to_string();
		Point blockRelativeCoords(0,0,0);

		auto setExtrusionOff = [this, &maxPointsInSubPath, &blockString, &subPath, &isExtruderOn]()
		{// If we are setting extrusion off, swap created path with the empty one in path vector.
			if (!isExtruderOn)
				return;
			isExtruderOn = false;

			if (subPath.empty())
				return;
			maxPointsInSubPath = std::max<unsigned>(maxPointsInSubPath, subPath.size());

			dumpSubPath(blockString, subPath);

			if (subPath.size() < 2)
			{
				static bool wasSubpathOfSmallSizeAnounced = false;
				if (!wasSubpathOfSmallSizeAnounced)
				{
					std::cout << "### WARNING: subpath of size: " << subPath.size() << std::endl;
					wasSubpathOfSmallSizeAnounced = true;
				}

				subPath.clear();
				return;
			}

			std::swap(_extruderSubPaths.back(), subPath);
			_extruderSubPaths.push_back(Points());
		};

		auto setExtrusionOn = [&subPath, &isExtruderOn](const Point& lastAbsCoords)
		{// If we are setting extrusion on, add new (empty) path to path vector.
			if (isExtruderOn)
				return;
			isExtruderOn = true;
			subPath.push_back(lastAbsCoords);
		};

		auto setAbsoluteModeOn = [&isAbsoluteMode, &newCoordsPtr, &blockAbsCoords]() {
			if (isAbsoluteMode)
				return;
			isAbsoluteMode = true;
			newCoordsPtr = &blockAbsCoords;
		};

		auto setAbsoluteModeOff = [&isAbsoluteMode, &newCoordsPtr, &blockRelativeCoords]() {
			if (!isAbsoluteMode)
				return;

			isAbsoluteMode = false;
			newCoordsPtr = &blockRelativeCoords;
		};

		Vector3i wichCoordsSetInBlock(0,0,0);

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
											setExtrusionOn(lastAbsCoords);
											break;
										case 90:
											setAbsoluteModeOn();
											break;
										case 91:
											setAbsoluteModeOff();
											break;
										case 92:
											//TODO: IMPLEMENT.
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
							wichCoordsSetInBlock.x() = true;
							break;
						case 'Y':
							newCoordsPtr->y() = float(ad.double_value());
							wichCoordsSetInBlock.y() = true;
							break;
						case 'Z':
							newCoordsPtr->z() = float(ad.double_value());
							wichCoordsSetInBlock.z() = true;
							break;
						default:
							break;
					}
					break;
				}
			}
		}

		if (wichCoordsSetInBlock.isZero())
			continue;

		for (unsigned short i = 0; i < wichCoordsSetInBlock.size(); ++i)
		{// If this coord is not set in this block, use the most recent for absolute coordinates.
			if (wichCoordsSetInBlock[i] != 0 || !isAbsoluteMode)
				continue;
			blockAbsCoords[i] = lastAbsCoords[i];
		}

		if (!isAbsoluteMode)
		{
			lastAbsCoords += blockRelativeCoords;
		}
		else
		{
			lastAbsCoords = blockAbsCoords;
		}

		if (isExtruderOn)
		{
			subPath.push_back(lastAbsCoords);
		}
	}

	if (!subPath.empty())
	{
		maxPointsInSubPath = std::max<unsigned>(maxPointsInSubPath, subPath.size());
		std::cout << " #### adding subPath no. " << _extruderSubPaths.size() -1<< ", maxPointsInSubPath: " << maxPointsInSubPath << std::endl;

		dumpSubPath(blockString, subPath);

		std::swap(_extruderSubPaths.back(), subPath);
	}

	_numPointsInSubPath = maxPointsInSubPath;
	_numSubPaths = _extruderSubPaths.size();
}

QString GCodeGeometry::getInputFile() const
{
	return _inputFile;
}

void GCodeGeometry::setInputFile(const QString& url)
{
	std::cout << "### " << __FUNCTION__ << std::endl;
	if (url == _inputFile)
		return;

	_inputFile = url;
	_areTrianglesReady = false;
	loadGCodeProgram();
	updateData();
	std::cout << "### isComponentComplete:" << isComponentComplete() << std::endl;
	update();

	emit modelLoaded();
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

void GCodeGeometry::setNumPathStrokesUsed(const uint32_t num)
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

void GCodeGeometry::generateSubPathTriangles(const Point& prevPoint,
											 const Vector3f& pathStep,
											 const uint32_t firstStructIndexInPathStep,
											 float*& coordsPtr,
											 uint32_t*& indicesPtr)
{
	const float length = pathStep.norm();
	assert(length > FLT_MIN);
	static const Vector3f profileDiag = _profile[2] - _profile[0];
	const Matrix3f scale{{profileDiag.x(), 0,      0              },
						 {0,               length, 0              },
						 {0,               0,      profileDiag.y()}};
	const Eigen::Quaternionf rotation = Quaternionf::FromTwoVectors(Vector3f{0,1,0}, pathStep);

	Vertices vertices = usedStructVertices;

	std::for_each(vertices.begin(), vertices.end(), [&scale, &rotation, &prevPoint](Vector3f& v){
		v = rotation*(scale*v) + prevPoint;
	});

	// Set vertex indices as in a rectangle.
	auto setTriangleVertexCoords = [&coordsPtr, &vertices](const unsigned index) {
		const Vertex vertex = vertices[index];
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
}

void GCodeGeometry::generateTriangles()
{
	if (_areTrianglesReady)
	{
		return;
	}

	Chronograph chronograph(__FUNCTION__, true);

	size_t numSubPathsUsed = std::min<uint32_t>(static_cast<uint32_t>(_extruderSubPaths.size()), _numSubPaths);
	if (numSubPathsUsed == 0)
	{
		std::cout << "### " << __FUNCTION__ << ": numSubPathUsed == 0, return" << std::endl;
		return;
	}
	std::cout << "### updateData() numSubPathUsed:" << numSubPathsUsed << std::endl;

	size_t numPathPoints = 0;
	for (uint32_t j = 0; j < numSubPathsUsed; ++j)
	{
		numPathPoints += std::min<uint32_t>(_numPointsInSubPath, static_cast<uint32_t>(_extruderSubPaths[j].size()));
	}
	if (numPathPoints == 0)
	{
		std::cout << "### " << __FUNCTION__ << ": numPathPoints == 0, return" << std::endl;
		return;
	}
	std::cout << "### updateData() numPathPoints:" << numPathPoints << std::endl;

	static const ushort verticesPerPathPoint = static_cast<ushort>(usedStructVertices.size());
	setStride(static_cast<int32_t>(3 * sizeof(float)));
	_allModelVertices.resize(static_cast<int64_t>(verticesPerPathPoint * numPathPoints * static_cast<uint64_t>(stride())));
	float* coordsPtr = reinterpret_cast<float*>(_allModelVertices.data());

	// One rectangle or 6 rects for cube.
	const ushort numRect = (_isUsingCubeStruct) ? 6u : 1u;
	const ushort numIndexPerRect = 6u;
	_allIndices.resize(static_cast<int64_t>(numPathPoints * numRect * numIndexPerRect * sizeof(uint32_t)));
	uint32_t* indicesPtr = reinterpret_cast<uint32_t*>(_allIndices.data());
	uint32_t totalPrevPathStrokesCount = 0;

	Point prevPoint;
	for (uint32_t subPathIndex = 0; subPathIndex < numSubPathsUsed; ++subPathIndex)
	{
		const Points& subPath = _extruderSubPaths[subPathIndex];

		if (subPath.empty())
		{
			std::cout << " ### WARNING empty path " << std::endl;
			continue;
		}

		// To get a path rectangle with known length we need the first point of subPath defined and start iterating from the second point.
		prevPoint = subPath[0];
		for (uint32_t subPathPointIndex = 1;
			 subPathPointIndex < std::min<uint32_t>(_numPointsInSubPath, uint32_t(subPath.size()));
			 ++subPathPointIndex)
		{
//			const Vector3f boundDiff = maxBound-minBound;
			const Point currPoint = subPath[subPathPointIndex];
			const Vector3f pathStep = currPoint - prevPoint;
			assert(std::numeric_limits<uint32_t>::max() >= static_cast<uint64_t>(totalPrevPathStrokesCount + subPathPointIndex -1) * static_cast<uint64_t>(usedStructVertices.size()));
			const uint32_t firstStructIndexInPathStep = (totalPrevPathStrokesCount + subPathPointIndex -1) * static_cast<uint32_t>(usedStructVertices.size());
			generateSubPathTriangles(prevPoint, pathStep, firstStructIndexInPathStep, coordsPtr, indicesPtr);
			prevPoint = currPoint;
		}
		// TODO: WATCH OUT FOR THIS -1 here!!! (It is necessary.)
		totalPrevPathStrokesCount += subPath.size() -1;
	}

	std::pair<Vertices, Indices> circleGeometry = generateCirclePies({100,100,0}, {15,15,0}, {-15,0,0}, 10);

	const Vertices& circleGeometryVertices = circleGeometry.first;
	const Indices& circleGeometryIndices = circleGeometry.second;

	const uint32_t numCircleGeometryVertices = uint32_t(circleGeometryVertices.size());
	const uint32_t numCircleGeometryindices = uint32_t(circleGeometryIndices .size());

	const uint32_t prevVerticesSize = uint32_t(_allModelVertices.size());
	const uint32_t prevIndicesSize = uint32_t(_allIndices.size());

//	_allModelVertices.resize(prevVerticesSize + numCircleGeometryVertices);
//		  _allIndices.resize(prevIndicesSize  + numCircleGeometryindices);
	_allModelVertices.resize(static_cast<int64_t>(numCircleGeometryVertices * static_cast<uint64_t>(stride())));
		  _allIndices.resize(numCircleGeometryindices * static_cast<uint32_t>(sizeof(uint32_t)));

	coordsPtr = reinterpret_cast<float*>(_allModelVertices.data());
	indicesPtr = reinterpret_cast<uint32_t*>(_allIndices.data());

	for(uint32_t v = 0; v < numCircleGeometryVertices; ++v)
	{
		const Vertex vertex = circleGeometryVertices[v];
		*coordsPtr++ = vertex.x();
		*coordsPtr++ = vertex.y();
		*coordsPtr++ = vertex.z();
	}

	for(uint32_t i = 0; i < numCircleGeometryindices; ++i)
	{
//		*indicesPtr++ = prevVerticesSize + circleGeometryIndices[i];
		*indicesPtr++ = circleGeometryIndices[i];
	}

	setBounds({minBound.x(), minBound.y(), minBound.z()}, {maxBound.x(), maxBound.y(),maxBound.z()});
	_numPathPointsUsed = totalPrevPathStrokesCount + 2;
	_areTrianglesReady = true;
}

void GCodeGeometry::updateData()
{

	clear();
	generateTriangles();

	QByteArray usedVertices(_allModelVertices);
	QByteArray usedIndices(_allIndices);

	// TODO: Refactor - use things initialized previously in generateTriangles().
	const Vertices& usedStructVertices = _isUsingCubeStruct ? _cubeVertices : _squareVertices;
	static const ushort verticesPerPathPoint = static_cast<ushort>(usedStructVertices.size());
//	usedVertices.resize(static_cast<int64_t>(verticesPerPathPoint * _numPathPointsUsed * uint32_t(stride())));
	const ushort numRect = (_isUsingCubeStruct) ? 6u : 1u;
	const ushort numIndexPerRect = 6u;
//	usedIndices.resize(static_cast<int64_t>(_numPathPointsUsed * numRect * numIndexPerRect * sizeof(uint32_t)));

	setVertexData(usedVertices);
	setIndexData(usedIndices);

    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);

	addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
				 0,
				 QQuick3DGeometry::Attribute::U32Type);
}

void GCodeGeometry::setRectProfile(const Real width, const Real height)
{
	const Point start = {-width/Real(2.0), -height/Real(2.0), Real(0.0)};
	_profile = {start, start + Vector3f{0.0, height, 0.0}, start + Vector3f{width, height, 0.0}, start + Vector3f{width, 0.0, 0.0}};
};

QT_END_NAMESPACE
