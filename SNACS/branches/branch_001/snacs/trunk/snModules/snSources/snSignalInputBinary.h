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

#ifndef DEF_snSIGNALINPUTBINARY_H_
#define DEF_snSIGNALINPUTBINARY_H_ 1

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <complex>

#include "../snWidget/snWidget.h"
#include "../snSimulation/ConcurrentBuffer3.h"

#include "../snSimulation/snCreatingModule.h"

/**
 * \brief This class reads a file that was created for software receivers, e.g. in Matlab.
 *
 * \ingroup sourceBlocks
 *
 */
class snSignalInputBinary: public snCreatingModule {

public:

	enum input_type {
		SIGNED_CHAR, REAL_DOUBLE, COMPLEX_DOUBLE
	};

	snSignalInputBinary(libconfig::Setting &Settings, SNSignal pSNSignal, snWidget *psnw);
	~snSignalInputBinary();

	void run();

private:
	std::ifstream *iFile;

	input_type it;
	// for plotting:
	snWidget *snw;
	snCurveComplex *curve;
};
#endif
