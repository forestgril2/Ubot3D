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
	Q_PROPERTY(uint32_t numPathPointsUsed READ getNumPathPointsUsed WRITE setNumPathStrokesUsed NOTIFY numPathPointsUsedChanged)
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
	unsigned getNumPointsInSubPath() const;

	void setNumPathStrokesUsed(const uint32_t num);
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
	void numPathPointsUsedChanged();

private:
	void loadGCodeProgram();
    void updateData();
	void generateTriangles();
	void generateSubPathTriangles(const Eigen::Vector3f& prevPoint, const Eigen::Vector3f& pathStep, const uint32_t firstStructIndexInPathStep, float*& coordsPtr, uint32_t*& indicesPtr);
	void createExtruderPaths(const gpr::gcode_program& gcodeProgram);
	void setRectProfile(const Real width, const Real height);
	void dumpSubPath(const std::string& blockString, const std::vector<Eigen::Vector3f>& subPath);

	QByteArray _allIndices;
	QByteArray _allModelVertices;
	std::vector<std::vector<Eigen::Vector3f>> _extruderSubPaths; /** Vectors of points along the center of the filament path. */
	std::vector<Eigen::Vector3f> _profile; /** Defines a cross section of the filament path boundary (along the z-direction). */
	uint32_t _numPathPointsUsed = 0;

	unsigned _numSubPaths = 0;
	unsigned _numPointsInSubPath = 0;
	bool _isPicked = false;
	bool _areTrianglesReady = false;

	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::MeshSubset> m_subsets;
	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::Joint> m_joints;

	QString _inputFile = "C:/Projects/Ubot3D/CE3_mandoblasterlow.gcode";
};
