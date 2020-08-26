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
 * \file snGPS.h
 *
 * \author Frank Schubert
 */

#ifndef SNGPS_H
#define SNGPS_H

#include <vector>

#include "snGNSSCode.h"

/**
 * \brief Class of member functions for GPS signal generation.
 *
 *
 */
class snGPS: public snGNSSCode {
public:
	/**
	 * \brief constructor
	 */
	snGPS(int _prn);
	~snGPS();

private:
	/**
	 * \brief generates one C/A pseudorandom code sequence
	 * This function is based on the CA-Code generation function of
	 * K. Borre et. al: "A Software-Defined GPS and Galileo Receiver"
	 *
	 * \param <int prn> PRN number
	 * \return <signed char *ca> code sequence
	 */
	std::vector<double> generate_ca_code(int prn);
};

#endif
