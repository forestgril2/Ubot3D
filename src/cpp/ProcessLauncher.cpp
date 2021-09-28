#include "ProcessLauncher.h"

#include <iostream>
#include <string>

#include <QCoreApplication>
#include <QDir>
#include <QProcess>

#include "TriangleGeometry.h"
#include "NaiveSlicerHeaders.h"

#include <json.hpp>

// for convenience
using json = nlohmann::json;


static const std::string kUbotSlicerSuccessString = "Exporting G-code to ";
static const std::string kGCodeExtension = ".gcode";

ProcessLauncher::ProcessLauncher(QObject *parent) : QObject(parent)
{

}

void ProcessLauncher::generateGCode(const QString& slicerExecPath, const QString& stlFilePath, const QString& paramsFilePath)
{
	if (slicerExecPath.isEmpty())
	{//TODO: All these errors should be bassed to the qml frontend.
		std::cout << " ### " << __FUNCTION__ << " ERROR: slicer executable not specified. " << std::endl;
		return;
	}

	QStringList arguments;

	std::ifstream paramsFileStream(paramsFilePath.toStdString());
	if (!paramsFileStream.is_open())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR: cannot open file for reading: " << paramsFilePath.toStdString() << std::endl;
		return;
	}
	json paramGroups = json::parse(paramsFileStream);
	paramsFileStream.close();

	for (const auto& paramGroup : paramGroups)
	{
		for (const auto& param : paramGroup["params"])
		{
			std::string value = param["value"].dump();

			if (value.empty() || value == "\"\"")
				continue;

			if (value.starts_with("\""))
			{
				value = value.substr(1, value.length() -2);
			}
			const std::string cliSwitch = param["cliSwitchLong"];

			if (param["value"] == false)
				continue;

			arguments << QString::fromStdString(cliSwitch);

			if (param["value"] == true)
				continue;

			arguments << QString::fromStdString(value);
		}
	}

	arguments << stlFilePath;

	qDebug() << arguments;

	QProcess *myProcess = new QProcess(this);
//	myProcess->setWorkingDirectory(QString::fromStdString(slicerDirFull));
	myProcess->start(slicerExecPath, arguments);

	if (QProcess::Running != myProcess->state())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR: slicer executable process could not be started. " << std::endl;
		return;
	}
	myProcess->waitForFinished();

	// Now parse standard output, to see, if we have succeeded.
	// TODO: QDebug correctly detects format of the standard output, but maybe we should use some more direct metods.
	const QString slicerOutput = QDebug::toString(myProcess->readAllStandardOutput());
	static const QString slicerSuccessString = QString::fromStdString(kUbotSlicerSuccessString);

//	if (!slicerOutput.contains(slicerSuccessString))
//	{
//		emit slicerError(slicerOutput);
		std::cout << " ### " << __FUNCTION__ << " ERROR, slicer output: " << slicerOutput.toStdString() << std::endl;
		std::cout << " ### " << __FUNCTION__ << " slicer myProcess->: " << myProcess->nativeArguments().toStdString() << std::endl;
//		return;
//	}

	const QString slicerOutputPath = slicerOutput.split(slicerSuccessString).back()
												 .split(QString::fromStdString(kGCodeExtension)).front() +
												  QString::fromStdString(kGCodeExtension);
	emit gcodeGenerated(slicerOutputPath);
}

void ProcessLauncher::generateSlices(TriangleGeometry* geometry)
{
    using namespace Slicer;

    NaiveSlicer slicer;
	auto layers = slicer.slice(*geometry);

    auto showYellowDebugs = [](auto& layers, auto& g){
        for(auto const& layer : layers)
        {
            QVector<QVector3D> vv;
            for(auto const& l : layer.polylines)
            {
                for(auto const& seg : l)
                {
                    vv.emplace_back(seg.x(), seg.y(), seg.z());
                }
            }
			g->getTriangleIslandBoundaries().push_back(vv);
        }
        g->raftGeometriesChanged();
    };

    //showYellowDebugs(layers, g);

    NaivePerimeterGenerator perGen(layers);
	perGen.generate(*geometry);
}
