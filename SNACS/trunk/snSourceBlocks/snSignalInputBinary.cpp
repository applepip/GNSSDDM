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

#include "snSignalInputBinary.h"

snSignalInputBinary::snSignalInputBinary(libconfig::Setting &Settings, SNSignal pSNSignal,
		snWidget *psnw, ConcBuf* pConcBuf1) :
	snBlock(pSNSignal), snw(psnw), ConcBuf1(pConcBuf1)

{

	std::string filename = static_cast<const char *> (Settings["file"]);

	iFile = new std::ifstream(filename.c_str(), std::ios::in | std::ios::binary);

	if (!*iFile) {
		throw snSimulationException("snSignalInputBinary: cannot open input file");
	}

	std::string data_type_str = static_cast<const char *> (Settings["data_type"]);

	if (data_type_str == "signed char") {
		it = SIGNED_CHAR;
	} else if (data_type_str == "double") {
		it = DOUBLE;
	} else {
		throw snSimulationException("snSignalInputBinary: unknown data type.");
	}

	// TODO check if file is large enough

	curve = new snCurveReal(plot_x_max, Qt::blue);
	snPlotReal *plot_real = new snPlotReal("real", snPlot::Linear, curve);
	snw->addPlot(plot_real);
	plot_real->setXAxis(0.0, Sig.plot_buffer_length);
	plot_real->setXAxisLabel("t [s]");
	plot_real->setYAxisLabel("A");
}

snSignalInputBinary::~snSignalInputBinary() {
}

void snSignalInputBinary::run() {
	snLog("snSignalInputBinary");
	std::complex<double> *Buf;
	while (isActive()) {
		Buf = ConcBuf1->RequestWriting();

		switch (it) {
		case SIGNED_CHAR:
			for (Samples_Type i = 0; i < Sig.BufSize; i++) {
				signed char input_value = static_cast<signed char> (Buf[i].real());
				iFile->read(reinterpret_cast<char*> (&input_value), sizeof(input_value));
				Buf[i] = std::complex<double>(input_value, 0.0);
			}
			break;
		case DOUBLE:
			for (Samples_Type i = 0; i < Sig.BufSize; i++) {
				double input_value = Buf[i].real();
				iFile->read(reinterpret_cast<char*> (&input_value), sizeof(input_value));
				Buf[i] = std::complex<double>(input_value, 0.0);
			}
			break;
		}

		//		if (iFile->eof() || (Sig.Len * Sig.SmplFreq < getBlks() * Sig.BufSize)) {
		if (iFile->eof() || (getBlks() + 1 == getTotalBlks())) {
			ConcBuf1->ReleaseWriting(Sig.BufSize, 1); // i is amount, but starts at 0, so +1. but eof happened one loop run before, so just i values were read in.

			iFile->close();
			snLog("snSignalInputBinary exit.");
			printSummary("snSignalInputBinary");
			return;
		}

		// should we plot?
		if (snw->requestPlotFlag()) {
			for (Samples_Type o = 0; o < plot_x_max; o++) {
				curve->X[o] = static_cast<double> (o) / Sig.SmplFreq;
				curve->Y[o] = Buf[o].real();
			}
			snw->releasePlotFlag();
		}

		ConcBuf1->ReleaseWriting(Sig.BufSize, 0);
		increaseBlks();
	}
	//	// do the following to let the following snBlocks exit cleanly:
	Buf = ConcBuf1->RequestWriting();
	ConcBuf1->ReleaseWriting(0, 1);
}
