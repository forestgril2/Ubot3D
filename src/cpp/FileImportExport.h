#pragma once

#include <qqml.h>

class FileImportExport : public QObject
{
	Q_OBJECT
	QML_NAMED_ELEMENT(FileImportExport)

public:
	static Q_INVOKABLE bool exportModelsToSTL(const QVariantList& stlExportData, const QString filePath);
};
