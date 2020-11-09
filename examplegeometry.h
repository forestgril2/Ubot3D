#ifndef EXAMPLEGEOMETRY_H
#define EXAMPLEGEOMETRY_H

#include <QQuick3DGeometry>
#include <QVector3D>
#include <QQuaternion>
#include <qqml.h>

class ExampleTriangleGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(bool normals READ normals WRITE setNormals NOTIFY normalsChanged)
    Q_PROPERTY(float normalXY READ normalXY WRITE setNormalXY NOTIFY normalXYChanged)
    Q_PROPERTY(bool uv READ uv WRITE setUV NOTIFY uvChanged)
    Q_PROPERTY(float uvAdjust READ uvAdjust WRITE setUVAdjust NOTIFY uvAdjustChanged)
	Q_PROPERTY(float warp READ warp WRITE setWarp NOTIFY warpChanged)
	Q_PROPERTY(QVector3D minBounds READ minBounds WRITE setMinBounds NOTIFY boundsChanged)
	Q_PROPERTY(QVector3D maxBounds READ maxBounds WRITE setMaxBounds NOTIFY boundsChanged)
	Q_PROPERTY(QString inputFile READ getInputFile WRITE setInputFile)// NOTIFY inputFileChanged)
	QML_NAMED_ELEMENT(ExampleTriangleGeometry)

public:
    ExampleTriangleGeometry();

	static Q_INVOKABLE QQuaternion getRotationFromDirection(const QVector3D& direction, const QVector3D& up);
	static Q_INVOKABLE QQuaternion getRotationFromAxes(const QVector3D& axisFrom, const QVector3D& axisTo);
	static Q_INVOKABLE QQuaternion getRotationFromAxisAndAngle(const QVector3D& axis, const float angle);
	static Q_INVOKABLE QQuaternion getRotationFromQuaternions(const QQuaternion& current, const QQuaternion& additional);
	static Q_INVOKABLE float getSmallRotationAngle(const QVector3D& from, const QVector3D& to);
	static Q_INVOKABLE QVector3D getRotationAxis(const QVector3D& from, const QVector3D& to);
	static Q_INVOKABLE QVector3D getRotationAxis(const QQuaternion& rotation);
	static Q_INVOKABLE float getRotationAngle(const QQuaternion& rotation);


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

private:
    void updateData();
	void reloadSceneIfNecessary();

    bool m_hasNormals = false;
    float m_normalXY = 0.0f;
    bool m_hasUV = false;
    float m_uvAdjust = 0.0f;
    float _warp = 0.0f;

	QString _inputFile = "C:/ProjectsData/stl_files/mandoriflelow.stl";
};

class ExamplePointGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ExamplePointGeometry)
//	QML_ELEMENT

public:
    ExamplePointGeometry();
	Q_INVOKABLE void updateData();
};

#endif
