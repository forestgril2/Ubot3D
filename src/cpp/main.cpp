#include <cstdio>
#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQuick3D/qquick3d.h>

#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtCore/QVector>

#include <Chronograph.h>

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	QSurfaceFormat::setDefaultFormat(QQuick3D::idealSurfaceFormat());

	QQmlApplicationEngine engine;
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}
