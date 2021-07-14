#pragma once

#include <qqml.h>
#include <QObject>

class Utils : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Utils);
public:
    explicit Utils(QObject *parent = nullptr);

    static Q_INVOKABLE QString generateSystemFilePath(const QString& filePath);
};

