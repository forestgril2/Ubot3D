#ifndef PROCESSLAUNCHER_H
#define PROCESSLAUNCHER_H

#include <QObject>
#include <qqml.h>
#include "TriangleGeometry.h"

#include <json.hpp>
// for convenience
using json = nlohmann::json;

class ProcessLauncher : public QObject
{
	Q_OBJECT
	QML_NAMED_ELEMENT(ProcessLauncher)

	void appendCliArgument(const nlohmann::json& param, QStringList& arguments);

public:
	explicit ProcessLauncher(QObject *parent = nullptr);

	Q_INVOKABLE void generateGCode(const QString& slicerExecPath, const QString& stlFilePath, const QString& paramsFilePath);
	Q_INVOKABLE void generateSlices(TriangleGeometry* geometry);

signals:
	void gcodeGenerated(const QString& outputFilePath);
	void slicerError(const QString& slicerStdOutput);

};

#endif // PROCESSLAUNCHER_H
