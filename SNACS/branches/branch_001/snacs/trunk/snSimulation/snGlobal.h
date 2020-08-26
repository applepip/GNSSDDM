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
 * \file snGlobal.h
 *
 * \brief Declarations common to all SNACS source files.
 *
 * \author Frank Schubert
 */

#ifndef SNGLOBAL_H_
#define SNGLOBAL_H_

#include <string>
const std::string WelcomeMessage = "SNACS - The Satellite Navigation Channel Simulation Software";
const std::string SNACSVersion = "0.2";

#include <cmath>
#include <cfloat>

#include "libconfig.h++"
#include <itpp/itbase.h>

typedef unsigned long long Samples_Type; ///< type for processed samples
typedef unsigned long Blocks_Type; ///< type for concbuf blocks
typedef double Freq_Type; ///< type for frequency

#include "snWriteResultFile.h"

/**
 * \brief Struct that contains general simulation parameters which are common to all snBlocks.
 */
struct SNSignal {
	int BytesPerSample; ///< for output how many bytes were processed
	Samples_Type PlotXSize; ///< amount of samples to be displayed per snPlot*
	double plot_buffer_length;
	double c0; ///< speed of light [m/s]
	std::string results_directory;
	snWriteResultFile *results_file; ///< file for output. All snBlocks write their output using this class.

	bool write_txt_data;
	Blocks_Type txt_data_block_interval;
	Samples_Type nof_txt_samples;
	std::string txt_data_path;
};

/**
 * \brief Parameters that are passed to each snBlock and can be altered by them.
 *
 */
struct ProcessingParameters {
	double ProcessingDelay; ///< each snBlock is supposed to add its processing delay to this variable if there is one, in seconds.
	double norm_eq_noise_bw; ///< normalized equivalent noise bandwidth
};

/**
 * \brief Exception class for simulation setup time and snBlocks.
 *
 */
class snSimulationException: public std::exception {
public:
	snSimulationException(const char *);
	snSimulationException(std::string);
	~snSimulationException() throw() { };
	virtual const char* what() const throw () {
		return exceptionDescription.c_str();
	}
private:
	std::string exceptionDescription;
};

/**
 * \brief compares two doubles with respect to floating point accuracy
 *
 */
bool doubles_equal(double a, double b);

/**
 * \brief Checks wether a double is integer or not
 *
 */
bool isInt(double a);

#endif
