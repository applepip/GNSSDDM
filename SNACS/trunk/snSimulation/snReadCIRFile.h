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
 * \file	snReadCIRFile.h
 *
 * \author	Frank Schubert
 */

#ifndef SNREADCIRFILE_H_
#define SNREADCIRFILE_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <complex>

#include <cstring>

#include "snSimulation.h"
#include "H5Cpp.h"

#ifndef H5_NO_NAMESPACE
#endif

using namespace H5;

/*!
 * \brief struct to read an echo from a HDF5 file containing CIRs.
 */
struct echo_struct_hdf5 {
	double delay;
	double real;
	double imag;
};

/*!
 * \brief struct for definition of one echo
 */
struct echo_struct {
	double delay;
	std::complex<double> weight;
};

/*!
 * \brief	struct used as return value for function getCIR.
 */
struct cir_struct {
	double ref_range;
	std::vector<echo_struct> echoes;
};


/*!
 * \brief	struct for reading general simulation parameters from HDF5 file containing CIRs.
 */
struct parameters_simulation_struct {
	double cir_rate;
	double MaximumSpeed;
	double SatElevation;
	double SatAzimut;
	Blocks_Type cir_amount;
	double c0;
};

/*!
 * \brief	Reads CIRs from CIR HDF5 file.
 *
 * \param	filename
 */
class snReadCIRFile {
public:
	snReadCIRFile(std::string filename);
	virtual ~snReadCIRFile();

	/*!
	 * \brief	returns CIR with a given number
	 *
	 * \param	<cir_num> CIR number
	 *
	 * \return	CIR
	 */
	cir_struct getCIR(Blocks_Type cir_num);

	/*!
	 * \brief	Return the CIR rate of the file
	 *
	 * \return	CIR rate
	 */
	double getCIRRate() {
		return my_params.cir_rate;
	}

	/*!
	 * \brief	Return the number of total CIRs in file
	 * \return	CIR amount
	 */
	Blocks_Type getCIRAmount() {
		return my_params.cir_amount;
	}

	/*!
	 * \brief	Returns the speed of light as defined in file.
	 * \return	value for c0
	 */
	double getC0() {
		return my_params.c0;
	}

private:
	H5File* file;
	Group *group_cirs;
	parameters_simulation_struct my_params;
	double *ref_ranges;
};

#endif /* SNREADCIRFILE_H_ */
