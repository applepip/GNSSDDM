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

/**
 * \file snBlock.h
 *
 * \author Frank Schubert
 */

#ifndef SNMODULE_H
#define SNMODULE_H

#include <iostream>
#include <fstream>
#include <vector>

#include <QThread>
#include <QReadWriteLock>
#include <QMetaType> // for Q_DECLARE_METATYPE to send enum LogType over signals

#include "../snSimulation/ConcurrentBuffer3.h"

#include "../snSimulation/snGlobal.h"

/**
 * \brief Base class for all snBlocks in SNACS
 *
 * This class serves as base for all processing blocks.
 * All blocks that work in the simulation chain are called snBlocks. snBlock is derived from QThread.
 * The processing blocks simply inherit from snBlock and implement the run() member function.
 * In the main program (i.e. snMainWindow), a base class pointer of type std::vector<snBlock *> is
 * used to access start() and stop() of all snBlocks.
 *
 * snBlocks process the data block-wise. If an application needs to read one samples after another,
 * snBlockSerial can be used.
 */
class snModule: public QThread {
Q_OBJECT

public:
	/**
	 * logging categories
	 */
	enum LogType {
		critical = 1, warning = 2, info = 3, debug = 4, debugv = 5
	};

	/**
	 * \brief snBlock constructor
	 */
	snModule(SNSignal pSNSignal);

	/**
	 * \brief returns snBlock status. will return true after requestStop() was called, e.g. by clicking on abort.
	 * This active flag is only used for signal sources.
	 *
	 */
	bool isActive();

	/**
	 * \brief requests to stop processing and return the snBlock main program in run()
	 *
	 */
	void requestStop();

	virtual ConcBuf3* get_conc_buf() = 0;
	virtual double get_current_simulation_time() = 0;
	virtual double get_total_simulation_time() = 0;

	bool do_write_txt_signals(Blocks_Type blk_num);

private:
	QReadWriteLock mfActive;
	bool fActive;

signals:
	/**
	 * \brief is connected in snMainWindow to the log text widget.
	 *
	 */
	void LogSignal(const QString &mess, const snModule::LogType &d);

protected:
	/**
	 * \brief can be used by snBlocks to log data to log text widget.
	 *
	 */
	void snLog(QString mess, LogType d = info);

	/**
	 * \brief This member function prints a summary of how much data was processed after the calling snBlock's exit.
	 */
	void printSummary(std::string caller);

	SNSignal Sig; ///< holds the simulation parameters which are common to all snBlocks
};

Q_DECLARE_METATYPE(snModule::LogType);

class txt_signal {
public:
	txt_signal(SNSignal pSNSignal, std::string sig_name, Blocks_Type blk_num);
	~txt_signal();

	void write_txt_vectors(itpp::vec &a, itpp::vec &b);

private:
	SNSignal Sig;
	std::ofstream f;
};

#endif
