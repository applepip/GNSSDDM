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
 * \file snBlock.cpp
 *
 * \author Frank Schubert
 */

#include "snBlock.h"

snBlock::snBlock(SNSignal pSNSignal) :
	Sig(pSNSignal) {
	cBlks = 0;
	cOutputBlks = 0;
	fActive = true;
	total_blocks = static_cast<Blocks_Type> (Sig.Len * Sig.SmplFreq / Sig.BufSize);

	plot_x_max = floor(Sig.plot_buffer_length * Sig.SmplFreq);
	// plot_x_max must not be greater than Sig.BufSize
	if (plot_x_max > Sig.BufSize) {
		throw snSimulationException("snBlock: plot_x_max must not be greater than Sig.BufSize");
	}

	max_simulation_length = Sig.Len * 1.1;
	max_blocks = floor(static_cast<double> (getTotalBlks()) * 1.1);
}

bool snBlock::isActive() {
	mfActive.lockForRead();
	bool a = fActive;
	mfActive.unlock();
	return a;
}

void snBlock::requestStop() {
	mfActive.lockForWrite();
	fActive = false;
	mfActive.unlock();
	return;
}

Blocks_Type snBlock::getBlks() {
	return cBlks;
}

double snBlock::get_blks_simulation_time() {
	return static_cast<double> (getBlks() * Sig.BufSize) / Sig.SmplFreq;
}

Blocks_Type snBlock::getOutputBlks() {
	mBlk.lockForRead();
	Blocks_Type Blocks = cOutputBlks;
	mBlk.unlock();
	return Blocks;
}

void snBlock::increaseBlks() {
	cBlks++;
	if (cBlks > getTotalBlks()) {
		std::cout << "increaseBlks called after increaseBlks > total blks\n";
		std::cout.flush();
		std::exit(1);
	}
}

void snBlock::increaseOutputBlks() {
	mBlk.lockForWrite();
	cOutputBlks++;
	mBlk.unlock();
}

void snBlock::printSummary(std::string caller) {
	Blocks_Type total_blocks_processed = getBlks() + 1;
	//	getBlks()+1 because getBlks start at 0

	snLog(QString(QString(caller.c_str()) + ": full blocks processed: " + QString::number(
			total_blocks_processed)));
	snLog(QString(QString(caller.c_str()) + ": total of " + QString::number((total_blocks_processed)
			* Sig.BufSize) + " samples (" + QString::number(total_blocks_processed * Sig.BufSize / 1024.0
			/ 1024.0 * Sig.BytesPerSample) + " MBytes) output."));
}

Blocks_Type snBlock::getTotalBlks() {
	return total_blocks;
}

void snBlock::snLog(QString mess) {
	emit snLogSignal(mess);
}

void snBlock::snLogErr(QString mess) {
	emit snLogSignal(mess);
}

void snBlock::snLogDeb(QString mess, int d) {
	emit snLogSignalDebug(mess, d);
}
