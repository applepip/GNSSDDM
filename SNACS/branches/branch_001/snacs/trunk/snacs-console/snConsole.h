/*
 * snConsole.h
 *
 *  Created on: Apr 26, 2012
 *      Author: frank
 */

#ifndef SNCONSOLE_H_
#define SNCONSOLE_H_

#include <QString>
#include <QTimer>

#include "../snModules/snModule.h"
#include "../snSimulation/snSimulation.h"

class snConsole: public QObject {
	Q_OBJECT

public:
	snConsole(snModule::LogType _log_type);
	virtual ~snConsole();

	void load_config_file(QString filename);
	void start_simulation();

public slots:
	void log(const QString &s, snModule::LogType d = snModule::info);
	void stop_simulation();
	void updateSimProgressBar();

private:
	snModule::LogType log_type;
	snSimulation *snSimu;
	std::vector<snModulePtr> snModules;
	QTimer *SimProgressBarTimer;
	QTime *SimTime;
};

#endif /* SNCONSOLE_H_ */
