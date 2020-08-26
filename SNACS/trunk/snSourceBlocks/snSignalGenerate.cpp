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

#include "snSignalGenerate.h"

snSignalGenerate::snSignalGenerate(libconfig::Setting &Settings,
		SNSignal pSNSignal, snWidget *psnw, ConcBuf* pConcBuf1) :
	snBlock(pSNSignal), snw(psnw), ConcBuf1(pConcBuf1) {

	snLog("SignalGenerate: setup.");

	//	code = new snGPS(prn);
	//	modulation = new snModulationBPSK(code, Sig.SmplFreq);
	//	code = new snCodeE1B(prn);
	//	modulation = new snModulationBOC(code, Sig.SmplFreq, 1, 1);

	gnss_signal = new snGNSSSignal(Settings["signal"], Sig.SmplFreq, Sig.Len);
	//code = gnss_signal->get_inphase_modulation()->get_code();
	inphase_enabled = gnss_signal->inphase_enabled();
	quadrature_enabled = gnss_signal->quadrature_enabled();
	if (inphase_enabled)
		inphase_modulation = gnss_signal->get_inphase_modulation();
	if (quadrature_enabled)
		quadrature_modulation = gnss_signal->get_quadrature_modulation();

	start_sample = static_cast<double> (Settings["start"]) * Sig.SmplFreq;

	plot_x_max = floor(Sig.plot_buffer_length * Sig.SmplFreq);
	curve = new snCurveComplex(plot_x_max, Qt::blue, Qt::red);
	snPlotComplexRealImag *plot_real_imag = new snPlotComplexRealImag(
			"real / imag", snPlot::Linear, curve);
	plot_real_imag->setXAxis(0.0, Sig.plot_buffer_length);
	plot_real_imag->setYAxis(-2.5, 2.5);
	plot_real_imag->setXAxisLabel("t [s]");
	plot_real_imag->setYAxisLabel("A");
	snPlotMagn *plot_magn = new snPlotMagn("magnitude", snPlot::Log20, curve, Sig.SmplFreq);
	snPlotAngl *plot_angl = new snPlotAngl("angle", snPlot::Linear, curve, Sig.SmplFreq);

	snw->addPlot(plot_real_imag);
	snw->addPlot(plot_magn);
	snw->addPlot(plot_angl);
}

snSignalGenerate::~snSignalGenerate() {
	delete gnss_signal;
}

void snSignalGenerate::run() {

	snLog("SignalGenerate: start.");

	//Samples_Type samples_per_code = modulation->get_samples_per_code();
	//int chips_per_code = code->get_number_of_chips_per_code();

	std::complex<double> *Buf;
	Samples_Type absolute_sample = 0;
	while (isActive()) {
		Buf = ConcBuf1->RequestWriting();
		for (Samples_Type o = 0; o < Sig.BufSize; o++) {

			//Samples_Type absolute_sample = o + getBlks() * Sig.BufSize;
			if (absolute_sample >= start_sample) {
				// amplitude in baseband with power P is A = sqrt(2.0 * P)
				// here: P = 1
				//				const int ca_idx = floor(static_cast<double> (fmod(absolute_sample, samples_per_code))
				//						/ static_cast<double> ((samples_per_code)) * (GPS.CAlen));
				//				const int code_chip_index = floor(static_cast<double> (fmod(absolute_sample - start_sample,
				//						samples_per_code)) / static_cast<double> (samples_per_code) * chips_per_code);
				//std::cout << "o: " << o << ", ca_idx: " << ca_idx << "\n";
				//				Buf[o].real() = sqrt(2.0) * code->get_chip(code_chip_index);
				const Samples_Type sample_number = absolute_sample
						- start_sample;
				if (inphase_enabled)
					Buf[o].real()
							= sqrt(1.0)
									* inphase_modulation->get_value_for_absolute_sample_number(
											sample_number);
				else
					Buf[o].real() = 0.0;

				if (quadrature_enabled)
					Buf[o].imag()
							= sqrt(1.0)
									* quadrature_modulation->get_value_for_absolute_sample_number(
											sample_number);
				else
					Buf[o].imag() = 0.0;

			} else {
				Buf[o] = std::complex<double>(0.0, 0.0);
			}

			if (((getBlks() + 1) * Sig.BufSize + o) >= Sig.aSamples) {
				//				(getBlks() + 1) because getBlks start at 0
				ConcBuf1->ReleaseWriting(o, true); // o is amount, but starts at 0, so +1. but eof happened one loop run before, so just o values were read in.
				requestStop();
				snLog("SignalGenerate: exit.");
				printSummary("snSignalGenerate");
				return;
			}
			absolute_sample++;

		}
		// should we plot?
		if (snw->requestPlotFlag()) {
			for (Samples_Type o = 0; o < plot_x_max; o++) {
				curve->X.at(o) = static_cast<double> (o) / Sig.SmplFreq; // in s
				curve->Y.at(o) = Buf[o];
			}
			snw->releasePlotFlag();
		}

		ConcBuf1->ReleaseWriting(Sig.BufSize, false);
		increaseBlks();
	}
	// Abort has been clicked.
	//	 do the following to let the following snBlocks exit cleanly (only necessary for snSourceBlocks:
	Buf = ConcBuf1->RequestWriting();
	ConcBuf1->ReleaseWriting(0, 1);
	snLog("SignalGenerate: exit.");
	printSummary("snSignalGenerate");
	return;
}
