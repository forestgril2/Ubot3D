#include "ProcessLauncher.h"

#include <iostream>
#include <string>

#include <QCoreApplication>
#include <QDir>
#include <QProcess>

#include "TriangleGeometry.h"
#include "NaiveSlicerHeaders.h"

static const std::string kUbotSlicerDir = "UBotSlicer";
static const std::string kUbotSlicerProgram = "UBotSlicer.exe";
static const std::string kUbotSlicerSuccessString = "Plik zostal skonwertowany i zapisany: ";
static const std::string kGCodeExtension = ".gcode";

ProcessLauncher::ProcessLauncher(QObject *parent) : QObject(parent)
{

}

void ProcessLauncher::generateGCode(const QString& stlFilePath, bool isTwoHeaderExtrusion)
{
	static const std::string slicerDirFull = QDir::currentPath().toStdString() + "/../" + kUbotSlicerDir;
	static const std::string slicerCommandFull = slicerDirFull + "/" + kUbotSlicerProgram;
	static const QString program = QString::fromStdString(slicerCommandFull);
	QStringList arguments;

	const QString filePathArgument = QString("-in--") + stlFilePath;
	const QString extruderNumArguments = QString("-headers--") + (isTwoHeaderExtrusion ? QString("2") : QString("1"));

	arguments << filePathArgument;
	arguments << extruderNumArguments;

	QProcess *myProcess = new QProcess(this);
	myProcess->setWorkingDirectory(QString::fromStdString(slicerDirFull));
	myProcess->start(program, arguments);
	myProcess->waitForFinished();

	// Now parse standard output, to see, if we have succeeded.
	// TODO: QDebug correctly detects format of the standard output, but maybe we should use some more direct metods.
	const QString slicerOutput = QDebug::toString(myProcess->readAllStandardOutput());
	static const QString slicerSuccessString = QString::fromStdString(kUbotSlicerSuccessString);

	if (!slicerOutput.contains(slicerSuccessString))
	{
		emit slicerError(slicerOutput);
		std::cout << " ### " << __FUNCTION__ << " ERROR, slicer output: " << slicerOutput.toStdString() << std::endl;
		return;
	}

	const QString slicerOutputPath = slicerOutput.split(slicerSuccessString).back()
												 .split(QString::fromStdString(kGCodeExtension)).front() +
												  QString::fromStdString(kGCodeExtension);
	emit gcodeGenerated(slicerOutputPath);
}

void ProcessLauncher::generateSlices(TriangleGeometry* g)
{
    using namespace Slicer;

    NaiveSlicer slicer;
    auto layers = slicer.slice(*g);

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
            g->_triangleIslandBoundaries.push_back(vv);
        }
        g->raftGeometriesChanged();
    };

    showYellowDebugs(layers, g);


    NaivePerimeterGenerator perGen(layers);
    perGen.generate();
}
