#pragma once

#include <QQuick3DGeometry>
#include <QMatrix4x4>
#include <QVector3D>

#include <qqml.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <Eigen/Geometry>
using Vec3 = Eigen::Vector3f;

//#include <D:\Projects\qt6-a80e52\qtquick3d\src\runtimerender\qssgrenderray_p.h>
//#include <D:\Projects\qt6-a80e52\qtquick3d\src\assetimport\qssgmeshbvhbuilder_p.h>

struct aiScene;
class QSSGMeshBVH;

using IndicesToVertices = std::map<Vec3, uint32_t, bool(*)(const Vec3& a, const Vec3& b)>;

class TriangleGeometry : public QQuick3DGeometry
{
	Q_OBJECT
	QML_NAMED_ELEMENT(TriangleGeometry)

	Q_PROPERTY(QVector3D minBounds READ minBounds WRITE setMinBounds NOTIFY boundsChanged)
	Q_PROPERTY(QVector3D maxBounds READ maxBounds WRITE setMaxBounds NOTIFY boundsChanged)
	Q_PROPERTY(bool isPicked READ isPicked WRITE setPicked NOTIFY isPickedChanged)
	Q_PROPERTY(QString inputFile READ getInputFile WRITE setInputFile)// NOTIFY inputFileChanged)
	Q_PROPERTY(QVector<QVector3D> overhangingTriangleVertices READ getOverhangingTriangleVertices NOTIFY overhangingTriangleVerticesChanged)
	Q_PROPERTY(QVector<QVector3D> overhangingPoints READ getOverhangingPoints NOTIFY overhangingPointsChanged)
	Q_PROPERTY(QVector<QVector3D> triangulationResult READ getTriangulationResult NOTIFY triangulationResultChanged)

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

	QVector<QVector3D> getOverhangingTriangleVertices() const;
	QVector<QVector3D> getOverhangingPoints() const;
	QVector<QVector3D> getTriangulationResult() const;

	const aiScene* getAssimpScene() const;

	QString getInputFile() const;
	void setInputFile(const QString& url);

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
	void boundsChanged();
	void modelLoaded();
	void isPickedChanged();

	void overhangingTriangleVerticesChanged();
	void overhangingPointsChanged();
	void triangulationResultChanged();

private:
	bool importModelFromFile(const std::string& pFile);
	void updateAllMeshBounds(const aiScene* scene, const unsigned meshIndex = 0u);
	void updateBounds(const float* vertexMatrixXCoord);
	void updateData();
	void reloadSceneIfNecessary();

	void buildIntersectionData();

	uint32_t calculateAndSetStride();
	void countAssimpFacesAndVertices(uint32_t& numAssimpMeshFaces, uint32_t& numAssimpVertices);
	void getContiguousAssimpVerticesAndNormals(std::vector<Vec3>& assimpVertices,
											   std::vector<Vec3>& assimpNormals);
	IndicesToVertices mapIndicesToUniqueVertices(const std::vector<Vec3>& assimpVertices,
												 const std::vector<Vec3>& assimpNormals,
												 std::vector<Vec3>& uniqueVertices,
												 std::vector<Vec3>& uniqueNormals);
	std::vector<uint32_t> calculateRemappedIndices(const IndicesToVertices& indicesToUniqueVertices,
												   const std::vector<Vec3>& assimpVertices);
	std::vector<float> calculateColorTriangles(const std::vector<Vec3>& uniqueVertices,
											   const std::vector<Vec3>& uniqueNormals);
	std::vector<uint32_t> calculateOverhangingTriangleIndices(const std::vector<Vec3>& normals, const std::vector<uint32_t>& indices);
	void collectOverhangingData(const std::vector<uint32_t>& overhangingTriangleIndices,
								const std::vector<Vec3>& vertices);

	void logBounds();

	// Member variables
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
	QVector<QVector3D> _overhangingTriangleVertices;
	std::vector<uint32_t> _overhangingTriangleIndices;
	QVector<QVector3D> _overhangingPoints;
	QVector<QVector3D> _triangulationResult;

	bool _hasColors = true;

	bool _isPicked = false;
	bool _isAssimpReadDone = false;

//	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::MeshSubset> m_subsets;
//	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::Joint> m_joints;

	QString _inputFile;
	//	QString _inputFile = "C:/ProjectsData/stl_files/mandoblasterlow.stl";
};
