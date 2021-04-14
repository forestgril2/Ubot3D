#include <cstdio>
#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQuick3D/qquick3d.h>

#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtCore/QVector>

#include <Chronograph.h>
#include <Helpers3D.h>
#include <ProcessLauncher.h>

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	QSurfaceFormat::setDefaultFormat(QQuick3D::idealSurfaceFormat());

	QQmlApplicationEngine engine;
	QQmlContext* context = engine.rootContext();

	Helpers3D helpers3D;
	ProcessLauncher launcher;
	context->setContextProperty("helper3D", &helpers3D);
	context->setContextProperty("ProcessLauncher", &launcher);

	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));


	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}
