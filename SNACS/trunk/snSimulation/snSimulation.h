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
 * \file snSimulation.h
 *
 * \brief Declarations common to all SNACS source files.
 *
 * \author Frank Schubert
 */

#ifndef SNSIMULATION_H_
#define SNSIMULATION_H_

#include <string>
const std::string WelcomeMessage = "SNACS - The Satellite Navigation Channel Simulation Software";
const std::string SNACSVersion = "0.1";

#include <cmath>
#include <cfloat>

#include "libconfig.h++"

typedef unsigned long long Samples_Type; ///< type for processed samples
typedef signed long Blocks_Type; ///< type for concbuf blocks
typedef double Freq_Type; ///< type for frequency

#include "snWriteResultFile.h"

/**
 * \brief Struct that contains general simulation parameters which are common to all snBlocks.
 */
struct SNSignal {
	Freq_Type SmplFreq; ///< simulation frequency
	double Ts; ///< sample interval
	double Len; ///< total simulation length
	Samples_Type aSamples; ///< total amount of samples to be processed
	int BytesPerSample; ///< for output how many bytes were processed
	Blocks_Type aBuf; ///< amount of concbuf blocks per concurrent buffer
	Samples_Type BufSize; ///< length of one concbuf block
	//double IF; ///< intermediate frequency
	Blocks_Type aBlock; ///< total amount of concbuf blocks to be processed
	//int MaxEchos;
	Samples_Type PlotXSize; ///< amount of samples to be displayed per snPlot*
	double plot_buffer_length;
	double c0; ///< speed of light [m/s]
	//double SigChAmp;
	snWriteResultFile *results_file; ///< file for output. All snBlocks write their output using this class.
};

/**
 * \brief Parameters that are passed to each snBlock and can be altered by them.
 *
 */
struct ProcessingParameters {
	double ProcessingDelay; ///< each snBlock is supposed to add its processing delay to this variable if there is one.
};

/**
 * \brief Exception class for simulation setup time and snBlocks.
 *
 */
class snSimulationException {
private:
	std::string exceptionDescription;
public:
	snSimulationException(const char *);
	snSimulationException(std::string);
	std::string what() {
		return exceptionDescription;
	}
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

#endif /* SNSIMULATION_H_ */
