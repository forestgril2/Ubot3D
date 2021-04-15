#include "ProcessLauncher.h"

#include <iostream>
#include <string>

#include <QCoreApplication>
#include <QDir>
#include <QProcess>

static const std::string kUbotSlicerDir = "UBotSlicer";
static const std::string kUbotSlicerProgram = "UBotSlicer.exe";
static const std::string kUbotSlicerSuccessString = "Plik zostal skonwertowany i zapisany: ";
static const std::string kGCodeExtension = ".gcode";

ProcessLauncher::ProcessLauncher(QObject *parent) : QObject(parent)
{

}

void ProcessLauncher::generateGCode()
{
	static const std::string slicerDirFull = QDir::currentPath().toStdString() + "/../" + kUbotSlicerDir;
	static const std::string slicerCommandFull = slicerDirFull + "/" + kUbotSlicerProgram;
	static const QString program = QString::fromStdString(slicerCommandFull);
	QStringList arguments;

	arguments << "-in--C:/ProjectsData/stl_files/mandoblasterlow.stl";
	arguments << "-headers--1";

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
	gCodeGenerated(slicerOutput);
	std::cout << " ### " << __FUNCTION__ << " slicerOutputPath: " << slicerOutputPath.toStdString() << std::endl;
}
