#pragma once

#include <QQuick3DGeometry>
#include <QMatrix4x4>
#include <QVector3D>

#include <qqml.h>

#include <assimp/scene.h>

//#include <D:\Projects\qt6-a80e52\qtquick3d\src\runtimerender\qssgrenderray_p.h>
//#include <D:\Projects\qt6-a80e52\qtquick3d\src\assetimport\qssgmeshbvhbuilder_p.h>

class aiScene;
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


	QString getInputFile() const;
	void setInputFile(const QString& url);

    bool normals() const { return m_hasNormals; }
    void setNormals(bool enable);

    float normalXY() const { return m_normalXY; }
    void setNormalXY(float xy);

    bool uv() const { return m_hasUV; }
    void setUV(bool enable);

    float uvAdjust() const { return m_uvAdjust; }
    void setUVAdjust(float f);

    float warp() const { return _warp;}
    void setWarp(float warp);

    void setBounds(const QVector3D &min, const QVector3D &max);
    QVector3D minBounds() const;
	QVector3D maxBounds() const;

	bool isPicked() const;
	void setPicked(const bool isPicked)
	{
		if (_isPicked == isPicked)
			return;

		_isPicked = isPicked;
		isPickedChanged();
	}

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

private:
	void updateAllMeshBounds(const aiScene* scene, const unsigned meshIndex = 0u);
	void updateBounds(const float* vertexMatrixXCoord);
    void updateData();
	void buildIntersectionData();
	void reloadSceneIfNecessary();

	void logBounds();

	QSSGMeshBVH* _intersectionData = nullptr;
	const aiScene* scene = nullptr;

	aiVector3D maxFloatBound;
	aiVector3D minFloatBound;

	aiVector3D maxBound;
	aiVector3D minBound;

	const uint32_t _indexAttributeIndex = 2;

	bool m_hasColors = true;
    bool m_hasNormals = false;
    float m_normalXY = 0.0f;
    bool m_hasUV = false;
    float m_uvAdjust = 0.0f;
    float _warp = 0.0f;

	bool _isPicked = false;
	bool isAssimpReadDone = false;

//	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::MeshSubset> m_subsets;
//	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::Joint> m_joints;

	QString _inputFile;
	//	QString _inputFile = "C:/ProjectsData/stl_files/mandoblasterlow.stl";
	bool importModelFromFile(const std::string& pFile);
};

//class PointGeometry : public QQuick3DGeometry
//{
//    Q_OBJECT
//    QML_NAMED_ELEMENT(ExamplePointGeometry)
////	QML_ELEMENT

//public:
//	PointGeometry();
//	Q_INVOKABLE void updateData();
//};
