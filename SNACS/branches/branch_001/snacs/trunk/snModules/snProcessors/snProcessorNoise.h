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

#ifndef DEF_snProcessorNoise_H_
#define DEF_snProcessorNoise_H_ 1

#include <cstdlib>
#include <complex>

#include <boost/random/mersenne_twister.hpp>

#include "../snWidget/snWidget.h"
#include "../snSimulation/snGlobal.h"
#include "../snModules/snCreatingModule.h"
#include "../snSimulation/ConcurrentBuffer3.h"

typedef boost::mt19937 base_generator_type;

/**
 * \brief Class for AWGN noise.
 *
 * \ingroup procBlocks
 */
class snProcessorNoise: public snCreatingModule {

public:
	snProcessorNoise(libconfig::Setting &Settings, SNSignal pSNSignal, widget_factory_base *widget_factory,
					ConcBuf3 *_circ_buf_in);
	~snProcessorNoise();

	void run();
private:
	double gaussian();

	base_generator_type random_number_generator;

	double n0_dB, n0_lin;
	double start_noise_at_sec;
	// for noise generator:
	double gstore;
	double rconst1;
	double rconst2;
	int ready;

	// for plotting:
	snWidget *snw;
	snCurveComplex *curve;

	ConcBuf3 *circ_buf_in;
};

#endif
