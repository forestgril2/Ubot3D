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

static std::pair<Vertices, Indices> generateCirclePies(const Point& center,
													   const Vector3f& radiusStart,
													   const Vector3f& radiusEnd,
													   const float height,
													   unsigned short numCirclePoints = 20)
{
	Vertices vert{center};
	Indices ind;
	const float startAngle = atan2(radiusStart.y(), radiusStart.x());
	float endAngle = atan2(radiusEnd.y(), radiusEnd.x());
	if (endAngle < startAngle)
	{
		endAngle += 2.0f*float(M_PI);
	}
	const float angleStep = (endAngle - startAngle) / float(numCirclePoints);
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

GCodeGeometry::GCodeGeometry() :
	_numPathStepsUsed(std::numeric_limits<uint32_t>::max())
{
	range_test<unsigned>();
	range_test<float>();

	setRectProfile(0.4f, 0.28f);

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
	connect(this, &GCodeGeometry::numPathPointsStepsChanged, this, updateDataAndUpdate);
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

	_maxNumPointsInSubPath = maxPointsInSubPath;
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
	if (_maxNumPointsInSubPath == num)
		return;

	_maxNumPointsInSubPath = num;

	emit numPointsInSubPathChanged();
}

unsigned GCodeGeometry::getNumPointsInSubPath() const
{
	return _maxNumPointsInSubPath;
}

void GCodeGeometry::setNumPathStepsUsed(const uint32_t num)
{
	if (_numPathStepsUsed == num)
		return;

	_numPathStepsUsed = num;

	emit numPathPointsStepsChanged();

}

uint32_t GCodeGeometry::getNumPathPointsUsed() const
{
	return _numPathStepsUsed;
}

void GCodeGeometry::generateSubPathData(const Point& prevPoint,
										const Vector3f& pathStep,
										const uint32_t meshIndexInPathStep,
										QByteArray& modelVertices,
										QByteArray& modelIndices)
{
	const float length = pathStep.norm();
	assert(length > FLT_MIN);

	// VERTICES
	static const Indices cubeMeshVertexIndices = {0,1,2,3,4,5,6,7};

	// QUADS
	static const std::vector<Indices> cubeMeshQuadIndices = {{0,1,2,0,2,3}, // bottom
															 {4,5,6,6,7,4}, // top
															 {0,1,5,5,4,0}, // left
															 {3,2,6,6,7,3}, // right
															 {0,3,7,7,4,0}, // back
															 {1,2,6,6,5,1}};// front*/

	static const uint32_t numAddedVertices = uint32_t(cubeMeshVertexIndices.size());
	static const uint32_t numAddedIndices = std::accumulate(cubeMeshQuadIndices.begin(), cubeMeshQuadIndices.end(), 0u,
															[](uint32_t acc, const Indices& indices){ return indices.size() + acc; });



	const uint32_t prevVerticesSize = uint32_t(modelVertices.size());
	const uint32_t prevIndicesSize = uint32_t(modelIndices.size());

	modelVertices.resize(prevVerticesSize + numAddedVertices*static_cast<int64_t>(stride()));
	modelIndices.resize(prevIndicesSize + numAddedIndices*sizeof(uint32_t));

	float* coordsPtr = reinterpret_cast<float*>(&(modelVertices[prevVerticesSize]));
	uint32_t* indicesPtr = reinterpret_cast<uint32_t*>(&(modelIndices[prevIndicesSize]));

	static const Vector3f profileDiag = _profile[2] - _profile[0];
	const Matrix3f scale{{profileDiag.x(), 0,      0              },
						 {0,               length, 0              },
						 {0,               0,      profileDiag.y()}};
	const Eigen::Quaternionf rotation = Quaternionf::FromTwoVectors(Vector3f{0,1,0}, pathStep);

	Vertices vertices = _cubeVertices;

	std::for_each(vertices.begin(), vertices.end(), [&scale, &rotation, &prevPoint](Vector3f& v){
		v = rotation*(scale*v) + prevPoint;
	});

	auto setTriangleVertexCoords = [&coordsPtr, &vertices](const unsigned index) {
		const Vertex vertex = vertices[index];
		*coordsPtr++ = vertex.x();
		*coordsPtr++ = vertex.y();
		*coordsPtr++ = vertex.z();
		updateBounds(coordsPtr - 3);
	};

	auto setTriangleVertexIndex = [&indicesPtr, meshIndexInPathStep](const unsigned structIndex) {
		*indicesPtr++ = meshIndexInPathStep + structIndex;
	};

	auto setQuadVertexCoords = [setTriangleVertexCoords](const std::vector<uint32_t>& indices) {
		std::for_each(indices.begin(), indices.end(), setTriangleVertexCoords);
	};

	auto setQuadTriangleIndices = [setTriangleVertexIndex](const std::vector<uint32_t>& indices) {
		std::for_each(indices.begin(), indices.end(), setTriangleVertexIndex);
	};

	setQuadVertexCoords(cubeMeshVertexIndices);
	std::for_each(cubeMeshQuadIndices.begin(), cubeMeshQuadIndices.end(), setQuadTriangleIndices);

	const short unsigned numOfPointsInCylinderCircleBase = 20;
	const float height = profileDiag.y();
	const float radius = 0.5f * profileDiag.x();
}

void GCodeGeometry::setupPieData(float*& coordsPtr, uint32_t*& indicesPtr)
{
	std::pair<Vertices, Indices> circleGeometry = generateCirclePies({100,100,0}, {15,0,0}, {0,-15,0}, 10);

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
}

size_t GCodeGeometry::calcVerifyModelNumbers()
{
	const size_t numSubPathsUsed = 1;//std::min<uint32_t>(static_cast<uint32_t>(_extruderSubPaths.size()), _numSubPaths);
	if (numSubPathsUsed == 0)
	{
		std::cout << "### " << __FUNCTION__ << ": numSubPathUsed == 0, return" << std::endl;
		exit(-1);
	}
	std::cout << "### updateData() numSubPathUsed:" << numSubPathsUsed << std::endl;

	size_t numPathPoints = 0;
	for (uint32_t subPathIndex = 0; subPathIndex < numSubPathsUsed; ++subPathIndex)
	{
		numPathPoints += std::min<uint32_t>(_maxNumPointsInSubPath, static_cast<uint32_t>(_extruderSubPaths[subPathIndex].size()));
	}
	if (numPathPoints == 0)
	{
		std::cout << "### " << __FUNCTION__ << ": numPathPoints == 0, return" << std::endl;
		exit(-1);
	}
	std::cout << "### updateData() numPathPoints:" << numPathPoints << std::endl;
	return numSubPathsUsed;
}

void GCodeGeometry::generateTriangles()
{
	if (_areTrianglesReady)
	{
		return;
	}

	Chronograph chronograph(__FUNCTION__, true);

	const size_t numSubPathsUsed = calcVerifyModelNumbers();

	// TODO: This needs to be changed to accomodate for a custom structure..
	setStride(static_cast<int32_t>(3 * sizeof(float)));
	_allModelVertices.resize(0);
	_allIndices.resize(0);

	// One rectangle or 6 rects for cube.
	// TODO: This needs to be changed to accomodate for a custom structure..
	uint32_t totalPathStepsCount = 0;

	Point prevPoint;
	for (const Points& subPath : _extruderSubPaths)
	{// For every subPath - a contiguous extrusion path points.
		if (subPath.empty())
		{
			std::cout << " ### WARNING empty path " << std::endl;
			continue;
		}

		const uint32_t numSubPathPoints = std::min<uint32_t>(_maxNumPointsInSubPath, uint32_t(subPath.size())) -1;
		// TODO: This needs to be changed to accomodate for a custom structure..
		assert(std::numeric_limits<uint32_t>::max() >= static_cast<uint64_t>(totalPathStepsCount + numSubPathPoints -1) * static_cast<uint64_t>(_cubeVertices.size()));

		// To get a path step with known length we need the first point of subPath defined and start iterating from the second point.
		prevPoint = subPath[0];
//		++totalPathStepsCount;
		for (uint32_t subPathPointIndex = 1; subPathPointIndex <= numSubPathPoints; ++subPathPointIndex)
		{
//			if (totalPathStepsCount >= _numPathStepsUsed)
//				break;

			const Point& currPoint = subPath[subPathPointIndex];
			const Vector3f pathStep = currPoint - prevPoint;
			// TODO: This needs to be changed to accomodate for a custom structure..
			const uint32_t meshVertexIndex = (totalPathStepsCount + subPathPointIndex -1) * static_cast<uint32_t>(_cubeVertices.size());

			generateSubPathData(prevPoint, pathStep, meshVertexIndex, _allModelVertices, _allIndices);

			prevPoint = currPoint;

//			++totalPathStepsCount;
		}
		totalPathStepsCount += numSubPathPoints;

//		if (totalPathStepsCount >= _numPathStepsUsed)
//			break;
	}

	_numPathStepsUsed = totalPathStepsCount;

//	setupPieData(coordsPtr, indicesPtr);

	setBounds({minBound.x(), minBound.y(), minBound.z()}, {maxBound.x(), maxBound.y(),maxBound.z()});
	_areTrianglesReady = true;
}

void GCodeGeometry::updateData()
{
	clear();
	generateTriangles();

	QByteArray usedVertices(_allModelVertices);
	QByteArray usedIndices(_allIndices);

	static const ushort numRectsPerPathStep = 6u;
	static const ushort numIndicesPerRect = 6u;
	static const ushort verticesPerPathStep = static_cast<ushort>(_cubeVertices.size());

	usedVertices.resize(_numPathStepsUsed * verticesPerPathStep *  uint32_t(stride()));
	usedIndices.resize(_numPathStepsUsed * numRectsPerPathStep * numIndicesPerRect * sizeof(uint32_t));

	// TODO?: Refactor - use things initialized previously in generateTriangles()?.

//	std::cout << "### verticesPerPathStep :" << verticesPerPathStep << std::endl;
//	std::cout << "### _numPathStepsUsed :" << _numPathStepsUsed << std::endl;
//	std::cout << "### stride() :" << stride() << std::endl;
//	std::cout << "### multiplied :" << _numPathStepsUsed * verticesPerPathStep *  uint32_t(stride()) << std::endl;
//	std::cout << "### usedVertices.size() :" << usedVertices.size() << std::endl;

//	assert(usedVertices.size() == static_cast<int64_t>(_numPathStepsUsed * verticesPerPathStep *  uint32_t(stride())));

//	assert(usedIndices.size() == static_cast<int64_t>(_numPathStepsUsed * numRectsPerPathStep * numIndicesPerRect * sizeof(uint32_t)));

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
