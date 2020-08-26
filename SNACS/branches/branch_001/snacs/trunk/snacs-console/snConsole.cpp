/*
 * snConsole.cpp
 *
 *  Created on: Apr 26, 2012
 *      Author: frank
 */

#include "snConsole.h"
//#include <QApplication>
#include <QtCore/QCoreApplication>
#include <QTime>
#include <QMetaType>

#include "snWidgetConsole.h"

snConsole::snConsole(snModule::LogType _log_type) :
		log_type(_log_type) {

}

snConsole::~snConsole() {
	delete snSimu;
	delete SimProgressBarTimer;
	delete SimTime;
}

void snConsole::load_config_file(QString filename) {
	// Load configuration file:
	libconfig::Config cfg;

	log(QString("loading configuration file ") + QString(filename));
	cfg.readFile(filename.toStdString().c_str());
	libconfig::Setting &set = cfg.lookup("Simulation");

	console_widget_factory widget_factory;

	snSimu = new snSimulation();
	snSimu->set_log_level(log_type);
	snModules = snSimu->setup_simulation(set, &widget_factory);
//	std::vector<snWidgetConsolePtr> snWs = widget_factory.get_created_widgets();
//	std::cout << "# widgets: " << snWs.size() << "\n";
//	snWs.at(0)->requestPlotFlag();
//	std::cout << "# widgets: " << snWs.size() << "\n";

	qRegisterMetaType<snModule::LogType>("snModule::LogType"); // to send as parameter in signals.

	// connect log signal of snSimulation to log slot:
	connect(snSimu, SIGNAL(LogSignal(const QString &, const snModule::LogType &)), this,
			SLOT(log(const QString &, const snModule::LogType &)));

	log("Setup complete.");

}

void snConsole::start_simulation() {
	log("Starting  threads...");
	for (size_t i = 0; i < snModules.size(); i++) {
		snModules.at(i)->start();
	}
	log("done...");

// when output blocks exits: stop progress bar
	connect(snModules.back().get(), SIGNAL(finished()), this, SLOT(stop_simulation()));

	SimTime = new QTime;
	SimTime->start();

	SimProgressBarTimer = new QTimer(); // QTimer updates progress bar periodically
	QObject::connect(SimProgressBarTimer, SIGNAL(timeout()), this, SLOT(updateSimProgressBar()));
	SimProgressBarTimer->start(1000);

}

void snConsole::updateSimProgressBar() {
	// plot program run time
	const double vRunTime = SimTime->elapsed() / 1000.0;

	const double vTotalTime = snModules.back()->get_total_simulation_time();

	// plot simulated time
	const double vSimTime = snModules.back()->get_current_simulation_time();

	log(
			"Program run time: " + QString::number(vRunTime, 'f', 2) + " s" + ", simulated time: "
					+ QString::number(vSimTime, 'f', 2) + " s, simulation factor: "
					+ QString::number(vSimTime / vRunTime, 'f', 2) + ", "
					+ QString::number(static_cast<int>((vSimTime / vTotalTime) * 100.0), 'f', 2) + "% done.");

}

void snConsole::stop_simulation() {
	log("waiting for threads to finish...");

// tell the signal source (snBlocks->at(0)) to quit, signal will end and all other threads will quit then:
	snModules.at(0)->requestStop();
	for (size_t i = 0; i < snModules.size(); i++) {
		snModules.at(i)->wait();
	}

	SimProgressBarTimer->stop();

	log("All threads finished. End of simulation.");

	qApp->quit();
}

void snConsole::log(const QString &s, snModule::LogType d) {
/*
	if (d == snModule::debug && DebugLevel < 1) // do not print debug msg
		return;
	if (d == snModule::debugv && DebugLevel < 2) // do not print debug msg
		return;
*/


/*	std::string msg;
	switch (d) {
	case snModule::critical:
		msg = "error: ";
		break;
	case snModule::warning:
		msg = "warning: ";
		break;
	case snModule::info:
		msg = "info: ";
		break;
	case snModule::debug:
		msg = "debug: ";
		break;
	case snModule::debugv:
		msg = "debug: ";
		break;
	}*/

	std::cout << s.toStdString() << std::endl;

	if (d == snModule::critical) {
		// exit now:
//		qApp->quit(); // why does his not work the first time it is called, from constructor?
		std::abort();
	}
}
