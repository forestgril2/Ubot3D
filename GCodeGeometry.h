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
	Q_PROPERTY(unsigned numSubpaths READ getNumSubpaths WRITE setNumSubpaths NOTIFY numSubpathsChanged)
	Q_PROPERTY(unsigned numPointsInSubpath READ getNumPointsInSubpath WRITE setNumPointsInSubpath NOTIFY numPointsInSubpathChanged)
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

	void setNumSubpaths(const unsigned num);
	unsigned getNumSubpaths() const;

	void setNumPointsInSubpath(const unsigned num);
	unsigned getNumPointsInSubpath() const;

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
	void numPointsInSubpathChanged();
	void numSubpathsChanged();


private:
    void updateData();

	void createExtruderPaths(const gpr::gcode_program& gcodeProgram);
	void setRectProfile(const Real width, const Real height);

    bool m_hasNormals = false;
    float m_normalXY = 0.0f;
    bool m_hasUV = false;
    float m_uvAdjust = 0.0f;

	bool _isPicked = false;
	unsigned _numSubpaths = 0;
	unsigned _numPointsInSubpath = 0;

	std::vector<std::vector<Eigen::Vector3f>> _extruderPaths; /** Vectors of points along the center of the filament path. */
	std::vector<Eigen::Vector3f> _profile; /** Defines a cross section of the filament path boundary (along the z-direction). */

	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::MeshSubset> m_subsets;
	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::Joint> m_joints;

	QString _inputFile = "C:/Projects/Ubot3D/CE3_mandoblasterlow.gcode";
//	QString _inputFile = "C:/Projects/Ubot3D/TEST.gcode";
        void updateWait();
};
