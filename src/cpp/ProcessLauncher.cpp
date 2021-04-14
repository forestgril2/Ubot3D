#include "ProcessLauncher.h"

#include <iostream>
#include <string>

#include <QCoreApplication>
#include <QDir>
#include <QProcess>

static const std::string kUbotSlicerDir = "UBotSlicer";
static const std::string kUbotSlicerProgram = "UBotSlicer.exe";

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

	qDebug() << myProcess->readAllStandardOutput();


//	std::cout << " ### " << __FUNCTION__ << " slicerCommand:" << program.toStdString() << "," << "" << std::endl;
}
