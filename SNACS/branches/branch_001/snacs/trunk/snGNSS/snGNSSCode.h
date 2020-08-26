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
 * \file snGNSSCode.h
 *
 * \author Frank Schubert
 */

#ifndef SNGNSSCode_H
#define SNGNSSCode_H

#include <vector>

#include "../snSimulation/snGlobal.h"

/**
 * \brief Class of member functions for GPS signal generation.
 */
class snGNSSCode {
public:

	snGNSSCode(int _prn);
	~snGNSSCode();

	/**
	 * \brief returns the code length in seconds
	 */
	double get_code_length_time() {
		return static_cast<double> (get_number_of_chips_per_code()) / get_code_freq_cps();
	}

	/**
	 * \brief returns one code in chips
	 */
	std::vector<double> get_code() {
		return code;
	}

	/**
	 * \brief returns one chip
	 */
	double get_chip(int n) {
		return code.at(n);
	}

	/**
	 * \brief get the code frequency (chips per second)
	 */
	double get_code_freq_cps() {
		return f_cps;
	}

	/**
	 * \brief get number of chips per code
	 */
	int get_number_of_chips_per_code() {
		return code.size();
	}

	/**
	 * \brief set the chipping rate
	 */
	void set_freq_cps(double f_cps_new) {
		f_cps = f_cps_new;
	}

protected:
	int prn;
	std::vector<double> code;
	double f_cps;
};

#endif
