#pragma once

#include <QQuick3DGeometry>
#include <QMatrix4x4>
#include <QVector3D>

#include <qqml.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <clipper.hpp>

//#include <D:\Projects\qt6-a80e52\qtquick3d\src\runtimerender\qssgrenderray_p.h>
//#include <D:\Projects\qt6-a80e52\qtquick3d\src\assetimport\qssgmeshbvhbuilder_p.h>

#include <CommonDefs.h>
#include <NaiveSlicer.h>

struct aiScene;
struct QSSGMeshBVH;

class PolygonTriangulation;
class TriangleIsland;
struct TriangleGeometryData
{
	std::vector<Vec3> vertices = {};
	std::vector<Vec3> normals = {};
	std::vector<uint32_t> indices = {};
};


struct GeometryVertexStrideData
{
	static uint32_t getStride(const bool hasColors)
	{
		return sizeof(float)*(numFloatsPerPositionAttribute +
							  hasColors * numFloatsPerColorAttribute); }
private:
	static const uint32_t numFloatsPerPositionAttribute = 3u;
	static const uint32_t numFloatsPerColorAttribute = 4u;

};


class TriangleGeometry : public QQuick3DGeometry
{
	Q_OBJECT
	QML_NAMED_ELEMENT(TriangleGeometry)

	Q_PROPERTY(bool isMainGeometry READ isMainGeometry)

	Q_PROPERTY(QVector3D minBounds READ minBounds WRITE setMinBounds NOTIFY boundsChanged)
	Q_PROPERTY(QVector3D maxBounds READ maxBounds WRITE setMaxBounds NOTIFY boundsChanged)
	Q_PROPERTY(bool isPicked READ isPicked WRITE setPicked NOTIFY isPickedChanged)
	Q_PROPERTY(QString inputFile READ getInputFile WRITE setInputFile)// NOTIFY inputFileChanged)

	Q_PROPERTY(QVector<QVector3D> debugTriangleEdges READ getDebugTriangleEdges NOTIFY debugTriangleEdgesChanged)
	Q_PROPERTY(QVector<QVector3D> triangulationResult READ getTriangulationResult NOTIFY triangulationResultChanged)

	Q_PROPERTY(QVector<TriangleGeometry*> supportGeometries READ getSupportGeometries NOTIFY supportGeometriesChanged)
	Q_PROPERTY(bool isSupportGenerated READ isSupportGenerated WRITE setSupportGenerated NOTIFY isSupportGeneratedChanged)

	Q_PROPERTY(QVector<TriangleGeometry*> raftGeometries READ getRaftGeometries NOTIFY raftGeometriesChanged)
	Q_PROPERTY(float raftOffset READ getRaftOffset WRITE setRaftOffset NOTIFY raftOffsetChanged)
	Q_PROPERTY(float raftHeight READ getRaftHeight WRITE setRaftHeight NOTIFY raftHeightChanged)
	Q_PROPERTY(bool areRaftsGenerated READ areRaftsGenerated WRITE setRaftsGenerated NOTIFY areRaftsGeneratedChanged)

	Q_PROPERTY(QVector<QVector<QVector3D>> triangleIslandBoundaries READ getTriangleIslandBoundaries NOTIFY triangleIslandBoundariesChanged)
	Q_PROPERTY(QMatrix4x4 sceneTransform WRITE setSceneTransform NOTIFY sceneTransformChanged)

public:
	TriangleGeometry();
	TriangleGeometry(const TriangleGeometryData& data);

	const TriangleGeometryData& getData() const { return _data; }

	struct PickResult
	{
		bool isPick = false;
		QVector3D pickPos;
	};

	bool isMainGeometry() const;

	void setSceneTransform(const QMatrix4x4& transform);

	void setSupportGenerated(bool isGenerated);
	bool isSupportGenerated() const;

	void setRaftsGenerated(bool isGenerated);
	bool areRaftsGenerated() const;
	float getRaftOffset() const;
	void setRaftOffset (float offset);
	float getRaftHeight() const;
	void setRaftHeight (float height);

	Q_INVOKABLE QVariantMap getPick(const QVector3D& origin,
									const QVector3D& direction,
									const QMatrix4x4& globalTransform);

	QVector<QVector3D> getDebugTriangleEdges() const;
	QVector<QVector3D> getTriangulationResult() const;
	QVector<TriangleGeometry*> getSupportGeometries() const;
	QVector<TriangleGeometry*> getRaftGeometries() const;
	QVector<QVector<QVector3D> >& getTriangleIslandBoundaries();

	const aiScene* getAssimpScene() const;

	QString getInputFile() const;
	void setInputFile(const QString& url);

	void setBounds(const QVector3D &min, const QVector3D &max);
	QVector3D minBounds() const;
	QVector3D maxBounds() const;

	bool isPicked() const;
	void setPicked(const bool isPicked);

	void setMinBounds(const QVector3D& minBounds);
	void setMaxBounds(const QVector3D& maxBounds);

public slots:
	void onZLevelChanged();
	void onBottomLayerChanged();
	void onRaftDataChanged();
	void onIsSupportGeneratedChanged();
	void onAreRaftsGeneratedChanged();

signals:
	void sceneTransformChanged();
	void zLevelChanged();
	void bottomLayerChanged();

	void normalsChanged();
	void normalXYChanged();
	void uvChanged();
	void uvAdjustChanged();
	void boundsChanged();
	void minZBoundsChanged();
	void modelLoaded();
	void isPickedChanged();

	void debugTriangleEdgesChanged();
	void overhangingPointsChanged();
	void triangulationResultChanged();

	void supportGeometriesChanged();
	void isSupportGeneratedChanged(bool isGenerated);

	void raftGeometriesChanged();
	void areRaftsGeneratedChanged(bool isGenerated);
	void raftOffsetChanged(float);
	void raftHeightChanged(float);

	void triangleIslandBoundariesChanged();

private:
	bool importModelFromFile(const std::string& pFile);
	void reloadAssimpScene();
	void updateAllMeshBounds(const aiScene* scene, const unsigned meshIndex = 0u);
	void updateBounds(const float* vertexMatrixXCoord);
	void buildIntersectionData();
	void updateData(const TriangleGeometryData& data);

	void generateSupportGeometries();
	void clearSupportGeometries();
	void generateRaftGeometries();
	void clearRaftGeometries();
	void computeBottomLayer();

	uint32_t calculateAndSetStride();
	std::vector<float> prepareColorTrianglesVertexData();
	void generateDebugTriangleEdges(const std::vector<Vec3>& vertices, const std::vector<uint32_t>& indices);
	std::vector<uint32_t> collectFloorTriangleIndices(const float floorOffsetLimit = 0.025f,
													  const Eigen::Matrix4f& transform = Eigen::Matrix4f::Identity());

	void logBounds();

	// Member variables
	TriangleGeometryData _data;
	QSSGMeshBVH* _intersectionData = nullptr;
	QMatrix4x4 _sceneTransform;

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
	std::vector<uint32_t> _overhangingTriangleIndices;
	QVector<QVector3D> _debugTriangleEdges;
	QVector<QVector3D> _triangulationResult;
	QVector<QVector<QVector3D>> _triangleIslandBoundaries;
	Slicer::Layer _bottomLayer;
	float _supportAlphaValue;
	std::vector<std::shared_ptr<TriangleGeometry>> _supportGeometries;
	std::vector<std::shared_ptr<TriangleGeometry>> _raftGeometries;


	bool _hasColors = true;
	bool _isPicked = false;
	bool _isAssimpReadDone = false;

	bool _isSupportGenerated = false;
	bool _areRaftsGenerated = false;
	float _raftOffset = 0.0;
	float _raftHeight = 0.0;

//	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::MeshSubset> m_subsets;
//	QSSGMeshUtilities::OffsetDataRef<QSSGMeshUtilities::Joint> m_joints;

	QString _inputFile;
	TriangleGeometryData prepareDataFromAssimpScene();
	std::shared_ptr<TriangleGeometry> extrudedTriangleIsland(const TriangleIsland& island);
	float getMinBoundZDistToSceneFloor() const;
	static ClipperLib::Paths offsetClipperPaths(const Slicer::Layer::Polylines& polylines, double offset);
	static TriangleGeometryData computePolygonTriangulationMesh(const ClipperLib::Paths& pathsCl,
																float zLevel,
																const Vec3& meshNormal,
																QVector<QVector<QVector3D>>* debugBoundaries = nullptr);

	bool _isMainGeometry;
};
