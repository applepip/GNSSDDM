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

#ifndef SNBLOCK_H
#define SNBLOCK_H

#include <iostream>
#include <vector>

#include <QThread>
#include <QReadWriteLock>

#include "ConcurrentBuffer.h"

#include "snSimulation.h"

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
class snBlock: public QThread {
Q_OBJECT

public:

	/**
	 * \brief snBlock constructor
	 */
	snBlock(SNSignal pSNSignal);

	/**
	 * \brief returns current block number which is being processed
	 */
	Blocks_Type getBlks();

	/**
	 * \brief returns processed block number of a signal sink
	 *
	 */
	Blocks_Type getOutputBlks();

	/**
	 * \brief returns total amount of blocks to be processed
	 *
	 */
	Blocks_Type getTotalBlks();

	double get_time_per_block() {
		return Sig.BufSize / Sig.SmplFreq;
	}

	/**
	 * \brief returns current simulation time dependent on current block number
	 *
	 */
	double get_blks_simulation_time();

	/**
	 * \brief returns snBlock status. will return true after requestStop() was called.
	 *
	 */
	bool isActive();

	/**
	 * \brief requests to stop processing and return the snBlock main program in run()
	 *
	 */
	void requestStop();

private:
	Blocks_Type cBlks; ///< saves the amount of processed blocks. unprotected.

	Blocks_Type cOutputBlks; ///< saves the amount of processed blocks for sink snBlocks. protected with a mutex.
	Blocks_Type total_blocks; ///< amount of blocks to be processed in total
	QReadWriteLock mBlk;
	QReadWriteLock mfActive;
	bool fActive;

signals:
	/**
	 * \brief is connected in snMainWindow to the log text widget.
	 *
	 */
	void snLogSignal(const QString &mess);

	/**
	 * \brief is connected in snMainWindow to the log text widget.
	 *
	 */
	void snLogSignalDebug(const QString &mess, int d);

protected:
	/**
	 * \brief can be used by snBlocks to log data to log text widget.
	 *
	 */
	void snLog(QString mess);

	/**
	 * \brief
	 *
	 */
	void snLogErr(QString mess);
	/**
	 * \brief can be used by snBlocks to log data to log text widget.
	 *
	 */
	void snLogDeb(QString mess, int DebugLevel);

	/**
	 * \brief has to be called by snBlocks after processing of the actual concbuf block
	 *
	 */
	void increaseBlks();

	/**
	 * \brief has to be called by the sink snBlocks after processing of the actual concbuf block
	 *
	 */
	void increaseOutputBlks();

	/**
	 * \brief This member function prints a summary of how much data was processed after the calling snBlock's exit.
	 */
	void printSummary(std::string caller);

	SNSignal Sig; ///< holds the simulation parameters which are common to all snBlocks
	unsigned int plot_x_max;
	double max_simulation_length; // for plotting, if simulation duration due to motion is a bit longer then length given in config file.
	Blocks_Type max_blocks;
};

#endif
