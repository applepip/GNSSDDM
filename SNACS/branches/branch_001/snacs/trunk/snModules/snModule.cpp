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

#include "snModule.h"
#include <sstream>

snModule::snModule(SNSignal pSNSignal) :
		Sig(pSNSignal) {
	fActive = true;

//	plot_x_max = floor(Sig.plot_buffer_length * smpl_freq);
	// plot_x_max must not be greater than Sig.BufSize
	/*
	 if (plot_x_max > block_size) {
	 throw snSimulationException("snBlock: plot_x_max must not be greater than block_size");
	 }
	 */
	//max_simulation_length = Sig.Len * 1.1;
	//max_blocks = floor(static_cast<double> (getTotalBlks()) * 1.1);
}

bool snModule::isActive() {
	mfActive.lockForRead();
	bool a = fActive;
	mfActive.unlock();
	return a;
}

void snModule::requestStop() {
	mfActive.lockForWrite();
	fActive = false;
	mfActive.unlock();
	return;
}

void snModule::printSummary(std::string caller) {
	snLog(QString(QString(caller.c_str()) + " done."));
}

void snModule::snLog(QString mess, LogType d) {
	emit LogSignal(mess, d);
}

bool snModule::do_write_txt_signals(Blocks_Type blk_num) {
//	std::cout << "Sig.txt_data_block_interval: " << Sig.txt_data_block_interval
//			<< ", blk_num: " << blk_num << "\n";

	if (Sig.write_txt_data == true) {
		if (blk_num % Sig.txt_data_block_interval == 0)
			return true;
	}
	return false;
}

txt_signal::txt_signal(SNSignal pSNSignal, std::string sig_name,
		Blocks_Type blk_num) :
		Sig(pSNSignal) {
	std::ostringstream s;

//	std::cout << "Sig.txt_data_path: " << Sig.txt_data_path << "\n";

	s << Sig.txt_data_path << blk_num / Sig.txt_data_block_interval << "/"
			<< sig_name;
	std::cout << "open file " << s.str().c_str() << "...\n";
	f.open(s.str().c_str(), std::ios::out);
	if (!f) {
		std::cerr << "txt_signal::txt_signal: cannot open output file.\n";
		std::cerr.flush();
		throw snSimulationException("txt_signal: cannot open output file.");
	}
}

txt_signal::~txt_signal() {
	f.close();
}

void txt_signal::write_txt_vectors(itpp::vec &a, itpp::vec &b) {
	if (Sig.write_txt_data == true) {
		size_t nof_points = Sig.nof_txt_samples;
		if (a.length() < Sig.nof_txt_samples
				|| b.length() < Sig.nof_txt_samples) {
			nof_points = a.length();
			if (b.length() < nof_points)
				nof_points = b.length();
		}

//		if (a.length() != b.length())
//			throw("write_txt_vectors: length a is not equal to length b.");

		for (size_t k = 0; k < nof_points; k++) {
			f << a(k);
			if (k != nof_points - 1)
				f << " ";
		}
		f << std::endl;
		for (size_t k = 0; k < nof_points; k++) {
			f << b(k);
			if (k != nof_points - 1)
				f << " ";
		}
		f << std::endl;
	}
}
