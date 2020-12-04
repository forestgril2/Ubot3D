#include "GCodeGeometry.h"

#include <QRandomGenerator>
#include <QVector3D>
#include <QQuaternion>

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

	connect(this, &GCodeGeometry::numSubpathsChanged, this, [this](){
		updateData();
		update();
	});
	connect(this, &GCodeGeometry::numPointsInSubpathChanged, this,  [this](){
		updateData();
		update();
	});
}

void GCodeGeometry::createExtruderPaths(const gpr::gcode_program& gcodeProgram)
{
	std::vector<Vector3f> subPath;
	_extruderPaths.push_back(std::vector<Vector3f>());

	unsigned maxPointsInSubPath = 0;

	bool isExtruderOn = false;
	unsigned blockCount = 0;
	const unsigned blockCountLimit = 500000;
	for (auto it = gcodeProgram.begin(); it != gcodeProgram.end() && blockCount < blockCountLimit; ++it, ++blockCount)
	{
		const auto& block = *it;
		const std::string blockString = block.to_string();
//		std::cout << blockString << std::endl;

		auto setExtrusionOff = [this, &maxPointsInSubPath, &blockString, &blockCount, &subPath, &isExtruderOn]()
		{// If we are setting extrusion off, swap created path with the empty one in path vector.
			if (!isExtruderOn)
				return;
//			std::cout << " ####### setExtrusionOff : " << std::endl;
			isExtruderOn = false;

			if (subPath.empty())
				return;

//			std::cout << " #### adding subpath with: " << subPath.size() << std::endl;
			maxPointsInSubPath = std::max<unsigned>(maxPointsInSubPath, subPath.size());
			std::swap(_extruderPaths.back(), subPath);
			_extruderPaths.push_back(std::vector<Vector3f>());
		};
		auto setExtrusionOn = [&isExtruderOn]()
		{// If we are setting extrusion on, add new (empty) path to path vector.
			if (isExtruderOn)
				return;
//			std::cout << " ####### setExtrusionOn : " << std::endl;
			isExtruderOn = true;
//			std::cout << " ####### adding empty subpath with: " << std::endl;
		};

		Vector3i coordsSet(0,0,0);
		Vector3f absoluteCoords(0,0,0);

		Vector3f currentCoords(0,0,0);
		Vector3f relativeCoords(0,0,0);

		Vector3f* newCoordsPtr = &absoluteCoords;

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
		std::cout << " #### adding subpath no. " << _extruderPaths.size()+1 << ", maxPointsInSubPath: " << maxPointsInSubPath << std::endl;
		std::swap(_extruderPaths.back(), subPath);
	}

	setNumPointsInSubpath(maxPointsInSubPath);
	setNumSubpaths(_extruderPaths.size());
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

void GCodeGeometry::setNumSubpaths(const unsigned num)
{
	if (_numSubpaths == num)
		return;

	_numSubpaths = num;

	emit numSubpathsChanged();
}

unsigned GCodeGeometry::getNumSubpaths() const
{
	return _numSubpaths;
}

void GCodeGeometry::setNumPointsInSubpath(const unsigned num)
{
	if (_numPointsInSubpath == num)
		return;

	_numPointsInSubpath = num;

	emit numPointsInSubpathChanged();
}

unsigned GCodeGeometry::getNumPointsInSubpath() const
{
	return _numPointsInSubpath;
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

	size_t numPathPoints = 0;
	size_t numSubpathUsed = std::min<uint32_t>(_extruderPaths.size(), _numSubpaths);
	for (uint32_t j = 0; j < numSubpathUsed; ++j)
	{
		numPathPoints += std::min<uint32_t>(_numPointsInSubpath, _extruderPaths[j].size());
	}

	std::cout << "### updateData() _numSubpaths:" << _numSubpaths << std::endl;
	std::cout << "### updateData() numPathPoints:" << numPathPoints << std::endl;

	QByteArray vertices;
	vertices.resize(4 * numPathPoints * stride);
	float* p = reinterpret_cast<float*>(vertices.data());

	QByteArray indices;
	indices.resize(6 * numPathPoints * sizeof(uint32_t));
	uint32_t* pi = reinterpret_cast<uint32_t*>(indices.data());

	const std::vector<Vector3f> squareVertices = {{-0.5, 0.0, 0.0},
												  {-0.5, 1.0, 0.0},
												  { 0.5, 1.0, 0.0},
												  { 0.5, 0.0, 0.0}};

	for (uint32_t j = 0; j < numSubpathUsed; ++j)
	{
		const std::vector<Vector3f>& path = _extruderPaths[j];

		//TODO: fails here.
//		assert(!path.empty());

		if (path.empty())
		{
//			std::cout << " ### empty path " << std::endl;
			continue;
		}

		Vector3f prevPoint = path[0];
		for (uint32_t i = 1; i < std::min<uint32_t>(_numPointsInSubpath, path.size()); ++i)
		{
			const Vector3f boundDiff = maxBound-minBound;
			const Vector3f pathStep = path[i] - prevPoint;
			const float length = pathStep.norm();
			assert(length > FLT_MIN);
			const Matrix3f scale{{_profile[0].x(), 0,      0},
								 {0,               length, 0},
								 {0,               0,      1}};
			Eigen::Quaternionf rotation = Quaternionf::FromTwoVectors(Vector3f{0,1,0}, pathStep);

			std::vector<Vector3f> rectVertices = squareVertices;
			std::for_each(rectVertices.begin(), rectVertices.end(), [&scale, &rotation, &prevPoint](Vector3f& v){
				v = rotation*(scale*v) + prevPoint;
			});

			const uint32_t firstSquareIndex = (i-1)*4;
			*pi++ = firstSquareIndex + 0;
			*pi++ = firstSquareIndex + 1;
			*pi++ = firstSquareIndex + 2;
			*pi++ = firstSquareIndex + 0;
			*pi++ = firstSquareIndex + 2;
			*pi++ = firstSquareIndex + 3;

			auto setTriangleVertex = [&p, &rectVertices](const unsigned index) {
				const Vector3f vertex = rectVertices[index];
				*p++ = vertex.x();
				*p++ = vertex.y();
				*p++ = vertex.z();
				updateBounds(p-3);
			};

			setTriangleVertex(0);
			setTriangleVertex(1);
			setTriangleVertex(2);
			setTriangleVertex(3);

			prevPoint = path[i];
		}
	}
//	std::cout << " ######### bounds x : " << minBound.x() << "," << maxBound.x() << std::endl;
//	std::cout << " ######### bounds y : " << minBound.y() << "," << maxBound.y() << std::endl;
//	std::cout << " ######### bounds z : " << minBound.z() << "," << maxBound.z() << std::endl;
	setBounds({minBound.x(), minBound.y(), minBound.z()}, {maxBound.x(), maxBound.y(),maxBound.z()});

	setVertexData(vertices);
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
	emit modelLoaded();
	std::cout << __FUNCTION__ << std::endl;
}

void GCodeGeometry::setRectProfile(const Real width, const Real height)
{
	const Vector3f start = {-width/Real(2.0), -height/Real(2.0), Real(0.0)};
	_profile = {start, start + Vector3f{0.0, height, 0.0}, start + Vector3f{width, height, 0.0}, start + Vector3f{width, 0.0, 0.0}};
};

QT_END_NAMESPACE
