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

#include "snSignalOutputBinary.h"

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <complex>

snSignalOutputBinary::snSignalOutputBinary(libconfig::Setting &Settings, SNSignal pSNSignal, snWidget *psnw,
		ConcBuf* pConcBuf1) :
	snBlock(pSNSignal), snw(psnw), ConcBuf1(pConcBuf1)

{

	curve = new snCurveReal(plot_x_max, Qt::blue);
	snPlotReal *plot_real = new snPlotReal("real", snPlot::Linear, curve);
	plot_real->setXAxis(0.0, Sig.plot_buffer_length);
	plot_real->setXAxisLabel("t [s]");
	plot_real->setYAxisLabel("A");

	snw->addPlot(plot_real);

	std::string filename = static_cast<const char *> (Settings["file"]);
	oFile = new std::fstream(filename.c_str(), std::ios::out | std::ios::binary);
	if (!*oFile) {
		throw snSimulationException("snSignalOutputBinary: cannot open output file");
	}

	std::string output_str = static_cast<const char *> (Settings["data_type"]);

	if (output_str == "signed char") {
		ot = SIGNED_CHAR;
	} else if (output_str == "double") {
		ot = DOUBLE;
	} else {
		throw snSimulationException("snSignalOutputBinary: unknown data type.");
	}

	// TODO write parameters to hdf5 file
}

snSignalOutputBinary::~snSignalOutputBinary() {
}

void snSignalOutputBinary::run() {
	snLog("snSignalOutputBinary: start.");

	std::complex<double> *Buf;
	Samples_Type nToRead;

	while (1) {
		Buf = ConcBuf1->RequestReading(&nToRead);

		double OutByte[nToRead];

		switch (ot) {
		case SIGNED_CHAR:
			for (Samples_Type i = 0; i < nToRead; i++) {
				signed char output_value = static_cast<signed char> (Buf[i].real());
				oFile ->write(reinterpret_cast<char*> (&output_value), sizeof(output_value));
				OutByte[i] = output_value;
			}
			break;
		case DOUBLE:
			for (Samples_Type i = 0; i < nToRead; i++) {
				double output_value = Buf[i].real();
				oFile->write(reinterpret_cast<char*> (&output_value), sizeof(output_value));
				OutByte[i] = output_value;
			}
			break;
		}

		// should we plot?
		if (snw->requestPlotFlag()) {
			for (Samples_Type o = 0; o < plot_x_max; o++) {
				curve->X[o] = static_cast<double> (o) / Sig.SmplFreq;
				curve->Y[o] = OutByte[o];
			}
			snw->releasePlotFlag();
		}

		if (ConcBuf1->ReleaseReading()) {
			oFile->flush();
			snLog("SignalOutputBinary: exit.");
			printSummary("snSignalOutputBinary");
			return;
		}
		increaseBlks();
		increaseOutputBlks();
	}
}
