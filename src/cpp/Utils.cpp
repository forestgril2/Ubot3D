#include "Utils.h"

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