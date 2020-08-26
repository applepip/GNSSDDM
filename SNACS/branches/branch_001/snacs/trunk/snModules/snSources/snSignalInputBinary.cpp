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

snSignalInputBinary::snSignalInputBinary(libconfig::Setting &Settings, SNSignal pSNSignal, snWidget *psnw) :
snCreatingModule(pSNSignal, "snSignalGenerate",
				Settings["sampling_frequency"], Settings["signal_length"],
				Settings["block_length"],
				Settings["number_of_blocks_per_buffer"]), snw(psnw)

{

	std::string filename = static_cast<const char *>(Settings["file"]);

	iFile = new std::ifstream(filename.c_str(), std::ios::in | std::ios::binary);

	if (!*iFile) {
		throw snSimulationException("snSignalInputBinary: cannot open input file");
	}

	std::string data_type_str = static_cast<const char *>(Settings["data_type"]);

	if (data_type_str == "signed char") {
		it = SIGNED_CHAR;
	} else if (data_type_str == "real double") {
		it = REAL_DOUBLE;
	} else if (data_type_str == "complex double") {
		it = COMPLEX_DOUBLE;
	} else {
		throw snSimulationException("snSignalInputBinary: unknown data type.");
	}

	// TODO check if file is large enough

//	curve = new snCurveComplex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
	curve = snw->get_new_curve_complex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
//	snPlot *plot_complex = new snPlot("real", snPlot::Linear, curve);
	snPlot *plot_complex = snw->get_new_plot_complex("real", snPlot::Linear, curve);
	snw->addPlot(plot_complex);
	plot_complex->setXAxis(0.0, get_max_xaxis_value());
	plot_complex->setXAxisLabel("t [s]");
	plot_complex->setYAxisLabel("A");
}

snSignalInputBinary::~snSignalInputBinary() {
}

void snSignalInputBinary::run() {
	snLog("snSignalInputBinary start.");

	Samples_Type absolute_sample = 0;
	const Samples_Type total_samples = conc_buf->get_total_samples();

	while (true) {
		itpp::cvec& Buf = conc_buf->RequestWriting();

		switch (it) {
		case SIGNED_CHAR:
			for (Samples_Type i = 0; i < block_size; i++) {
				signed char input_value = static_cast<signed char>(Buf[i].real());
				iFile->read(reinterpret_cast<char*>(&input_value), sizeof(input_value));
				Buf[i] = std::complex<double>(input_value, 0.0);
			}
			break;
		case REAL_DOUBLE:
			for (Samples_Type i = 0; i < block_size; i++) {
				double input_value = 0.0;
				iFile->read(reinterpret_cast<char*>(&input_value), sizeof(input_value));
				Buf[i] = std::complex<double>(input_value, 0.0);
			}
			break;
		case COMPLEX_DOUBLE:
			for (Samples_Type i = 0; i < block_size; i++) {
				double input_value_r = 0.0;
				double input_value_i = 0.0;
				iFile->read(reinterpret_cast<char*>(&input_value_r), sizeof(input_value_r));
				iFile->read(reinterpret_cast<char*>(&input_value_i), sizeof(input_value_i));
				Buf[i] = std::complex<double>(input_value_r, input_value_i);
			}
			break;
		}

		if ((absolute_sample + 1 == total_samples) || !isActive() || iFile->eof()) {
			// absolute_sample + 1 because absolute_sample starts at 0 and total_samples gives the absolut number starting at 1
			conc_buf->ReleaseWriting(true);
			requestStop();
			snLog("snSignalInputBinary: exit.");
			printSummary("snSignalInputBinary");
			return;
		}
		// should we plot?
		if (snw->requestPlotFlag()) {
			for (Samples_Type o = 0; o < get_nof_xaxis_indices(); o++) {
				curve->update_value_X(o, static_cast<double>(o) / smpl_freq);
				curve->update_value_Y(o, Buf[o]);
			}
			snw->releasePlotFlag();
		}

		absolute_sample++;
		conc_buf->ReleaseWriting(false);
	}
}
