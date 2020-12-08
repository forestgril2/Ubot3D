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
    Q_PROPERTY(bool normals READ normals WRITE setNormals NOTIFY normalsChanged)
    Q_PROPERTY(float normalXY READ normalXY WRITE setNormalXY NOTIFY normalXYChanged)
    Q_PROPERTY(bool uv READ uv WRITE setUV NOTIFY uvChanged)
	Q_PROPERTY(float uvAdjust READ uvAdjust WRITE setUVAdjust NOTIFY uvAdjustChanged)
	Q_PROPERTY(QVector3D minBounds READ minBounds WRITE setMinBounds NOTIFY boundsChanged)
	Q_PROPERTY(QVector3D maxBounds READ maxBounds WRITE setMaxBounds NOTIFY boundsChanged)
	Q_PROPERTY(bool isPicked READ isPicked WRITE setPicked NOTIFY isPickedChanged)
	Q_PROPERTY(QString inputFile READ getInputFile WRITE setInputFile)// NOTIFY inputFileChanged)
	Q_PROPERTY(uint32_t numSubPaths READ getNumSubPaths WRITE setNumSubPaths NOTIFY numSubPathsChanged)
	Q_PROPERTY(uint32_t numPointsInSubPath READ getNumPointsInSubPath WRITE setNumPointsInSubPath NOTIFY numPointsInSubPathChanged)
	Q_PROPERTY(uint32_t numPathPointsUsed READ getNumPathPointsUsed WRITE setNumPathPointsUsed NOTIFY numPathPointsUsedChanged)
	QML_NAMED_ELEMENT(GCodeGeometry)

public:
	GCodeGeometry();

	QString getInputFile() const;
	void setInputFile(const QString& url);

	bool normals() const;
    void setNormals(bool enable);

	float normalXY() const;
    void setNormalXY(float xy);

	bool uv() const;
    void setUV(bool enable);

	float uvAdjust() const;
    void setUVAdjust(float f);

    void setBounds(const QVector3D &min, const QVector3D &max);
    QVector3D minBounds() const;
	QVector3D maxBounds() const;

	bool isPicked() const;
	void setPicked(const bool isPicked);

	void setNumSubPaths(const uint32_t num);
	uint32_t getNumSubPaths() const;

	void setNumPointsInSubPath(const uint32_t num);
	unsigned getNumPointsInSubPath() const;

	void setNumPathPointsUsed(const uint32_t num);
	uint32_t getNumPathPointsUsed() const;

public slots:
    void setMinBounds(const QVector3D& minBounds);
	void setMaxBounds(const QVector3D& maxBounds);

signals:
    void normalsChanged();
    void normalXYChanged();
    void uvChanged();
    void uvAdjustChanged();
	void boundsChanged();
	void modelLoaded();
	void isPickedChanged();
	void numPointsInSubPathChanged();
	void numPathPointsUsedChanged();
	void numSubPathsChanged();

private:
    void updateData();
	void generateTriangles();
	void createExtruderPaths(const gpr::gcode_program& gcodeProgram);
	void setRectProfile(const Real width, const Real height);
	void dumpSubPath(const std::string& blockString, const std::vector<Eigen::Vector3f>& subPath);

	QByteArray _allIndices;
	QByteArray _allModelVertices;
	std::vector<std::vector<Eigen::Vector3f>> _extruderSubPaths; /** Vectors of points along the center of the filament path. */
	std::vector<Eigen::Vector3f> _profile; /** Defines a cross section of the filament path boundary (along the z-direction). */
	uint32_t _numPathPointsUsed = 0;

    bool m_hasNormals = false;
    float m_normalXY = 0.0f;
    bool m_hasUV = false;
    float m_uvAdjust = 0.0f;

	unsigned _numSubPaths = 0;
	unsigned _numPointsInSubPath = 0;
	bool _isPicked = false;
	bool _areTrianglesReady = false;

	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::MeshSubset> m_subsets;
	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::Joint> m_joints;

	QString _inputFile = "C:/Projects/Ubot3D/CE3_mandoblasterlow.gcode";
//	QString _inputFile = "C:/Projects/Ubot3D/TEST.gcode";
	//	QString _inputFile = "C:/ProjectsData/stl_files/CE3_mandoblaster.gcode";
};
