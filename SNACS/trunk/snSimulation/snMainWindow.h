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

#include <QtGui>
#include <libconfig.h++>
#include "snSimulation.h"

#include "snBlock.h"

// source snBlocks
#include "../snSourceBlocks/snSignalGenerate.h"
#include "../snSourceBlocks/snSignalInputBinary.h"

// processor snBlocks
#include "../snProcessorBlocks/snProcessorADC.h"
#include "../snProcessorBlocks/snProcessorLPF.h"
#include "../snProcessorBlocks/snProcessorChannel.h"
#include "../snProcessorBlocks/snProcessorNoise.h"

// sink snBlocks
#include "../snSinkBlocks/snSDRStandard.h"
#include "../snSinkBlocks/snSignalOutputBinary.h"
#include "../snSinkBlocks/snSignalOutputWidget.h"

#include "ConcurrentBuffer.h"

/*!
 * \brief	Class for the main window widget
 */
class snMainWindow: public QMainWindow {
	Q_OBJECT

public:
	snMainWindow(bool pquit_when_finished);
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
	 * \brief loggs a message with a certain debugging level
	 *
	 */
	void logDeb(const QString &, int d);

	/**
	 * \brief loggs a message
	 *
	 */
	void log(const QString &);

	/**
	 * \brief loggs an error
	 *
	 */
	void logErr(const QString &);

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
	void setDebugLevel(int);

private:
	void createActions();
	void createToolBars();
	void createMenus();
	void createStatusBar();
	bool okToContinue();

	bool quit_when_finished; ///< if parameter -s is given on command line, SNACS will exit after simulation is done.
	
	QFile logfile_handle;

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

	SNSignal Sig; ///< parameters that are common to every snBlock
	ProcessingParameters SigProc; ///< parameters that can be changed by every snBlock

	unsigned int PlotUpdateInterval;
	QTimer *timer;

	std::vector<snWidget *> *snWs; ///< contains pointers to all widgets
	std::vector<ConcBuf *> *ConcBufs; ///< pointers to all ConcBuffers
	std::vector<snBlock *> *snBlocks; ///< vector of base class pointers for all snBlocks

private slots:
	bool openFile();
	void stopSimu();
	void exitProg();
	void about();

protected:
	void closeEvent(QCloseEvent *event);
};

#endif /* SNMAINWINDOW_H_ */
