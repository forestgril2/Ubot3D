#pragma once

#include <QQuick3DGeometry>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>

#include <Eigen/Core>

#include <qqml.h>

#include <D:\Projects\qt6\qtquick3d\src\runtimerender\qssgrenderray_p.h>
#include <D:\Projects\qt6\qtquick3d\src\assetimport\qssgmeshbvhbuilder_p.h>

using Real = float;
using Vertex = Eigen::Vector3f;
using ExtrPoint = Eigen::Vector4f;
using Vertices = std::vector<Vertex>;
using ExtrPath = std::vector<ExtrPoint>;
using Indices = std::vector<uint32_t>;

namespace gpr
{
	class gcode_program;
};

class GCodeGeometry : public QQuick3DGeometry
{
    Q_OBJECT
	Q_PROPERTY(QVector3D minBounds READ minBounds WRITE setMinBounds NOTIFY boundsChanged)
	Q_PROPERTY(QVector3D maxBounds READ maxBounds WRITE setMaxBounds NOTIFY boundsChanged)
	Q_PROPERTY(bool isPicked READ isPicked WRITE setPicked NOTIFY isPickedChanged)
	Q_PROPERTY(QString inputFile READ getInputFile WRITE setInputFile)// NOTIFY inputFileChanged)
	Q_PROPERTY(uint32_t numSubPaths READ getNumSubPaths WRITE setNumSubPaths NOTIFY numSubPathsChanged)
	Q_PROPERTY(uint32_t numPointsInSubPath READ getNumPointsInSubPath WRITE setNumPointsInSubPath NOTIFY numPointsInSubPathChanged)
	Q_PROPERTY(uint32_t numPathStepsUsed READ getNumPathPointsUsed WRITE setNumPathStepsUsed NOTIFY numPathPointsStepsChanged)
	QML_NAMED_ELEMENT(GCodeGeometry)

public:
	GCodeGeometry();

	void setInputFile(const QString& url);
	QString getInputFile() const;

    void setBounds(const QVector3D &min, const QVector3D &max);
    QVector3D minBounds() const;
	QVector3D maxBounds() const;

	void setPicked(const bool isPicked);
	bool isPicked() const;

	void setNumSubPaths(const uint32_t num);
	uint32_t getNumSubPaths() const;

	void setNumPointsInSubPath(const uint32_t num);
	uint32_t getNumPointsInSubPath() const;

	void setNumPathStepsUsed(const uint32_t num);
	uint32_t getNumPathPointsUsed() const;

public slots:
    void setMinBounds(const QVector3D& minBounds);
	void setMaxBounds(const QVector3D& maxBounds);

signals:
	void boundsChanged();
	void modelLoaded();
	void isPickedChanged();
	void numPointsInSubPathChanged();
	void numSubPathsChanged();
	void numPathPointsStepsChanged();

private:
	void reset();
	void updateData();
	void loadGCodeProgram();
	void generate();
	Eigen::Vector3f calculateSubpathCuboid(const ExtrPoint& pathStart,
											   const ExtrPoint& pathEnd,
											   const float pathBaseLevelZ);
	void createExtruderPaths(const gpr::gcode_program& gcodeProgram);
	size_t calcVerifyModelNumbers();
	bool verifyEnoughPoints(const ExtrPath& subPath);
	void generateSubPathTurn(const ExtrPoint& center,
							 const Eigen::Vector3f& radiusStart,
							 const Eigen::Vector3f& axis,
							 const float angle,
							 const float height,
							 QByteArray& modelVertices,
							 QByteArray& modelIndices);
	void generateSubPathStep(const Eigen::Vector3f& prevPoint,
							 const Eigen::Vector3f& pathStep, const Eigen::Vector3f& cuboid,
							 QByteArray& modelVertices,
							 QByteArray& modelIndices);
	void dumpSubPath(const std::string& blockString, const ExtrPath& subPath);

	bool _isPicked = false;
	bool _wasGenerated = false;

	uint32_t _numSubPaths;
	uint32_t _maxNumPointsInSubPath;
	std::vector<ExtrPath> _extruderSubPaths;            /** Vectors of points along the center of the filament path. */
	uint32_t _numPathStepsUsed;
	QByteArray _modelIndices;
	QByteArray _modelVertices;
	std::vector<uint32_t> _numTotalPathStepVertices; /** Remember how many vertices are added in each consecutive path step. */
	std::vector<uint32_t> _numTotalPathStepIndices;  /** Remember how many indices are added in each consecutive path step. */
	float _filamentCrossArea;                        /** Filament cross-section area in mm^2. */

	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::MeshSubset> m_subsets;
	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::Joint> m_joints;

	QString _inputFile;
};
