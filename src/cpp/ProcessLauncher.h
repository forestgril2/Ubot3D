#ifndef PROCESSLAUNCHER_H
#define PROCESSLAUNCHER_H

#include <QObject>
#include <qqml.h>

class ProcessLauncher : public QObject
{
	Q_OBJECT
	QML_NAMED_ELEMENT(ProcessLauncher)

public:
	explicit ProcessLauncher(QObject *parent = nullptr);

	Q_INVOKABLE void generateGCode();

signals:

};

#endif // PROCESSLAUNCHER_H
