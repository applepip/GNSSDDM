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

//#include <cstdlib>
#include <cstdio>
#include "remez-algorithm/remez.c"

#include "snProcessorLPF.h"

snProcessorLPF::snProcessorLPF(libconfig::Setting &Settings, SNSignal pSNSignal,
		ProcessingParameters &ProcSig, snWidget *psnw, ConcBuf* pConcBuf1, ConcBuf* pConcBuf2) :
	snBlock(pSNSignal), snw(psnw), ConcBuf1(pConcBuf1), ConcBuf2(pConcBuf2) {

	snLog("snProcessorLPF: start.");

	// filter order
	filter_order = Settings["order"];

	numtaps = filter_order + 1; // TODO numtaps must be greater than 3

	Coeffs.resize(numtaps);

	cut_off_frequency = Settings["cut_off_frequency"];
	if (cut_off_frequency >= Sig.SmplFreq / 2.0) {
		throw snSimulationException(
				"snProcessorLPF: low pass filter cut-off frequency is larger or equal than Sig.SmplFreq/2.0");
	}

	// filter type
	std::string filter_type = static_cast<const char *> (Settings["coefficients_type"]);

	// calculate filter coefficients
	if (filter_type == "sinc") {
		double Omg = ((2.0 * M_PI * cut_off_frequency) / Sig.SmplFreq);
		for (size_t i = 0; i < Coeffs.size(); i++) {
			double fac = Omg * (((double) i) - (((double) filter_order) / 2.0));
			if (fac == 0) {
				Coeffs.at(i) = std::complex<double>(Omg / M_PI, 0.0);
			} else {
				Coeffs.at(i) = std::complex<double>((Omg / M_PI) * sin(fac) / fac, 0.0);
			}
		}
	} else if (filter_type == "remez") {
		std::vector<double> remez_coeffs = remez_coefficients(cut_off_frequency / Sig.SmplFreq
				/ 2.0, numtaps);
		for (size_t i = 0; i < numtaps; i++) {
			Coeffs.at(i) = std::complex<double>(remez_coeffs.at(i), .0);
		}

	} else {
		throw snSimulationException(
				"Unknown filter coefficient calculation type. Must be either sinc or remez.");
	}

	// setup FIR filter:
	CmplxFilter = new snCmplxFIRFilter(Sig.BufSize, filter_order);

	// processing delay caused by LPF
	// filter_order - 1 = number of FIR taps
	ProcSig.ProcessingDelay += static_cast<double> (filter_order) / 2.0 / Sig.SmplFreq;

	// modify our widget
	curve = new snCurveComplex(plot_x_max, Qt::blue, Qt::red);
	snPlotComplexRealImag *plot_real_imag = new snPlotComplexRealImag("real / imag",
			snPlot::Linear, curve);
	plot_real_imag->setXAxis(0.0, Sig.plot_buffer_length);
	plot_real_imag->setYAxis(-1.5, 1.5);
	plot_real_imag->setXAxisLabel("t [s]");
	plot_real_imag->setYAxisLabel("A");

	snPlotMagn *plot_magn = new snPlotMagn("magn", snPlot::Log20, curve, Sig.SmplFreq);
	snPlotAngl *plot_angl = new snPlotAngl("angl", snPlot::Linear, curve, Sig.SmplFreq);

	snw->addPlot(plot_real_imag);
	snw->addPlot(plot_magn);
	snw->addPlot(plot_angl);
}

snProcessorLPF::~snProcessorLPF() {
	delete CmplxFilter;
}

void snProcessorLPF::run() {

	// calculate hann window:
	/*
	 std::vector<std::complex<double> > hann(filter_order + 1);
	 for (size_t i = 0; i < hann.size(); i++) {
	 hann.at(i).real() = .5 * (1.0 - cos(2 * M_PI / filter_order * i));
	 hann.at(i).imag() = 0;
	 }
	 for (size_t i = 0; i < Coeffs.size(); i++) {
	 Coeffs.at(i) = Coeffs.at(i) * hann.at(i);
	 }
	 */

	while (isActive()) {
		Samples_Type nToRead;
		std::complex<double> *BufIn = ConcBuf1->RequestReading(&nToRead);
		std::complex<double> *BufOut = ConcBuf2->RequestWriting();

		CmplxFilter->filter(BufIn, BufOut, Coeffs);

		if (ConcBuf1->ReleaseReading()) {
			ConcBuf2->ReleaseWriting(nToRead, 1);
			snLog("snProcessorLPF: exit.");
			printSummary("snProcessorLPF");
			return;
		} else {
			// should we plot?
			if (snw->requestPlotFlag()) {
				for (Samples_Type o = 0; o < plot_x_max; o++) {
					curve->X.at(o) = static_cast<double> (o) / Sig.SmplFreq; // in s
					curve->Y.at(o) = BufOut[o];
				}
				snw->releasePlotFlag();
			}

			ConcBuf2->ReleaseWriting(nToRead, 0);
		}
		increaseBlks();
	}
}

std::vector<double> snProcessorLPF::remez_coefficients(double stop, size_t numtaps) {
	const int numbands = 2;

	double h[numtaps];

	double bands[2 * numbands];
	double desired[numbands];
	double weights[numbands];

	desired[0] = 1;
	desired[1] = 0;

	weights[0] = 1.0;
	weights[1] = 1.0;

	bands[0] = 0;
	bands[1] = stop;
	bands[2] = 0.1 + (stop);
	bands[3] = 0.5;

	std::cout << "LPF: band[0]" << bands[0] << ", band[1]: " << bands[1] << ", band[2]: "
			<< bands[2] << ", band[3]: " << bands[3] << "\n";
	std::cout.flush();

	int error = remez(h, numtaps, numbands, bands, desired, weights, BANDPASS);

	if (error != 0) {
		std::cout << "remez error.\n";
		std::cout.flush();
		throw snSimulationException("Remez filter design failed.");
	}

	std::vector<double> out(numtaps);
	for (size_t i = 0; i < numtaps; i++) {
		out.at(i) = h[i];
		//std::cout << "LPF coeff(" << i << "): " << h[i] << "\n";
	}
	return out;
}
