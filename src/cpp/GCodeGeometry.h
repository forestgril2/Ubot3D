#pragma once

#include <QQuick3DGeometry>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>

#include <qqml.h>

#include <CommonDefs.h>
#include <Extrusion.h>

class GCodeGeometry : public QQuick3DGeometry
{
	Q_OBJECT
	QML_NAMED_ELEMENT(GCodeGeometry)

	Q_PROPERTY(QVector3D minBounds READ minBounds WRITE setMinBounds NOTIFY boundsChanged)
	Q_PROPERTY(QVector3D maxBounds READ maxBounds WRITE setMaxBounds NOTIFY boundsChanged)
	Q_PROPERTY(bool isPicked READ isPicked WRITE setPicked NOTIFY isPickedChanged)
	Q_PROPERTY(QString inputFile READ getInputFile WRITE setInputFile)// NOTIFY inputFileChanged)
	Q_PROPERTY(uint32_t numSubPaths READ getNumSubPaths WRITE setNumSubPaths NOTIFY numSubPathsChanged)
	Q_PROPERTY(uint32_t numPointsInSubPath READ getNumPointsInSubPath WRITE setNumPointsInSubPath NOTIFY numPointsInSubPathChanged)
	Q_PROPERTY(uint32_t numPathStepsUsed READ getNumPathPointsUsed WRITE setNumPathStepsUsed NOTIFY numPathPointsStepsChanged)
	Q_PROPERTY(QList<GCodeGeometry*> subGeometries READ getSubGeometries NOTIFY subGeometriesChanged)

public:
	GCodeGeometry();
	~GCodeGeometry();

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

	QList<GCodeGeometry*> getSubGeometries();

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
	void subGeometriesChanged();

private:
	// This constructor is private - to create extruder data for subgeometries.
	GCodeGeometry(const Extrusion& extruderData);

	void initialize();
	void reset();
	void updateData();
	void loadExtruderData();
	void generate();
	Eigen::Vector3f calculateSubpathCuboid(const Extrusion::Point& pathStart,
											   const Extrusion::Point& pathEnd,
											   const float pathBaseLevelZ);
	size_t calcVerifyModelNumbers();
	bool verifyEnoughPoints(const Extrusion::Path& subPath);
	void generateSubPathTurn(const Extrusion::Point& center,
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
	float getLayerBottom(const uint32_t layerIndex);
	void logSubPath(const Extrusion::Path& path);

	bool _isPicked = false;
	bool _wasGenerated = false;

	Extrusion _extrData;
	QList<GCodeGeometry*> _subGeometries;

	uint32_t _numPathStepsUsed;

	QByteArray _modelIndices;
	QByteArray _modelVertices;
	std::vector<uint32_t> _numTotalPathStepVertices;              /** Remember how many vertices are added in each consecutive path step. */
	std::vector<uint32_t> _numTotalPathStepIndices;               /** Remember how many indices are added in each consecutive path step. */

	QString _inputFile;
};
