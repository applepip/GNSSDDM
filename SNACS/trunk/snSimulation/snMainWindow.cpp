/*
 *  SNACS - The Satellite Navigation Radio Channel Simulator
 *
 *  Copyright (C) 2009  F. M. Schubert
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

snMainWindow::snMainWindow(bool pquit_when_finished) :
	quit_when_finished(pquit_when_finished) {

	DebugLevel = 0;

	snWs = new std::vector<snWidget *>; // vector that contains pointers to all widgets
	ConcBufs = new std::vector<ConcBuf *>; // vector with pointers to all ConcBuffers
	snBlocks = new std::vector<snBlock *>; // vector of base class pointers for all snBlocks

	Sig.aBuf = 0; // will be set from config file
	Sig.BytesPerSample = 8;

	SigProc.ProcessingDelay = 0;

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

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	setSimStatus(NoConfig);
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
		QString filename = QFileDialog::getOpenFileName(this, tr(
				"Open Simulation Configuration File"), ".", tr(
				"SNSignal configuration files (*.cfg)"));
		if (!filename.isEmpty()) {
			loadFile(filename);
			return true;
		}
	}
	return false;
}

void snMainWindow::exitProg() {
	if (getSimStatus() == Stopped) { // close Simulation window

	}
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
	DebugSpin->setRange(0, 9);
	ToolBar->addWidget(new QLabel("Debug level: "));
	ToolBar->addWidget(DebugSpin);

	connect(DebugSpin, SIGNAL(valueChanged(int)), this, SLOT(setDebugLevel(int)));

	ToolBar->addSeparator();

	ToolBar->addWidget(new QLabel("real time speed: "));
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
		QMessageBox::warning(this, "SNSignal",
				"Simulation is running. Unable to perform requested function.", QMessageBox::Ok);
		return false;
	} else {
		return true;
	}

	/*
	 int r = QMessageBox::warning(this, "SNSignal", "Do you want to quit?", QMessageBox::Yes
	 | QMessageBox::Cancel);
	 if (r == QMessageBox::Yes) {
	 return true;
	 } else if (r == QMessageBox::Cancel) {
	 return false;
	 }
	 */
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
			<< "<p>Copyright &copy; 2007-2009 Frank Schubert."
				"<p>SNACS comes with ABSOLUTELY NO WARRANTY.";
	QMessageBox::about(this, tr("About SNACS"), QString(ss.str().c_str()));
}

void snMainWindow::logDeb(const QString &s, int d) {
	if (DebugLevel >= d) {
		QTime Dd;
		QString Ds;
		Ds = QString("[") + Dd.currentTime().toString("hh:mm:ss.zzz") + QString("]  ");

		QTextStream logfile(&logfile_handle);

		//		logfile << Ds.toStdString() << s.toStdString() << "\n";
		logfile << Ds << s << "\n";
		logfile.flush();
		Log->moveCursor(QTextCursor::End);
		Log->setTextColor(QColor("black"));
		Log->insertPlainText(Ds);
		Log->setTextColor(QColor("green"));
		Log->insertPlainText(s + QString("\n"));
		Log->ensureCursorVisible();
	}
}

void snMainWindow::log(const QString &s) {
	QTime Dd;
	QString Ds;
	Ds = QString("[") + Dd.currentTime().toString("hh:mm:ss.zzz") + QString("]  ");

	QTextStream logfile(&logfile_handle);

	//	logfile << Ds.toStdString() << s.toStdString() << "\n";
	logfile << Ds << s << "\n";
	logfile.flush();

	Log->moveCursor(QTextCursor::End);
	Log->setTextColor(QColor("black"));
	Log->insertPlainText(Ds);
	Log->setTextColor(QColor("blue"));
	Log->insertPlainText(s + QString("\n"));
	Log->ensureCursorVisible();
}

void snMainWindow::logErr(const QString &s) {
	QTime Dd;
	QString Ds;
	Ds = QString("[") + Dd.currentTime().toString("hh:mm:ss.zzz") + QString("]  ");

	QTextStream logfile(&logfile_handle);

	//	logfile << Ds.toStdString() << s.toStdString() << "\n";
	logfile << Ds << s << "\n";
	logfile.flush();

	Log->moveCursor(QTextCursor::End);
	Log->setTextColor(QColor("black"));
	Log->insertPlainText(Ds);
	Log->setTextColor(QColor("red"));
	Log->insertPlainText(s + QString("\n"));
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

		std::cout << "loading configuration file " << filename.toStdString() << "...\n";
		cfg.readFile(filename.toStdString().c_str());

		// check if output directory exists:
		std::string output_directory_str = static_cast<const char *> (cfg.lookup(
				"Simulation.result_directory"));

		QString out_dir_str = output_directory_str.c_str();
		if (static_cast<bool> (cfg.lookup("Simulation.append_date_to_outdir")))
			out_dir_str += QDateTime::currentDateTime().toString("_yyyy-MM-dd_HH-mm-ss");

		QDir out_dir;

		// TODO: remove trailing slash or backslash here

		if (!out_dir.exists(out_dir_str)) {
			log("result_directory " + out_dir_str + " does not exist. creating it now.");
			out_dir.mkpath(out_dir_str);
		} else {
			throw snSimulationException("Error: Result directory '" + out_dir_str.toStdString() + "' exists already. Try adding parameter 'Simulation.append_date_to_outdir = true;' This will create a new output directory for every simulation run.");
		}

#ifdef __unix__
		QString slash = "/";
#else
		QString slash = "/";
#endif
		std::cout << "open file "
				<< QString(out_dir_str + slash + "SNACS-log.txt").toStdString().c_str();
		//		logfile.open(QString(out_dir_str + slash + "SNACS-log.txt").toStdString().c_str(), std::ios::out);
		//		logfile << "lala\n";
		//		if (!logfile.good()) {
		//			throw snSimulationException("snMainWindow: cannot open log file. exit.");
		//		}

		logfile_handle.setFileName(out_dir_str + slash + "SNACS-log.txt");
		if (!logfile_handle.open(QIODevice::WriteOnly | QIODevice::Text)) {
			throw snSimulationException("snMainWindow: cannot open log file. exit.");
		}

		log(QString("loading configuration file ") + filename);

		// read general Simulation parameters:
		// read Sampling Frequency
		double val = cfg.lookup("Simulation.sampling_frequency");
		if (val < 0) {
			logErr("abort.");
			throw snSimulationException("Simulation.SamplingFrequency must be greater than 0.");
		}
		Sig.SmplFreq = static_cast<Freq_Type> (val);
		log("Sampling Frequency: " + QString::number(Sig.SmplFreq) + " Hz");

		// read c0
		val = cfg.lookup("Simulation.c0");
		if (val < 0) {
			logErr("abort.");
			throw snSimulationException("Simulation.c0 must be greater than 0.");
		}
		Sig.c0 = val;
		log("c0: " + QString::number(Sig.c0) + " m/s");

		// read Signal length
		val = cfg.lookup("Simulation.length");
		if (val < 0) {
			logErr("abort.");
			throw snSimulationException("Simulation.length must be greater than 0.");
		}
		Sig.Len = val;
		log("Signal length: " + QString::number(Sig.Len) + " s");

		// calculate total amount of samples
		Sig.Ts = 1.0 / Sig.SmplFreq;
		if (Sig.Len * Sig.SmplFreq > ULLONG_MAX) { // Sig.aSamples is ULLONG
			logErr("abort.");
			throw snSimulationException(QString(
					"Variable overflow: Total amount of samples must be smaller than"
							+ QString::number(ULLONG_MAX) + " samples").toStdString());
		} else if (!isInt(Sig.Len * Sig.SmplFreq)) {
			logErr("abort.");
			throw snSimulationException("(Sig.Len * Sig.SmplFreq) must be integer.");
		}
		Sig.aSamples = static_cast<Samples_Type> (Sig.Len * Sig.SmplFreq);

		log("info: total amount of samples: " + QString::number(Sig.aSamples));

		// amount of buffers per concbuf:
		Sig.aBuf = cfg.lookup("Simulation.buffers_per_concbuf");
		if (Sig.aBuf < 2) {
			logErr("abort.");
			throw snSimulationException("Simulation.buffers_per_concbuf must be >= 2!");
		}

		// read size of concurrent buffers:
		val = cfg.lookup("Simulation.conc_buf_length");
		if (val < 0) {
			logErr("abort.");
			throw snSimulationException("Simulation.conc_buf_length must be greater than 0.");
		}
		if (val * Sig.SmplFreq > INT_MAX) {
			logErr("abort.");
			throw snSimulationException(QString(
					"int overflow: conc_buf_length must be smaller than " + QString::number(INT_MAX
							/ Sig.SmplFreq) + " s").toStdString());
		}
		if (!isInt(val * Sig.SmplFreq)) {
			logErr("abort.");
			throw snSimulationException("(ConcBufSize * sampling frequency) must be integer.");
		}
		Sig.BufSize = static_cast<int> (val * Sig.SmplFreq);

		log("Size of concurrent buffers: " + QString::number(val) + "s = " + QString::number(
				Sig.BufSize) + " samples");

		// open results file
		Sig.results_file = new snWriteResultFile(QString(out_dir_str + "/result.h5").toStdString());

		// calculate total amount of buffers:
		if (Sig.Len * Sig.SmplFreq / Sig.BufSize > ULONG_MAX) { // Sig.aBlocks is ULONG
			logErr("abort.");
			throw snSimulationException(QString(
					"error: variable overflow: total amount of buffer blocks must be smaller than "
							+ QString::number(ULONG_MAX)).toStdString());
		} else if (!isInt(val * Sig.SmplFreq)) {
			logErr("abort.");
			throw snSimulationException(
					"(Signal Length * Sampling Frequency / ConcBufSize) must be integer.");
		} else {
			Sig.aBlock = static_cast<unsigned long> (Sig.Len * Sig.SmplFreq / Sig.BufSize);
		}
		log("info: total amount of buffer blocks: " + QString::number(Sig.aBlock));

		PlotUpdateInterval = cfg.lookup("Plotting.UpdateInterval");
		log("info: UpdateInterval: " + QString::number(PlotUpdateInterval));

		Sig.PlotXSize = cfg.lookup("Plotting.PlotXSize");
		if (Sig.PlotXSize > Sig.BufSize) {
			logErr("abort.");
			throw snSimulationException(
					"Plotting.PlotXSize must be smaller or equal than ConcBufSize.");
		}

		Sig.plot_buffer_length = cfg.lookup("Plotting.plot_buffer_length"); // in s
		if (Sig.plot_buffer_length > static_cast<double> (Sig.BufSize) / Sig.SmplFreq) {
			logErr("abort.");
			throw snSimulationException(
					"Plotting.plot_buffer_length must be smaller or equal than the length of a concbuf.");
		}

		// setup SNBlocks:
		int aSNBlocks = cfg.lookup("Simulation.SNBlocks").getLength();
		log(QString("info: setting up " + QString::number(aSNBlocks) + " SNBlocks"));

		QProgressDialog progress("setting up snBlocks...", "Abort", 0, aSNBlocks, this);
		progress.setMinimumDuration(0);
		progress.setWindowModality(Qt::WindowModal);

		for (int i = 0; i < aSNBlocks; i++) {

			QString BlockPath("Simulation.SNBlocks.[" + QString::number(i) + "]");
			QString curBlock(cfg.lookup(QString(BlockPath + ".Type").toStdString()));

			progress.setValue(i);
			progress.setLabelText("setting up " + curBlock);
			progress.update();
			QCoreApplication::processEvents();
			QCoreApplication::flush();

			if (curBlock == "snSignalGenerate") {
				ConcBufs->push_back(new ConcBuf(Sig.aBuf, Sig.BufSize, "GenSig"));

				snWs->push_back(new snWidget(0, "Signal Generator", snWidget::INLINE));

				snBlocks->push_back(new snSignalGenerate(cfg.lookup(
						QString(BlockPath).toStdString()), Sig, snWs->back(), ConcBufs->back()));

				connect(snBlocks->back(), SIGNAL(snLogSignal(const QString &)), this,
						SLOT(log(const QString &)));
				connect(snBlocks->back(), SIGNAL(snLogSignalDebug(const QString &, int)), this,
						SLOT(logDeb(const QString &, int)));

				log("setup of snBlock SignalGenerate complete.");
			} else if (curBlock == "snSignalInputBinary") {
				ConcBufs->push_back(new ConcBuf(Sig.aBuf, Sig.BufSize, "Signal Input Binary"));

				snWs->push_back(new snWidget(this, "Signal Input Binary", snWidget::INLINE));

				snBlocks->push_back(new snSignalInputBinary(cfg.lookup(
						QString(BlockPath).toStdString()), Sig, snWs->back(), ConcBufs->back()));

				connect(snBlocks->back(), SIGNAL(snLogSignal(const QString &)), this,
						SLOT(log(const QString &)));
				connect(snBlocks->back(), SIGNAL(snLogSignalDebug(const QString &, int)), this,
						SLOT(logDeb(const QString &, int)));

			} else if (curBlock == "snSignalOutputBinary") {

				snWs->push_back(new snWidget(this, "Signal Output Binary", snWidget::INLINE));
				snBlocks->push_back(new snSignalOutputBinary(cfg.lookup(
						QString(BlockPath).toStdString()), Sig, snWs->back(), ConcBufs->back()));
				connect(snBlocks->back(), SIGNAL(snLogSignal(const QString &)), this,
						SLOT(log(const QString &)));
				connect(snBlocks->back(), SIGNAL(snLogSignalDebug(const QString &, int)), this,
						SLOT(logDeb(const QString &, int)));

				log("setup of snBlock SignalOutputBinary complete.");

			} else if (curBlock == "snSignalOutputWidget") {

				snWs->push_back(new snWidget(0, "Signal Output", snWidget::INLINE));
				snBlocks->push_back(new snSignalOutputWidget(cfg.lookup(
						QString(BlockPath).toStdString()), Sig, snWs->back(), ConcBufs->back()));
				connect(snBlocks->back(), SIGNAL(snLogSignal(const QString &)), this,
						SLOT(log(const QString &)));
				connect(snBlocks->back(), SIGNAL(snLogSignalDebug(const QString &, int)), this,
						SLOT(logDeb(const QString &, int)));

				log("setup of snBlock SignalOutputWidget complete.");

			} else if (curBlock == "snADC") {
				ConcBufs->push_back(new ConcBuf(Sig.aBuf, Sig.BufSize, "ADC"));

				snWs->push_back(new snWidget(this, "ADC", snWidget::INLINE));

				snBlocks->push_back(new snProcessorADC(
						cfg.lookup(QString(BlockPath).toStdString()), Sig, snWs->back(),
						ConcBufs->at(ConcBufs->size() - 2), ConcBufs->back()));

				connect(snBlocks->back(), SIGNAL(snLogSignal(const QString &)), this,
						SLOT(log(const QString &)));
				connect(snBlocks->back(), SIGNAL(snLogSignalDebug(const QString &, int)), this,
						SLOT(logDeb(const QString &, int)));

				log("setup of snBlock snADC complete.");

			} else if (curBlock == "snNoise") {
				ConcBufs->push_back(new ConcBuf(Sig.aBuf, Sig.BufSize, "AWG Noise"));

				snWs->push_back(new snWidget(this, "Noise", snWidget::INLINE));

				snBlocks->push_back(new snProcessorNoise(cfg.lookup(
						QString(BlockPath).toStdString()), Sig, snWs->back(), ConcBufs->at(
						ConcBufs->size() - 2), ConcBufs->back()));

				connect(snBlocks->back(), SIGNAL(snLogSignal(const QString &)), this,
						SLOT(log(const QString &)));
				connect(snBlocks->back(), SIGNAL(snLogSignalDebug(const QString &, int)), this,
						SLOT(logDeb(const QString &, int)));

				log("setup of snBlock snNoise complete.");

			} else if (curBlock == "snLowPassFilter") {
				ConcBufs->push_back(new ConcBuf(Sig.aBuf, Sig.BufSize, "Low Pass Filter"));

				snWs->push_back(new snWidget(this, "Low Pass Filter", snWidget::INLINE));

				snBlocks->push_back(new snProcessorLPF(
						cfg.lookup(QString(BlockPath).toStdString()), Sig, SigProc, snWs->back(),
						ConcBufs->at(ConcBufs->size() - 2), ConcBufs->back()));

				connect(snBlocks->back(), SIGNAL(snLogSignal(const QString &)), this,
						SLOT(log(const QString &)));
				connect(snBlocks->back(), SIGNAL(snLogSignalDebug(const QString &, int)), this,
						SLOT(logDeb(const QString &, int)));

				log("setup of snBlock snLowPassFilter complete.");

			} else if (curBlock == "snChannel") {
				ConcBufs->push_back(new ConcBuf(Sig.aBuf, Sig.BufSize, "Channel"));

				snWs->push_back(new snWidget(this, "channel impulse response", snWidget::INLINE));

				snWs->push_back(new snWidget(this, "FIR coefficients", snWidget::INLINE));
				snWs->push_back(new snWidget(this, "convoluted signal", snWidget::INLINE));

				snBlocks->push_back(new snProcessorChannel(cfg.lookup(
						QString(BlockPath).toStdString()), Sig, SigProc,
						snWs->at(snWs->size() - 3), snWs->at(snWs->size() - 2), snWs->back(),
						ConcBufs->at(ConcBufs->size() - 2), ConcBufs->back()));

				connect(snBlocks->back(), SIGNAL(snLogSignal(const QString &)), this,
						SLOT(log(const QString &)));
				connect(snBlocks->back(), SIGNAL(snLogSignalDebug(const QString &, int)), this,
						SLOT(logDeb(const QString &, int)));

				log("setup of snBlock snChannel complete.");

			} else if (curBlock == "snSDRStandard") {

				snWs->push_back(new snWidget(0, "SDR1", snWidget::SEPARATE));

				snWs->push_back(new snWidget(this, "SDR2", snWidget::INLINE));
				snWs->push_back(new snWidget(this, "SDR3", snWidget::INLINE));
				snWs->push_back(new snWidget(this, "SDR4", snWidget::INLINE));
				snWs->push_back(new snWidget(this, "SDR5", snWidget::INLINE));

				size_t snWsSize = snWs->size();

				snBlocks->push_back(new snSDRStandard(cfg.lookup(QString(BlockPath).toStdString()), Sig,
						SigProc, snWs->at(snWsSize - 5), snWs->at(snWsSize - 4), snWs->at(snWsSize
								- 3), snWs->at(snWsSize - 2), snWs->at(snWsSize - 1),
						ConcBufs->back()));

				connect(snBlocks->back(), SIGNAL(snLogSignal(const QString &)), this,
						SLOT(log(const QString &)));
				connect(snBlocks->back(), SIGNAL(snLogSignalDebug(const QString &, int)), this,
						SLOT(logDeb(const QString &, int)));

				log("setup of snBlock snSDRStandard complete.");
			} else {
				logErr("abort.");
				throw snSimulationException(
						QString("Unknown SNBlock type " + curBlock).toStdString());
			}
		}

		// save a copy of the configuration file to the output directory
		cfg.writeFile(QString(out_dir_str + "/config.cfg").toStdString().c_str());

		setSimStatus(Ready);
		log("Setup complete. Click Start to start simulation.");
		return;
	} // try
	catch (libconfig::FileIOException) {
		QMessageBox::warning(this, "Error while loading configuration file",
				"error: file I/O error.", QMessageBox::Ok);
	} catch (libconfig::ParseException p) {
		QMessageBox::warning(this, "Error while loading configuration file", "In file " + filename
				+ ": \nat line " + QString::number(p.getLine()) + ": \n" + p.getError(),
				QMessageBox::Ok);
	} catch (libconfig::SettingNotFoundException e) {
		QMessageBox::warning(this, "Error while loading configuration file", "In file " + filename
				+ ": \nSetting not found: \n" + QString(e.getPath()), QMessageBox::Ok);
	} catch (libconfig::SettingTypeException e) {
		QMessageBox::warning(
				this,
				"Error while loading configuration file",
				"In file " + filename
						+ ": \nType of the setting's value does not match the requested type: \n"
						+ QString(e.getPath())
						+ ". (Always enter integer values as doubles with a .0: for example 1.0 instead of 1)",
				QMessageBox::Ok);

	} catch (libconfig::SettingNameException e) {
		QMessageBox::warning(this, "Error while loading configuration file", "In file " + filename
				+ ": \nAttempt is made to add a new setting with a non-unique or invalid name:\n "
				+ QString(e.getPath()), QMessageBox::Ok);
	} catch (snSimulationException e) {
		logErr(QString(e.what().c_str()));
		QMessageBox::warning(this, "Error while loading configuration file", e.what().c_str(),
				QMessageBox::Ok);
	} catch (std::exception &e) {
		QMessageBox::warning(this, "Error: ", e.what(), QMessageBox::Ok);
	} catch (...) {
		QMessageBox::warning(this, "Error while loading configuration file", "Unknown reason.",
				QMessageBox::Ok);
	}

	// if we are here, an exception was thrown
	// delete all created objects!
	//
	for (size_t i = 0; i < snBlocks->size(); i++) {
		delete snBlocks->at(i);
	}
	snBlocks->clear();
	for (size_t i = 0; i < snWs->size(); i++) {
		delete snWs->at(i);
	}
	snWs->clear();
	for (size_t i = 0; i < ConcBufs->size(); i++) {
		std::cout << "deleteing " << i << "\n";
		std::cout.flush();
		delete ConcBufs->at(i);
	}
	ConcBufs->clear();

}

void snMainWindow::startSimu() {

	try {
		if (getSimStatus() != Ready) {
			throw snSimulationException("Simulation cannot be started.");
		}
		setSimStatus(Running);

		for (size_t i = 0; i < snWs->size(); i++) {
			if (snWs->at(i)->getWindowType() == snWidget::INLINE) {
				QDockWidget *dock = new QDockWidget(snWs->at(i)->getTitle());

				//				snWs->at(i)->resize(80, 400);
				int desktop_height = QApplication::desktop()->screenGeometry().height();
				int desktop_width = QApplication::desktop()->screenGeometry().height();
				int docks_height = static_cast<int> (floor(desktop_height * .7));
				int docks_width = static_cast<int> (floor(desktop_width / snWs->size()));
				snWs->at(i)->setMaximumHeight(docks_height);
				//				snWs->at(i)->setMaximumWidth(docks_width);
				snWs->at(i)->resize(docks_width, docks_height);
				dock->setWidget(snWs->at(i));

				this->addDockWidget(Qt::BottomDockWidgetArea, dock);
				//snWs->at(i)->setParent(SimulationWindow);
			} else {
				snWs->at(i)->setParent(0);
				snWs->at(i)->show();
				snWs->at(i)->resize(480, 480);
			}
		}

		//this->showMaximized();

		reqplot();

		timer = new QTimer(); // QTimer calls update all widgets periodically
		QObject::connect(timer, SIGNAL(timeout()), this, SLOT(reqplot()), Qt::DirectConnection);
		timer->start(PlotUpdateInterval);

		SimTime = new QTime;
		SimTime->start();

		SimProgressBarTimer = new QTimer(); // QTimer updates progress bar periodically
		QObject::connect(SimProgressBarTimer, SIGNAL(timeout()), this, SLOT(
				updateSimProgressBar()));
		SimProgressBarTimer->start(300);

		log("Starting  threads.");
		for (size_t i = 0; i < snBlocks->size(); i++) {
			snBlocks->at(i)->start();
		}

		// when output blocks exits: stop progress bar
		connect(snBlocks->back(), SIGNAL(finished()), this, SLOT(stopSimu()));

	} catch (snSimulationException e) {
		QMessageBox::warning(this, "Error: ", e.what().c_str(), QMessageBox::Ok);
		logErr(QString("Error: ") + QString(e.what().c_str()));
	} catch (std::exception &e) {
		QMessageBox::warning(this, "Error: ", e.what(), QMessageBox::Ok);
		logErr(QString("Error: ") + QString(e.what()));
	}

}

void snMainWindow::stopSimu() {
	log("waiting for threads to finish...");

	// tell the signal source (snBlocks->at(0)) to quit, signal will end and all other threads will quit then:
	snBlocks->at(0)->requestStop();
	for (size_t i = 0; i < snBlocks->size(); i++) {
		snBlocks->at(i)->wait();
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
	for (size_t i = 0; i < snWs->size(); i++) {
		snWs->at(i)->replotWidgets();
	}
}

void snMainWindow::updateSimProgressBar() {
	// plot program run time
	double vRunTime = SimTime->elapsed() / 1000.0;
	LRunTimeValue->setText(QString::number(vRunTime) + " s");

	// plot simulated time
	Blocks_Type processedBlks = snBlocks->back()->getOutputBlks() + 1;
	// getOutputBlks()+1 because Blks() start at 0

	double vSimTime = static_cast<double> (processedBlks) * static_cast<double> (Sig.BufSize)
			* Sig.Ts;
	LSimTimeValue->setText(QString::number(vSimTime) + " s");

	// ratio is speed compared to real time:
	LRealTimeValue->setText(QString::number(vSimTime / vRunTime));

	SimProgress->setValue(static_cast<int> ((vSimTime / Sig.Len) * 100.0));
}

void snMainWindow::setDebugLevel(int d) {
	if (d < 0) {
		logErr("Debug level must be greater than 0.");
	} else if (d > 9) {
		logErr("Debug level must be smaller than 9.");
	}
	DebugSpin->setValue(d);
	DebugLevel = d;
}
