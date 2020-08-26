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
#include <iostream>
#include <iomanip>
#include <complex>

snSignalOutputBinary::snSignalOutputBinary(libconfig::Setting &Settings, SNSignal pSNSignal,
		ProcessingParameters &_ProcSig, snWidget *psnw, ConcBuf3* _circbuf) :
		snSerialModule(pSNSignal, _circbuf), ProcSig(_ProcSig), snw(psnw)

{
	nBufs = Settings["buffers_to_display"];

	std::cout << "snSignalOutputBinary: block_size: " << block_size << "\n";

//	curve = new snCurveComplex(nBufs * block_size, snCurve::blue, snCurve::red);
	curve = snw->get_new_curve_complex(nBufs * block_size, snCurve::blue, snCurve::red);
//	snPlot *plot_complex = new snPlot("real / imag", snPlot::Linear, curve);
	snPlot *plot_complex = snw->get_new_plot_complex("real / imag", snPlot::Linear, curve);
	plot_complex->setXAxis(0.0, Sig.plot_buffer_length);
	plot_complex->setXAxisLabel("t [s]");
	plot_complex->setYAxisLabel("A");

	snw->addPlot(plot_complex);

	std::string filename = static_cast<const char *>(Settings["file"]);
	oFile = new std::fstream(filename.c_str(), std::ios::out | std::ios::binary);
	if (!*oFile) {
		throw snSimulationException("snSignalOutputBinary: cannot open output file");
	}

	std::string output_str = static_cast<const char *>(Settings["data_type"]);

	if (output_str == "signed char") {
		ot = SIGNED_CHAR;
	} else if (output_str == "real double") {
		ot = REAL_DOUBLE;
	} else if (output_str == "complex double") {
		ot = COMPLEX_DOUBLE;
	} else {
		throw snSimulationException("snSignalOutputBinary: unknown data type.");
	}

	// init serial buffer
	InitSerialBuffer(2 * block_size);

	// TODO write parameters to hdf5 file
}

snSignalOutputBinary::~snSignalOutputBinary() {
}

void snSignalOutputBinary::run() {
	snLog("snSignalOutputBinary: start.");

	while (true) {
		itpp::cvec circ_buf = get_next_samples(block_size);

		if (static_cast<unsigned int>(circ_buf.size()) != block_size) {
			// simulation is over now.
			snLog("snSignalOutputBinary: exit.");
			printSummary("snSignalOutputBinary");
			return;
		}

		itpp::cvec OutByte(block_size);

		switch (ot) {
		case SIGNED_CHAR:
			for (Samples_Type i = 0; i < block_size; i++) {
				signed char output_value = static_cast<signed char>(circ_buf[i].real());
				oFile->write(reinterpret_cast<char*>(&output_value), sizeof(output_value));
				OutByte[i] = output_value;
			}
			break;
		case REAL_DOUBLE:
			for (Samples_Type i = 0; i < block_size; i++) {
				double output_value = circ_buf[i].real();
				oFile->write(reinterpret_cast<char*>(&output_value), sizeof(output_value));
				OutByte[i] = output_value;
			}
			break;
		case COMPLEX_DOUBLE:
			for (Samples_Type i = 0; i < block_size; i++) {
				double output_value_r = circ_buf[i].real();
				double output_value_i = circ_buf[i].imag();
				oFile->write(reinterpret_cast<char*>(&output_value_r), sizeof(output_value_r));
				oFile->write(reinterpret_cast<char*>(&output_value_i), sizeof(output_value_i));
				OutByte[i] = circ_buf[i];
			}
			break;
		}

		// should we plot?
		if (snw->requestPlotFlag()) {
			for (Samples_Type o = 0; o < block_size; o++) {
				curve->update_value_X(o, static_cast<double>(o) / smpl_freq);
				curve->update_value_Y(o, OutByte[o]);
			}
			snw->releasePlotFlag();
		}

		// set the current simulation time
		set_current_simulation_time(
				circbuf->get_current_consumer_block() / static_cast<double>(circbuf->get_total_blocks())
						* signal_length);
	}
}
