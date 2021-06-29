#include <cstdio>
#include <iostream>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQuick3D/qquick3d.h>

#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtCore/QVector>

#include <Chronograph.h>
#include <FileImportExport.h>
#include <Helpers3D.h>
#include <ProcessLauncher.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	QSurfaceFormat::setDefaultFormat(QQuick3D::idealSurfaceFormat());

	QQmlApplicationEngine engine;
	QQmlContext* context = engine.rootContext();

	Chronograph::setOutputFile("Chronograph.log");
	Helpers3D helpers3D;
	FileImportExport fileImportExport;
	ProcessLauncher launcher;
	context->setContextProperty("helpers3D", &helpers3D);
	context->setContextProperty("fileImportExport", &fileImportExport);

	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));


	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}
