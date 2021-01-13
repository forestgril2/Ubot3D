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
using Point = Eigen::Vector3f;
using Vertices = std::vector<Vertex>;
using Points = std::vector<Point>;
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

	QString getInputFile() const;
	void setInputFile(const QString& url);

    void setBounds(const QVector3D &min, const QVector3D &max);
    QVector3D minBounds() const;
	QVector3D maxBounds() const;

	bool isPicked() const;
	void setPicked(const bool isPicked);

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
	void loadGCodeProgram();
	void setRectProfile(const Real width, const Real height);
	void createExtruderPaths(const gpr::gcode_program& gcodeProgram);
	size_t calcVerifyModelNumbers();
    void updateData();
	void generateSubPathCurve(QByteArray& modelVertices,
							  QByteArray& modelIndices);
	void generateSubPathStep(const Point& prevPoint,
							 const Eigen::Vector3f& pathStep,
							 QByteArray& modelVertices,
							 QByteArray& modelIndices);
	void generate();
	void dumpSubPath(const std::string& blockString, const Points& subPath);

	bool _isPicked = false;
	bool _wasGenerated = false;

	uint32_t _numSubPaths = 0;
	uint32_t _maxNumPointsInSubPath = 0;
	std::vector<Points> _extruderSubPaths;            /** Vectors of points along the center of the filament path. */
	uint32_t _numPathStepsUsed;
	QByteArray _modelIndices;
	QByteArray _modelVertices;
	std::vector<uint32_t> _numTotalPathStepVertices; /** Remember how many vertices are added in each consecutive path step. */
	std::vector<uint32_t> _numTotalPathStepIndices;  /** Remember how many indices are added in each consecutive path step. */
	Vertices _profile;                               /** Defines a cross section of the filament path boundary (along the z-direction). */

	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::MeshSubset> m_subsets;
	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::Joint> m_joints;

	QString _inputFile = "C:/Projects/Ubot3D/CE3_mandoblasterlow.gcode";
};
