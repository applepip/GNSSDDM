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
 * \file	snMainWindow.h
 * \brief	Declarations for main window.
 *
 * \author	Frank Schubert
 */

#ifndef SNMAINWINDOW_H_
#define SNMAINWINDOW_H_

#include <vector>
#include <sstream>
#include <tr1/memory>

#include <QtGui>
#include <libconfig.h++>

#include "../snSimulation/snGlobal.h"
#include "../snSimulation/snSimulation.h"
#include "../snModules/snModule.h"
#include "snWidgetQwt.h"

/*!
 * \brief	Class for the main window widget
 */
class snMainWindow: public QMainWindow {
	Q_OBJECT

public:
	snMainWindow(int &, char **);
	virtual ~snMainWindow();

	/*!
	 * \brief	Load configuration file
	 *
	 * \param	<QString filename> filename
	 */
	void loadFile(QString filename);

	/*!
	 * \brief	Defines the status of the current simulation.
	 * After configuration file was loaded: NoConfig -> Ready
	 * After start button was pressed: Ready -> Running
	 * After stop button was pressed or simulation finished: Running -> Stopped
	 */
	enum SimStatusType {
		NoConfig, Ready, Running, Stopped
	};

	/**
	 * \brief Returns the current simulation status
	 *
	 */
	SimStatusType getSimStatus();

public slots:

	/**
	 * \brief loggs a message
	 *
	 */
	void log(const QString &, const snModule::LogType &d = snModule::info);

	/**
	 * \brief is periodically called by a timer to update the display
	 *
	 */
	void reqplot();

	/**
	 * \brief updates the simulation progress bar in the main window
	 *
	 */
	void updateSimProgressBar();

	/**
	 * \brief loggs start the simulation
	 *
	 */
	void startSimu();

	/**
	 * \brief set debugging level. Messages with a debugging level smaller or equal to this value will be shown
	 *
	 */
	void setDebugLevel(int d);

signals:
		void set_log_level(snModule::LogType t);

private:
	void createActions();
	void createToolBars();
	void createMenus();
	void createStatusBar();
	bool okToContinue();

	bool quit_when_finished; ///< if parameter -s is given on command line, SNACS will exit after simulation is done.

	QAction *openAction, *startAction, *stopAction, *exitAction, *aboutAction;
	QMenu *fileMenu, *helpMenu;
	QToolBar *ToolBar, *ConfigBar;
	QTextEdit *Log;
	QLabel *StatusLabel;
	QSpinBox *DebugSpin;
	int DebugLevel;

	QTime *SimTime;
	QProgressBar *SimProgress;
	QTimer *SimProgressBarTimer;
	QLineEdit *LRealTimeValue;
	QLineEdit *LRunTimeValue;
	QLineEdit *LSimTimeValue;

	SimStatusType SimStatus;
	void setSimStatus(SimStatusType t);

//	SNSignal Sig; ///< parameters that are common to every snBlock
//	ProcessingParameters SigProc; ///< parameters that can be changed by every snBlock

	unsigned int PlotUpdateInterval;
	QTimer *timer;

	snWidget *get_new_qt_widget(std::string name);

	std::vector<snWidgetQwtPtr> snWs; ///< contains pointers to all widgets
	std::vector<snModulePtr> snModules; ///< vector of base class pointers for all snBlocks

private slots:
	bool openFile();
	void stopSimu();
	void exitProg();
	void about();

protected:
	void closeEvent(QCloseEvent *event);

	snSimulation *snSimu;
};

#endif /* SNMAINWINDOW_H_ */
