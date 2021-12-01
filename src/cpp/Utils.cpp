#include "Utils.h"

#include <fstream>
Utils::Utils(QObject *parent) : QObject(parent)
{

}

QString Utils::generateSystemFilePath(const QString &filePath)
{
	qsizetype pos = filePath.indexOf(R"(file:///)");
#ifdef Q_OS_WIN
    return filePath.sliced(pos+8);
#else
    return filePath.sliced(pos+7);
#endif
}

QString Utils::getDefaultSlicerExecutablePath()
{
	std::ifstream slicerPathFile("C:\\Projects\\slicerPath.txt");
	std::string path((std::istreambuf_iterator<char>(slicerPathFile)), std::istreambuf_iterator<char>());
	return QString::fromStdString(path);
}
