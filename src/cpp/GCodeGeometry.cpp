#include "GCodeGeometry.h"

#include <QRandomGenerator>
#include <QVector3D>
#include <QQuaternion>

#include <algorithm>
#include <assert.h>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <optional>
#include <string>

#include <Eigen/Geometry>

#include <CommonDefs.h>

#include <Chronograph.h>
#include <Extrusion.h>
#include <GCodeProgramProcessor.h>
#include <Helpers3D.h>


using namespace Eigen;


using ExtrPath = Extrusion::Path;
using ExtrPoint = Extrusion::Point;
using ExtrLayer = Extrusion::LayerBottom;

static Vector3f maxFloatBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
static Vector3f minFloatBound(FLT_MAX, FLT_MAX, FLT_MAX);

static Vector3f maxBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
static Vector3f minBound(FLT_MAX, FLT_MAX, FLT_MAX);


// To have QSG included
QT_BEGIN_NAMESPACE

bool isFinite(const Vector3f &v)
{
	return qIsFinite(v.x()) && qIsFinite(v.y()) && qIsFinite(v.z());
}

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
									   {-0.5, 0.0, 0},
									   {-0.5, 1.0, 0},
									   { 0.5, 1.0, 0},
									   { 0.5, 0.0, 0},
									   //top
									   {-0.5, 0.0, 1},
									   {-0.5, 1.0, 1},
									   { 0.5, 1.0, 1},
									   { 0.5, 0.0, 1}};

static void dumpVector3f(const Vector3f& v, const std::string& name = "")
{
	std::cout << " ### vector " << name << ": " << v.x() << "," << v.y() << "," << v.z() << std::endl;
}

#define ENABLE_GENERATION_CODE

Vector3f GCodeGeometry::calculateSubpathCuboid(const ExtrPoint& pathStart, const ExtrPoint& pathEnd, const float pathBaseLevelZ)
{// x is for width, y is for height, z is for length of the cuboid
	const float height = pathStart.z() - pathBaseLevelZ;
	const Vector4f pathStep = pathEnd - pathStart;
	const float length = pathStep.head<3>().norm();
//	if (length == 0.0)
//	{
//		assert(length > 0);
//	}

	if (pathStep.w() <= 0)
		return {0,0,length};

	const float width = (_extrData.filamentCrossArea * pathStep.w()) / (height * length);

	return {qIsFinite(width) ? width : 0, height, length};
};

static std::pair<Vertices, Indices> generateCylinderPieSection(const ExtrPoint& center,
															   const Vector3f& radiusStart,
															   const Vector3f& axis,
															   const float angle,
															   const float thickness,
															   uint16_t numAngleSteps)
{
	Vertices vert{{center.x(), center.y(), center.z()}};
	Indices ind;
//	if (angle < 0)

	const float angleStep = angle / float(numAngleSteps);

//	const Vector3f& usedAxis;

	for(float p : range<float>(numAngleSteps+1))
	{
		assert(isFinite(vert[0] + AngleAxisf(p * angleStep, axis) * radiusStart));
		vert.push_back(vert[0] + AngleAxisf(p * angleStep, axis) * radiusStart);

		if (unsigned(p) == numAngleSteps)
		{// We only need the last vertex from the final iteration.
			break;
		}

		ind.push_back(0);
		ind.push_back(uint32_t(p)+1);
		ind.push_back(uint32_t(p)+2);
	}

	// Copy the circle  pie shifted by height in rotation axis direction.
	const Vector3f heightShift = thickness * axis;
	const uint32_t upperCircleCenterIndex = uint32_t(vert.size());
	const uint32_t firstIndicesSize = uint32_t(ind.size());
	vert.resize(2*upperCircleCenterIndex);
	ind.resize(2*firstIndicesSize);
	for (uint32_t i = 0, j = upperCircleCenterIndex; i < upperCircleCenterIndex; ++i, ++j)
	{
		assert(isFinite(vert[i]));
		vert[j] = vert[i] + heightShift;
	}
	for (uint32_t i = 0, j = firstIndicesSize; i < firstIndicesSize; ++i, ++j)
	{
		ind[j] = ind[i] + upperCircleCenterIndex;
	}

	// Setup pie cylinder side part indices, 1 quad == 2 triangles per each point
	const uint32_t secondIndSize = uint32_t(ind.size());
	ind.resize(ind.size() + (6 * numAngleSteps));
	uint32_t* triangleIndexPtr = &ind[secondIndSize];
	for (uint32_t i = 1, j = firstIndicesSize +1; i < 3*numAngleSteps; i+=3, j+=3)
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

void GCodeGeometry::loadExtruderData()
{
	Chronograph chrono(__FUNCTION__);
	if (_inputFile.isEmpty())
		return;

	GCodeProgramProcessor gCodeParser;
	std::map<uint32_t, Extrusion> extrusions = gCodeParser.generateExtrusionsFromGCode(_inputFile.toStdString());

	assert(extrusions.size() > 0);

	std::set<uint32_t> extruderToolIndices;
	std::vector<uint32_t> extruderToolIndicesVec;
	for(const auto& mapping: extrusions)
	{// Get all extruder indices ordered, to generate table with the lowest one (0 - most probably).
		extruderToolIndices.insert(mapping.first);
	}

	// Treat extrusion data with the lowest index as 'this' geometry.
	auto indicesIterator = extruderToolIndices.begin();
	_extrData = extrusions[*indicesIterator++];
	_subGeometries.push_back(this);

	initialize();

	while(indicesIterator != extruderToolIndices.end())
	{// Initialize all following indices as subgeometries.
		GCodeGeometry* subGeometry = new GCodeGeometry(extrusions[*indicesIterator++]);
		_subGeometries.push_back(subGeometry);
	}

	emit subGeometriesChanged();
}

void GCodeGeometry::initialize()
{
	setStride(static_cast<int32_t>(3 * sizeof(float)));

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

GCodeGeometry::GCodeGeometry() :
	_subGeometries({}),
	_numPathStepsUsed(0),
	_modelIndices({}),
	_modelVertices({}),
	_inputFile("")
{
	range_test<unsigned>();
	range_test<float>();

	initialize();
}

GCodeGeometry::GCodeGeometry(const Extrusion& extruderData) : GCodeGeometry()
{
	_extrData = extruderData;
	initialize();
}

GCodeGeometry::~GCodeGeometry()
{// Remember to delete all subgeometries, but this one (which is at index 0).
	for (uint32_t i=1; i<_subGeometries.size(); ++i)
	{
		_subGeometries[i]->deleteLater();
	}
}

void GCodeGeometry::setInputFile(const QString& url)
{
	std::cout << "### " << __FUNCTION__ << ": " << url.toStdString() << std::endl;
	if (url == _inputFile)
		return;

	_inputFile = url;
	_wasGenerated = false;

	_modelIndices.clear();
	_modelVertices.clear();

	loadExtruderData();

	std::cout << "### " << __FUNCTION__ << "_extruderSubPaths.size(): " << _extrData.paths.size() << std::endl;
	std::cout << "### " << __FUNCTION__ << "_numSubPaths: " << _extrData.numPaths << std::endl;

	updateData();
	update();

	emit modelLoaded();
}

QString GCodeGeometry::getInputFile() const
{
	return _inputFile;
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
	if (_extrData.numPaths == num)
		return;

	_extrData.numPaths = num;

	emit numSubPathsChanged();
}

unsigned GCodeGeometry::getNumSubPaths() const
{
	return _extrData.numPaths;
}

void GCodeGeometry::setNumPointsInSubPath(const unsigned num)
{
	if (_extrData.numPathPointsMax == num)
		return;

	_extrData.numPathPointsMax = num;

	emit numPointsInSubPathChanged();
}

uint32_t GCodeGeometry::getNumPointsInSubPath() const
{
	return _extrData.numPathPointsMax;
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

QList<GCodeGeometry*> GCodeGeometry::getSubGeometries()
{
	return _subGeometries;
}

size_t GCodeGeometry::calcVerifyModelNumbers()
{
	const size_t numSubPathsUsed = std::min<uint32_t>(static_cast<uint32_t>(_extrData.paths.size()), _extrData.numPaths);
	if (numSubPathsUsed == 0)
	{
		std::cout << "### " << __FUNCTION__ << ": numSubPathUsed == 0, return" << std::endl;
		return 0;
	}
	std::cout << "### calcVerifyModelNumbers() numSubPathUsed:" << numSubPathsUsed << std::endl;

	size_t numPathPoints = 0;
	for (uint32_t subPathIndex = 0; subPathIndex < numSubPathsUsed; ++subPathIndex)
	{
		numPathPoints += std::min<uint32_t>(_extrData.numPathPointsMax, static_cast<uint32_t>(_extrData.paths[subPathIndex].size()));
	}
	if (numPathPoints == 0)
	{
		std::cout << "### " << __FUNCTION__ << ": numPathPoints == 0, return" << std::endl;
		return 0;
	}
	std::cout << "### calcVerifyModelNumbers() numPathPoints:" << numPathPoints << std::endl;
	return numSubPathsUsed;
}

#ifdef ENABLE_GENERATION_CODE


void GCodeGeometry::reset()
{
	_numPathStepsUsed = 0;
	_modelVertices.resize(0);
	_modelIndices.resize(0);
	_numTotalPathStepVertices.resize(0);
	_numTotalPathStepIndices.resize(0);
	// Push back 0's to always be able to quickly compare the new size
	// with the previous (without additional checks for container size);
	_numTotalPathStepVertices.push_back(0);
	_numTotalPathStepIndices.push_back(0);
}

bool GCodeGeometry::verifyEnoughPoints(const Extrusion::Path& subPath)
{
	if (subPath.size() > 1)
		return true;

	std::cout << " ### WARNING extruder subPath.size(): " << subPath.size() << std::endl;
	return false;
}

float GCodeGeometry::getLayerBottom(const uint32_t layerIndex)
{// TODO: Ideally, this function should look down below from the current nozzle level and detect the previous layer.
	return _extrData.layerBottoms[layerIndex].second;
}

void GCodeGeometry::logSubPath(const Extrusion::Path& path)
{
	std::cout << " ### " << __FUNCTION__ << " subPath:" << "" << "," << "" << std::endl;
	std::for_each(path.begin(), path.end(), [](const ExtrPoint& p) {
		std::cout << " ### " << "ExtrPoint" << " :" << p.x() << "," << p.y() << "," << p.z() << "," << p.w() << std::endl;
	});
}

void GCodeGeometry::generate()
{
	Chronograph chronograph(__FUNCTION__, true);

	if (_wasGenerated)
	{
		return;
	}

	reset();

	if (calcVerifyModelNumbers() < 1)
		return;

	//Remember start point to know the direction and level, from which extruder head arrives in next subpath.
	ExtrPoint lastStartPoint{0,0,0,0};
	const std::vector<ExtrLayer>& layerBottoms = _extrData.layerBottoms;
	const std::vector<ExtrPath>& extruderSubPaths = _extrData.paths;

	for (uint32_t layerIndex=0; layerIndex<layerBottoms.size(); ++layerIndex)
	{// For every model layer.
		const float layerBottom = getLayerBottom(layerIndex);

		uint32_t subPathIndex = layerBottoms[layerIndex].first;
		const uint32_t nextLayerSubPathIndex = (layerIndex < layerBottoms.size() -1) ?
												   layerBottoms[layerIndex +1].first :
												   uint32_t(extruderSubPaths.size());

		for (; subPathIndex<nextLayerSubPathIndex; ++subPathIndex)
		{// For every layer subPath - which is a set of consecutive extrusion points - generate a corresponding contiguous geometry.
			const ExtrPath& subPath = extruderSubPaths[subPathIndex];

			if (!verifyEnoughPoints(subPath))
				continue;

			// To get a path step with known length, remember the first point of new subPath
			lastStartPoint = subPath[0];

			// Profile recalculation for the beginning of the subpath.
			Vector3f subPathCuboid = calculateSubpathCuboid(subPath[0], subPath[1], layerBottom);

			// Prepend the first subPath point with half of a cylinder.
			const Vector3f dirAtBeginning = (subPath[1] - subPath[0]).head<3>().normalized();
			static const Vector3f upVector{0,0,1};
			Vector3f rightToDir = dirAtBeginning.cross(upVector);
			Vector3f turnAxis = rightToDir.cross(dirAtBeginning);
			// TODO: The turn radius is calculated based on a certain simplification.
			assert(qIsFinite(subPathCuboid.x()));
			assert(isFinite(rightToDir));
			generateSubPathTurn(subPath[0],
					0.5f * subPathCuboid.x() * rightToDir,
					turnAxis, -float(M_PI), subPathCuboid.y(), _modelVertices, _modelIndices);

			// Start iterating from the second point, end before the last one.
			const uint32_t numSubPathPoints = std::min<uint32_t>(_extrData.numPathPointsMax, uint32_t(subPath.size()));
			for (uint32_t subPathPointIndex = 1; subPathPointIndex < numSubPathPoints -1; ++subPathPointIndex)
			{
				const ExtrPoint& currPoint = subPath[subPathPointIndex];
				const ExtrPoint& nextPoint = subPath[subPathPointIndex +1];
				const Vector4f pathStep = currPoint - lastStartPoint;

				subPathCuboid = calculateSubpathCuboid(lastStartPoint, currPoint, layerBottom);
				generateSubPathStep(lastStartPoint.head<3>(), pathStep.head<3>(), subPathCuboid, _modelVertices, _modelIndices);

				// Insert a cylinder section (pie) between two path steps, forget about w==filament extrusion length.
				const Vector3f prevDirection = pathStep.head<3>().normalized();
				const Vector3f nextDirection = (nextPoint - currPoint).head<3>().normalized();

				const Vector3f turnAxis = prevDirection.cross(nextDirection).normalized();

                const float turnAngle = std::acos(static_cast<float>(prevDirection.dot(nextDirection)));
				if (!approximatelyEqual(turnAngle, 0, FLT_EPSILON))
				{
					//TODO: Watch out - HACKING a bit.
					const Vector3f bottomShift = subPathCuboid.y() * turnAxis * (turnAxis.dot(upVector) > 0 ? 0 : 1);
					assert(qIsFinite(subPathCuboid.x()));
					assert(isFinite(prevDirection.cross(turnAxis)));
					if (qIsFinite(turnAngle))
					{
						generateSubPathTurn(currPoint - ExtrPoint{bottomShift.x(), bottomShift.y(), bottomShift.z(), 0},
										0.5f * subPathCuboid.x() * prevDirection.cross(turnAxis),
										turnAxis, turnAngle, subPathCuboid.y(), _modelVertices, _modelIndices);
					}
				}

				lastStartPoint = currPoint;
			}

			// The last point should be generated differently.
			const ExtrPoint& currPoint = subPath[numSubPathPoints -1];
			const Vector4f pathStep = currPoint - lastStartPoint;

			// Profile recalculation for the end of the subpath.
			subPathCuboid = calculateSubpathCuboid(lastStartPoint, currPoint, layerBottom);
			generateSubPathStep(lastStartPoint.head<3>(), pathStep.head<3>(), subPathCuboid, _modelVertices, _modelIndices);

			// Append a semi-circle cylinder half to the end of the subPath.
			const Vector3f dirAtEnd = (currPoint - lastStartPoint).head<3>().normalized();
			rightToDir = dirAtEnd.cross(upVector);
			turnAxis = rightToDir.cross(dirAtEnd);

			assert(qIsFinite(subPathCuboid.x()));
			assert(isFinite(rightToDir));
			generateSubPathTurn(currPoint,
								0.5f * subPathCuboid.x() * rightToDir,
								turnAxis, float(M_PI), subPathCuboid.y(), _modelVertices, _modelIndices);

			lastStartPoint = currPoint;
		}
	}

	setBounds({minBound.x(), minBound.y(), minBound.z()}, {maxBound.x(), maxBound.y(),maxBound.z()});

	_wasGenerated = true;
}

void GCodeGeometry::updateData()
{
	Chronograph chrono(__FUNCTION__, true);
	clear();
	generate();

	if (!_wasGenerated)
	{
		setVertexData({});
		setIndexData({});
		return;
	}

	QByteArray usedVertices(_modelVertices);
	QByteArray usedIndices(_modelIndices);

	const uint32_t numTotalPathStepVerticesUsed = _numTotalPathStepVertices[_numPathStepsUsed];
	const uint32_t numTotalPathStepIndicesUsed = _numTotalPathStepIndices[_numPathStepsUsed];

	usedVertices.resize(numTotalPathStepVerticesUsed * static_cast<uint32_t>(stride()));
	usedIndices.resize(numTotalPathStepIndicesUsed * sizeof(uint32_t));

//	for (uint32_t pos = 0; pos < _modelVertices.size(); pos+=static_cast<uint32_t>(stride()))
//	{
//		const Vertex& v = *reinterpret_cast<const Vertex*>(&_modelVertices[pos]);
//		std::cout << " ### " << __FUNCTION__ << " v:" << v.x() << "," << v.y() << "," << v.z() << std::endl;
//	}

//	for (uint32_t pos = 0; pos < _modelIndices.size(); pos+=sizeof(uint32_t))
//	{
//		const uint32_t& i = *reinterpret_cast<const uint32_t*>(&_modelIndices[pos]);
//		std::cout << " ### " << __FUNCTION__ << " i:" << i << std::endl;
//	}

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


void GCodeGeometry::generateSubPathStep(const Vector3f& prevPoint,
										const Vector3f& pathStep,
										const Vector3f& cuboid,
										QByteArray& modelVertices,
										QByteArray& modelIndices)
{
	if (cuboid.z() < FLT_MIN)
		return;

	// VERTICES
	static const Indices cubeMeshVertexIndices = {0,1,2,3,4,5,6,7};

	// QUADS
	static const std::vector<Indices> cubeMeshQuadIndices = {{0,1,2,0,2,3}, // bottom
															 //{0,3,7,7,4,0}, // back
															 //{1,2,6,6,5,1}, // front
															 {4,5,6,6,7,4}, // top
															 {0,1,5,5,4,0}, // left
															 {3,2,6,6,7,3}}; // right

	static const uint32_t numAddedVertices = uint32_t(cubeMeshVertexIndices.size());
	static const uint32_t numAddedIndices = std::accumulate(cubeMeshQuadIndices.begin(), cubeMeshQuadIndices.end(), 0u,
															[](uint32_t acc, const Indices& indices){ return indices.size() + acc; });

	const uint32_t prevVerticesSize = uint32_t(modelVertices.size());
	const uint32_t prevIndicesSize = uint32_t(modelIndices.size());

	const uint32_t newVertexIndex = prevVerticesSize/static_cast<uint32_t>(stride());

	modelVertices.resize(prevVerticesSize + (numAddedVertices * static_cast<int64_t>(stride())));
	modelIndices.resize(prevIndicesSize + (numAddedIndices * sizeof(uint32_t)));

	float* coordsPtr = reinterpret_cast<float*>(&(modelVertices[prevVerticesSize]));
	uint32_t* indicesPtr = reinterpret_cast<uint32_t*>(&(modelIndices[prevIndicesSize]));

	// Rotate the cuboid from direction upwards to direction along the path.
	const Eigen::Quaternionf rotation = Quaternionf::FromTwoVectors(Vector3f{0,1,0}, pathStep);

	// Watch out! Our cuboid is rotated to lay parallel to the z=0 plane, so it y becomes z, and vice-versa.
	const Matrix3f scale{{cuboid.x(), 0,          0          },
						 {0,          cuboid.z(), 0          },
						 {0,          0,          cuboid.y()}};

	Vertices vertices = _cubeVertices;

	std::for_each(vertices.begin(), vertices.end(), [&scale, &rotation, &prevPoint](Vector3f& v){
		v = rotation * (scale * v) + prevPoint;
	});

	const auto setTriangleVertexCoords = [&coordsPtr, &vertices](const unsigned index) {
		const Vertex vertex = vertices[index];
		assert(isFinite(vertex));
		*coordsPtr++ = vertex.x();
		*coordsPtr++ = vertex.y();
		*coordsPtr++ = vertex.z();
		updateBounds(coordsPtr - 3);
	};

	const auto setTriangleVertexIndex = [&indicesPtr, newVertexIndex](const unsigned appendedStructIndex) {
		*indicesPtr++ = newVertexIndex + appendedStructIndex;
	};

	const auto setQuadVertexCoords = [setTriangleVertexCoords](const std::vector<uint32_t>& indices) {
		std::for_each(indices.begin(), indices.end(), setTriangleVertexCoords);
	};

	const auto setQuadTriangleIndices = [setTriangleVertexIndex](const std::vector<uint32_t>& indices) {
		std::for_each(indices.begin(), indices.end(), setTriangleVertexIndex);
	};

	setQuadVertexCoords(cubeMeshVertexIndices);
	std::for_each(cubeMeshQuadIndices.begin(), cubeMeshQuadIndices.end(), setQuadTriangleIndices);

	// Remember how many vertices and indices are added in this structure.
	_numTotalPathStepVertices.push_back(_numTotalPathStepVertices.back() + numAddedVertices);
	_numTotalPathStepIndices.push_back(_numTotalPathStepIndices.back() + numAddedIndices);
	++_numPathStepsUsed;
}

void GCodeGeometry::generateSubPathTurn(const ExtrPoint& center,
										const Vector3f& radiusStart,
										const Vector3f& axis,
										const float angle,
										const float height,
										QByteArray& modelVertices,
										QByteArray& modelIndices)
{
	//	Chronograph chrono(__FUNCTION__);
	static const uint32_t numStepsPerFullCircle = 20;
	static const float anglePerStep = 2.0f*float(M_PI)/numStepsPerFullCircle;
	uint16_t numAngleSteps = static_cast<uint16_t>(std::abs(angle)/anglePerStep);
	if (numAngleSteps < 1)
	{
		numAngleSteps = 1;
	}

	const std::pair<Vertices, Indices> cylinderPieGeometry = generateCylinderPieSection(center, radiusStart, axis, angle, height, numAngleSteps);

	//	std::for_each(cylinderPieGeometry.first.begin(), cylinderPieGeometry.first.end(), [](const Vertex& v) {
	//		std::cout << " ### " << __FUNCTION__ << " vertex:" << v.x() << "," << v.y() << "," << v.z() << std::endl;
	//	});

	//	std::for_each(cylinderPieGeometry.second.begin(), cylinderPieGeometry.second.end(), [](const Index& v) {
	//		std::cout << " ### " << __FUNCTION__ << " index:" << v << std::endl;
	//	});

	const Vertices& cylinderPieVertices = cylinderPieGeometry.first;
	const Indices& cylinderPieIndices = cylinderPieGeometry.second;

	const uint32_t prevVerticesSize = uint32_t(modelVertices.size());
	const uint32_t prevIndicesSize = uint32_t(modelIndices.size());

	const uint32_t numCircleGeometryVertices = uint32_t(cylinderPieVertices.size());
	const uint32_t numCircleGeometryindices = uint32_t(cylinderPieIndices.size());

	assert(std::numeric_limits<uint32_t>::max() >= prevVerticesSize + (numCircleGeometryVertices * static_cast<uint32_t>(stride())));
	assert(std::numeric_limits<uint32_t>::max() >= prevIndicesSize + (numCircleGeometryindices * sizeof(uint32_t)));

	modelVertices.resize(prevVerticesSize + (numCircleGeometryVertices * static_cast<uint32_t>(stride())));
	modelIndices.resize(prevIndicesSize  + (numCircleGeometryindices  * sizeof(uint32_t)));

	float* coordsPtr = reinterpret_cast<float*>(&(modelVertices.data()[prevVerticesSize]));
	uint32_t* indicesPtr = reinterpret_cast<uint32_t*>(&(modelIndices.data()[prevIndicesSize]));
	const uint32_t newMeshVertexIndex = prevVerticesSize/static_cast<uint32_t>(stride());

	//	std::cout << " ### " << __FUNCTION__ << " newMeshVertexIndex:" << newMeshVertexIndex << std::endl;

	for(uint32_t v=0; v<numCircleGeometryVertices; ++v)
	{
		const Vertex vertex = cylinderPieVertices[v];
		assert(isFinite(vertex));
		*coordsPtr++ = vertex.x();
		*coordsPtr++ = vertex.y();
		*coordsPtr++ = vertex.z();
	}

	for(uint32_t i=0; i<numCircleGeometryindices; ++i)
	{
		*indicesPtr++ = newMeshVertexIndex + cylinderPieIndices[i];
	}

	// Remember how many vertices and indices are added in this structure.
	_numTotalPathStepVertices.push_back(_numTotalPathStepVertices.back() + numCircleGeometryVertices);
	_numTotalPathStepIndices.push_back(_numTotalPathStepIndices.back() + numCircleGeometryindices);
	++_numPathStepsUsed;
}

#endif

QT_END_NAMESPACE
