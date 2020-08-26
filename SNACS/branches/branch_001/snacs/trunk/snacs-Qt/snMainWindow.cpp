/*
 *  SNACS - The Satellite Navigation Radio Channel Simulator
 *
 *  Copyright (C) 2012  F. M. Schubert
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*!
 * \file	snMainWindow.cpp
 * \brief	Implementations for main window.
 *
 * \author	Frank Schubert
 */

#include "snMainWindow.h"
#include <tclap/CmdLine.h>

snMainWindow::snMainWindow(int &argc, char **argv) {
	showMaximized();

	// setup main window
	DebugLevel = 0;
	quit_when_finished = false;

	Log = new QTextEdit(this);
	Log->setUndoRedoEnabled(false);
	Log->setReadOnly(true);
	Log->moveCursor(QTextCursor::End);
	Log->setFontFamily("Courier");
	Log->setFontPointSize(12);

	QDockWidget *Log_dock = new QDockWidget("log messages");
	Log_dock->setWidget(Log);
	this->addDockWidget(Qt::TopDockWidgetArea, Log_dock);
	//	setCentralWidget(Log);

	log(QString(WelcomeMessage.c_str()));

	log(QString("SNACS ") + SNACSVersion.c_str() + QString(" Copyright (C) 2012  F. M. Schubert"));
	log(QString("This program comes with ABSOLUTELY NO WARRANTY."));
	log(QString("This is free software, and you are welcome to redistribute it"));
	log(QString("under certain conditions. See the licenses/gpl-3.0.txt file for more information."));

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	setSimStatus(NoConfig);

	// parse command line
	QString ConfigFile;
	bool StartSimulation = false;
	int DebugLevel = 0;

	TCLAP::CmdLine cmd("", ' ', SNACSVersion);
//	cmd.setExceptionHandling(false); // false: use external exception handling.

//	try {
	TCLAP::ValueArg<int> debugArg("d", "debug", "debug level (1 or 2)", false, 0, "int", cmd);

	TCLAP::ValueArg<std::string> nameArg("f", "file", "load configuration file", false, "", "string", cmd);

	TCLAP::SwitchArg StartSimulationSwitch("s", "start",
			"start simulation right after loading configuration file", false);
	cmd.add(StartSimulationSwitch);

//	TCLAP::SwitchArg ConsoleSwitchArg("c", "console", "do not use the GUI, print all messages to the console",
//			false);
//	cmd.add(ConsoleSwitchArg);

// Parse the argv array.
	cmd.parse(argc, argv);

	// Get the value parsed by each arg.
	std::string ConfigFileArg = nameArg.getValue();
	ConfigFile = ConfigFileArg.c_str();
	StartSimulation = StartSimulationSwitch.getValue();
	DebugLevel = debugArg.getValue();

//	} catch (TCLAP::ArgException &e) {
////		TCLAP::StdOutput out_message;
////		out_message.usage(cmd);
////		cmd.getMessage();
//		cmd.getOutput()->usage(cmd);
//std::cerr.flush();
//		log(
//				"Error while parsing command line arguments:\n" + QString(e.what()) + ".\n"
//						+ QString(e.typeDescription().c_str()) + "\n" + QString(e.argId().c_str()),
//				snModule::critical);
//	}

	setDebugLevel(DebugLevel);

	if (ConfigFile != "") {
		loadFile(ConfigFile);
		if (StartSimulation && getSimStatus() == snMainWindow::Ready) {
			quit_when_finished = true;
			startSimu();
		}
	} else if (StartSimulation) {
		log("Cannot start simulation: no config file given");
	}
}

snMainWindow::~snMainWindow() {
}

void snMainWindow::createActions() {
	openAction = new QAction(tr("&Open"), this);
	openAction->setShortcut(tr("Ctrl+O"));
	connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));

	startAction = new QAction(tr("&Start"), this);
	startAction->setShortcut(tr("Ctrl+S"));
	startAction->setStatusTip("Start simulation");
	connect(startAction, SIGNAL(triggered()), this, SLOT(startSimu()));

	stopAction = new QAction(tr("&Abort"), this);
	stopAction->setShortcut(tr("Ctrl+A"));
	stopAction->setStatusTip("Stop simulation");
	connect(stopAction, SIGNAL(triggered()), this, SLOT(stopSimu()));

	exitAction = new QAction(tr("&Quit"), this);
	exitAction->setShortcut(tr("Ctrl+Q"));
	exitAction->setStatusTip("Exit the application");
	connect(exitAction, SIGNAL(triggered()), this, SLOT(exitProg()));

	aboutAction = new QAction(tr("&About"), this);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

}

bool snMainWindow::openFile() {
	if (okToContinue()) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open Simulation Configuration File"), ".",
				tr("SNSignal configuration files (*.cfg)"));
		if (!filename.isEmpty()) {
			loadFile(filename);
			return true;
		}
	}
	return false;
}

void snMainWindow::exitProg() {
	qApp->quit();
}

void snMainWindow::createMenus() {
	fileMenu = menuBar()->addMenu("Simulation");
	fileMenu->addAction(openAction);
	fileMenu->addAction(startAction);
	fileMenu->addAction(stopAction);
	fileMenu->addAction(exitAction);

	helpMenu = menuBar()->addMenu("Help");
	helpMenu->addAction(aboutAction);
}

void snMainWindow::createStatusBar() {
	StatusLabel = new QLabel("SNSignal");
	statusBar()->addWidget(StatusLabel);
}

void snMainWindow::createToolBars() {

	ConfigBar = addToolBar("Config");

	ConfigBar->addAction(openAction);

	ToolBar = addToolBar("Control");
	ToolBar->addAction(startAction);
	ToolBar->addAction(stopAction);
	ToolBar->addAction(exitAction);

	ToolBar->addSeparator();

	DebugSpin = new QSpinBox();
	DebugSpin->setRange(0, 2);
	ToolBar->addWidget(new QLabel("Debug level: "));
	ToolBar->addWidget(DebugSpin);

	connect(DebugSpin, SIGNAL(valueChanged(int)), this, SLOT(setDebugLevel(int)));

	ToolBar->addSeparator();

	ToolBar->addWidget(new QLabel("simulation factor: "));
	LRealTimeValue = new QLineEdit();
	LRealTimeValue->setReadOnly(true);
	ToolBar->addWidget(LRealTimeValue);

	ToolBar->addSeparator();

	ToolBar->addWidget(new QLabel("run time: "));
	LRunTimeValue = new QLineEdit();
	LRunTimeValue->setReadOnly(true);
	ToolBar->addWidget(LRunTimeValue);

	ToolBar->addSeparator();

	ToolBar->addWidget(new QLabel("simulated time: "));
	LSimTimeValue = new QLineEdit();
	LSimTimeValue->setReadOnly(true);
	ToolBar->addWidget(LSimTimeValue);

	ToolBar->addSeparator();

	SimProgress = new QProgressBar();
	SimProgress->setRange(0, 100);
	SimProgress->setValue(0);
	ToolBar->addWidget(SimProgress);
}

bool snMainWindow::okToContinue() {
	if (getSimStatus() == Running) {
		QMessageBox::warning(this, "SNSignal", "Simulation is running. Unable to perform requested function.",
				QMessageBox::Ok);
		return false;
	} else {
		return true;
	}
}

void snMainWindow::closeEvent(QCloseEvent *event) {

	if (okToContinue()) {
		event->accept();
	} else {
		event->ignore();
	}

}

void snMainWindow::about() {
	std::ostringstream ss;
	ss << "<h2>SNACS " << SNACSVersion << "</h2> <p>" << WelcomeMessage
			<< "<p>Copyright &copy; 2007-2012 Frank Schubert."
					"<p>SNACS comes with ABSOLUTELY NO WARRANTY.";
	QMessageBox::about(this, tr("About SNACS"), QString(ss.str().c_str()));
}

void snMainWindow::log(const QString &s, const snModule::LogType &d) {
	/*
	 if (d == snModule::debug && DebugLevel < 1) // do not print debug msg
	 return;
	 if (d == snModule::debugv && DebugLevel < 2) // do not print debug msg
	 return;

	 QTime Dd;
	 QString Ds;
	 Ds = QString("[") + Dd.currentTime().toString("hh:mm:ss.zzz") + QString("]  ");
	 */

	Log->moveCursor(QTextCursor::End);
	Log->setTextColor(QColor("black"));
//	Log->insertPlainText(Ds);

	QString msg;
	QColor msg_color("black");
	switch (d) {
	case snModule::critical:
		msg_color = QColor("darkRed");
		msg = "error: ";
		break;
	case snModule::warning:
		msg_color = QColor("red");
		msg = "warning: ";
		break;
	case snModule::info:
		msg_color = QColor("green");
		msg = "info: ";
		break;
	case snModule::debug:
		msg_color = QColor("darkGrey");
		msg = "debug: ";
		break;
	case snModule::debugv:
		msg_color = QColor("grey");
		msg = "debug: ";
		break;
	}

	Log->setTextColor(msg_color);
	Log->insertPlainText(s + QString("\n"));
	Log->ensureCursorVisible();

	if (d == snModule::critical) {
		QMessageBox::warning(this, "Error: ", s, QMessageBox::Ok);
		// exit now:
		qApp->quit(); // why does his not work the first time it is called, from constructor?
	}
}

snMainWindow::SimStatusType snMainWindow::getSimStatus() {
	return SimStatus;
}

void snMainWindow::setSimStatus(SimStatusType t) {
	SimStatus = t;
	switch (t) {
	case NoConfig: // set Start button inactive:
		startAction->setEnabled(false);
		stopAction->setEnabled(false);
		openAction->setEnabled(true);
		exitAction->setEnabled(true);
		break;
	case Ready: //   set Start button active:
		startAction->setEnabled(true);
		stopAction->setEnabled(false);
		openAction->setEnabled(false);
		exitAction->setEnabled(true);
		break;
	case Running:
		startAction->setEnabled(false);
		stopAction->setEnabled(true);
		openAction->setEnabled(false);
		exitAction->setEnabled(false);
		break;
	case Stopped:
		startAction->setEnabled(false);
		stopAction->setEnabled(false);
		openAction->setEnabled(false);
		exitAction->setEnabled(true);
		break;
	}
}

void snMainWindow::loadFile(QString filename) {

	try {
		libconfig::Config cfg;

		log(QString("loading configuration file ") + QString(filename));
		cfg.readFile(filename.toStdString().c_str());

		// read general plotting parameters:
		libconfig::Setting &set_plt = cfg.lookup("Plotting");

		PlotUpdateInterval = set_plt["UpdateInterval"];
		log("UpdateInterval: " + QString::number(PlotUpdateInterval));

		/* TODO
		 if (Sig.PlotXSize > Sig.BufSize) {
		 log("abort.");
		 throw snSimulationException(
		 "Plotting.PlotXSize must be smaller or equal than ConcBufSize.");
		 }
		 */

		// read general Simulation parameters:
		libconfig::Setting &set = cfg.lookup("Simulation");

//		QProgressDialog progress("setting up modules...", "Abort", 0, nof_modules, this);
//		progress.setMinimumDuration(0);
//		progress.setWindowModality(Qt::WindowModal);

//		progress.setValue(i);
//		progress.setLabelText("setting up " + curBlock);
//		progress.update();
//		QCoreApplication::processEvents();
//		QCoreApplication::flush();

		qt_widget_factory widget_factory;

		snSimu = new snSimulation();

		qRegisterMetaType<snModule::LogType>("snModule::LogType"); // to send as parameter in signals.

		// connect signal set log level of this to snSimulation:
		connect(this, SIGNAL(set_log_level(const snModule::LogType &)), snSimu,
				SLOT(set_log_level(const snModule::LogType &)));

		snModules = snSimu->setup_simulation(set, &widget_factory);
		if (DebugLevel == 0)
			snSimu->set_log_level(snModule::info);
		else if (DebugLevel == 1)
			snSimu->set_log_level(snModule::debug);
		if (DebugLevel == 2)
			snSimu->set_log_level(snModule::debugv);

		snWs = widget_factory.get_created_widgets();
		//std::cout << "snModules.size(): " << snModules.size() << "\n";

		// connect log signal of snSimulation to log slot:
		connect(snSimu, SIGNAL(LogSignal(const QString &, const snModule::LogType &)), this,
				SLOT(log(const QString &, const snModule::LogType &)));

		setSimStatus(Ready);
		log("Setup complete. Click Start to start simulation.");

		return;
	} // try
	catch (libconfig::FileIOException) {
		QMessageBox::warning(this, "Error while loading configuration file", "error: file I/O error.",
				QMessageBox::Ok);
	} catch (libconfig::ParseException p) {
		QMessageBox::warning(this, "Error while loading configuration file",
				"In file " + QString(p.getFile()) + ": \nat line " + QString::number(p.getLine()) + ": \n"
						+ p.getError(), QMessageBox::Ok);
	} catch (libconfig::SettingNotFoundException e) {
		QMessageBox::warning(this, "Error while loading configuration file",
				"In file " + filename + ": \nSetting not found: \n" + QString(e.getPath()), QMessageBox::Ok);
	} catch (libconfig::SettingTypeException e) {
		QMessageBox::warning(this, "Error while loading configuration file",
				"In file " + filename
						+ ": \nType of the setting's value does not match the requested type: \n"
						+ QString(e.getPath())
						+ ".\n(Have you entered an integer value where a double was required, i.e. have you entered '1' instead of '1.0'?)",
				QMessageBox::Ok);

	} catch (libconfig::SettingNameException e) {
		QMessageBox::warning(this, "Error while loading configuration file",
				"In file " + filename
						+ ": \nAttempt is made to add a new setting with a non-unique or invalid name:\n "
						+ QString(e.getPath()), QMessageBox::Ok);
	} catch (std::exception &e) {
		log(e.what(), snModule::critical);
	}

	// if we are here, an exception was thrown
	// delete all created objects!
//	//
//	for (size_t i = 0; i < snModules.size(); i++) {
//		delete snModules.at(i);
//	}
	snModules.clear();
//	for (size_t i = 0; i < snWs.size(); i++) {
//		delete snWs.at(i);
//	}
	snWs.clear();
}

void snMainWindow::startSimu() {

	try {
		if (getSimStatus() != Ready) {
			throw snSimulationException("Simulation cannot be started.");
		}
		setSimStatus(Running);
		for (size_t i = 0; i < snWs.size(); i++) {
			// std::cout << "getting widget " << i << "\n";
//			if (snWs.at(i)->getWindowType() == snWidgetQwt::INLINE) {
			QDockWidget *dock = new QDockWidget(snWs.at(i)->getTitle().c_str());

//				int desktop_height = QApplication::desktop()->screenGeometry().height();
//				int desktop_width = QApplication::desktop()->screenGeometry().height();
//				int docks_height = floor(static_cast<double>(desktop_height) * .7);
//				int docks_width = floor(
//						static_cast<double>(desktop_width) / static_cast<double>(snWs.size()));
//				snWs.at(i)->get_widget()->setMaximumHeight(docks_height);
//				snWs.at(i)->get_widget()->resize(docks_width, docks_height);

			snWs.at(i)->get_widget()->setParent(this);

			dock->setWidget(snWs.at(i)->get_widget());

			this->addDockWidget(Qt::BottomDockWidgetArea, dock);
//			} else {
//				snWs.at(i)->get_widget()->setParent(0);
//				snWs.at(i)->get_widget()->show();
//				//snWs.at(i)->get_widget()->resize(480, 480);
//			}
		}

		//this->showMaximized();
		// std::cout << "requesting plot\n";

		reqplot();

		// std::cout << "start...\n";

		timer = new QTimer(); // QTimer calls update all widgets periodically
		QObject::connect(timer, SIGNAL(timeout()), this, SLOT(reqplot()), Qt::DirectConnection);
		timer->start(PlotUpdateInterval);

		SimTime = new QTime;
		SimTime->start();

		SimProgressBarTimer = new QTimer(); // QTimer updates progress bar periodically
		QObject::connect(SimProgressBarTimer, SIGNAL(timeout()), this, SLOT(updateSimProgressBar()));
		SimProgressBarTimer->start(1000);

		log("Starting  threads...");
		for (size_t i = 0; i < snModules.size(); i++) {
			snModules.at(i)->start();
		}
		log("done...");

		// when output blocks exits: stop progress bar
		connect(snModules.back().get(), SIGNAL(finished()), this, SLOT(stopSimu()));

	} catch (snSimulationException e) {
		QMessageBox::warning(this, "Error: ", e.what(), QMessageBox::Ok);
		log(QString("Error: ") + QString(e.what()));
	} catch (std::exception &e) {
		QMessageBox::warning(this, "Error: ", e.what(), QMessageBox::Ok);
		log(QString("Error: ") + QString(e.what()));
	}

}

void snMainWindow::stopSimu() {
	log("waiting for threads to finish...");

	// tell the signal source (snBlocks->at(0)) to quit, signal will end and all other threads will quit then:
	snModules.at(0)->requestStop();
	for (size_t i = 0; i < snModules.size(); i++) {
		snModules.at(i)->wait();
	}

	setSimStatus(Stopped);
	SimProgressBarTimer->stop();

	//  close results file:
	//  TODO commented out because it lets the program crush when pressing Abort button.
	//	log("closing results file...");
	//	delete Sig.results_file;

	log("All threads finished. End of simulation.");

	if (quit_when_finished) {
		exitProg();
	}
}

void snMainWindow::reqplot() {
	for (size_t i = 0; i < snWs.size(); i++) {
		snWs.at(i)->replotWidgets();
	}
}

void snMainWindow::updateSimProgressBar() {
	// plot program run time
	const double vRunTime = SimTime->elapsed() / 1000.0;
	LRunTimeValue->setText(QString::number(vRunTime) + " s");

	// plot simulated time
	const double vSimTime = snModules.back()->get_current_simulation_time();
	LSimTimeValue->setText(QString::number(vSimTime) + " s");

	// ratio is speed compared to real time:
	LRealTimeValue->setText(QString::number(vSimTime / vRunTime));

	const double vTotalTime = snModules.back()->get_total_simulation_time();
	SimProgress->setValue(static_cast<int>((vSimTime / vTotalTime) * 100.0));
}

void snMainWindow::setDebugLevel(int d) {
	if (d < 0) {
		log("Debug level must be greater than or equal 0.", snModule::critical);
	} else if (d > 2) {
		log("Debug level must be smaller than or equal 2.", snModule::critical);
	}
	DebugSpin->setValue(d);
	DebugLevel = d;
	if (d == 0) {
		emit set_log_level(snModule::info);
	} else if (d == 1) {
		emit set_log_level(snModule::debug);
	} else if (d == 2) {
		emit set_log_level(snModule::debugv);
	}
}
