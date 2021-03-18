#pragma once

#include <QQuick3DGeometry>
#include <QMatrix4x4>
#include <QVector3D>

#include <qqml.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <Eigen/Geometry>

//#include <D:\Projects\qt6-a80e52\qtquick3d\src\runtimerender\qssgrenderray_p.h>
//#include <D:\Projects\qt6-a80e52\qtquick3d\src\assetimport\qssgmeshbvhbuilder_p.h>

struct aiScene;
class QSSGMeshBVH;

class TriangleGeometry : public QQuick3DGeometry
{
	Q_OBJECT
	QML_NAMED_ELEMENT(TriangleGeometry)

	Q_PROPERTY(bool normals READ normals WRITE setNormals NOTIFY normalsChanged)
	Q_PROPERTY(float normalXY READ normalXY WRITE setNormalXY NOTIFY normalXYChanged)
	Q_PROPERTY(bool uv READ uv WRITE setUV NOTIFY uvChanged)
	Q_PROPERTY(float uvAdjust READ uvAdjust WRITE setUVAdjust NOTIFY uvAdjustChanged)
	Q_PROPERTY(float warp READ warp WRITE setWarp NOTIFY warpChanged)
	Q_PROPERTY(QVector3D minBounds READ minBounds WRITE setMinBounds NOTIFY boundsChanged)
	Q_PROPERTY(QVector3D maxBounds READ maxBounds WRITE setMaxBounds NOTIFY boundsChanged)
	Q_PROPERTY(bool isPicked READ isPicked WRITE setPicked NOTIFY isPickedChanged)
	Q_PROPERTY(QString inputFile READ getInputFile WRITE setInputFile)// NOTIFY inputFileChanged)
	Q_PROPERTY(QList<QVector3D> overhangingVertices READ getOverhangingVertices NOTIFY overhangingVerticesChanged)// NOTIFY inputFileChanged)

public:
	TriangleGeometry();

	static Q_INVOKABLE void exportModelToSTL(const QString& filePath);


	struct PickResult
	{
		bool isPick = false;
		QVector3D pickPos;
	};

	Q_INVOKABLE QVariantMap getPick(const QVector3D& origin,
									const QVector3D& direction,
									const QMatrix4x4& globalTransform);

	QVector<QVector3D> getOverhangingVertices() const;

	const aiScene* getAssimpScene() const;

	QString getInputFile() const;
	void setInputFile(const QString& url);

	bool normals() const { return _hasNormals; }
	void setNormals(bool enable);

	float normalXY() const { return _normalXY; }
	void setNormalXY(float xy);

	bool uv() const { return _hasUV; }
	void setUV(bool enable);

	float uvAdjust() const { return _uvAdjust; }
	void setUVAdjust(float f);

	float warp() const { return _warp;}
	void setWarp(float warp);

	void setBounds(const QVector3D &min, const QVector3D &max);
	QVector3D minBounds() const;
	QVector3D maxBounds() const;

	bool isPicked() const;
	void setPicked(const bool isPicked);

public slots:
	void setMinBounds(const QVector3D& minBounds);
	void setMaxBounds(const QVector3D& maxBounds);

signals:
	void normalsChanged();
	void normalXYChanged();
	void uvChanged();
	void uvAdjustChanged();
	void warpChanged();
	void boundsChanged();
	void modelLoaded();
	void isPickedChanged();
	void overhangingVerticesChanged();

private:
	bool importModelFromFile(const std::string& pFile);
	void updateAllMeshBounds(const aiScene* scene, const unsigned meshIndex = 0u);
	void updateBounds(const float* vertexMatrixXCoord);
	void updateData();
	void buildIntersectionData();
	void reloadSceneIfNecessary();

	int performTriangulation();
	int createCgalMesh();

	void logBounds();

	QSSGMeshBVH* _intersectionData = nullptr;

	Assimp::Importer importer;
	const aiScene* _scene = nullptr;

	Eigen::Vector4f _baseModelColor = {1, 1, 1, 1};
	Eigen::Vector4f _overhangColor = {1, 0.3f, 0, 1};

	aiVector3D _maxFloatBound;
	aiVector3D _minFloatBound;

	aiVector3D _maxBound;
	aiVector3D _minBound;

	const uint32_t _indexAttributeIndex = 2;

	float _overhangAngleMax = float(M_PI_4);
	QList<QVector3D> _overhangingVertices;

	bool _hasColors = true;
	bool _hasNormals = false;
	float _normalXY = 0.0f;
	bool _hasUV = false;
	float _uvAdjust = 0.0f;
	float _warp = 0.0f;

	bool _isPicked = false;
	bool _isAssimpReadDone = false;

//	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::MeshSubset> m_subsets;
//	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::Joint> m_joints;

	QString _inputFile;
	//	QString _inputFile = "C:/ProjectsData/stl_files/mandoblasterlow.stl";
};

class PointGeometry : public QQuick3DGeometry
{
	Q_OBJECT
	QML_NAMED_ELEMENT(PointGeometry)

public:
	Q_PROPERTY(QList<QVector3D> points READ getPoints WRITE setPoints NOTIFY pointsChanged)

	PointGeometry();

	Q_INVOKABLE void updateData();

	QList<QVector3D> getPoints() const;
	void setPoints(QList<QVector3D> newPoints);

signals:
	void pointsChanged();

private:
	QList<QVector3D> _points;
};
