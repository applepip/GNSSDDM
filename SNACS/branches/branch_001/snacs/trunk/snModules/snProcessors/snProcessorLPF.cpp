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
		ProcessingParameters &ProcSig, widget_factory_base *widget_factory, ConcBuf3 *_conc_buf_in) :
	snCreatingModule(pSNSignal, "snLPF", _conc_buf_in), conc_buf_in(_conc_buf_in) {

	snLog("snProcessorLPF: start.");
	// filter order
	filter_order = Settings["order"];

	numtaps = filter_order + 1; // TODO numtaps must be greater than 3

	Coeffs.set_size(numtaps);

	cut_off_frequency = Settings["cut_off_frequency"];
	if (cut_off_frequency >= smpl_freq / 2.0) {
		throw snSimulationException(
				"snProcessorLPF: low pass filter cut-off frequency must be smaller than smpl_freq / 2.0");
	}

	// filter type
	std::string filter_type = static_cast<const char *> (Settings["coefficients_type"]);

	// calculate filter coefficients
	if (filter_type == "sinc") {
		const double Omg = ((2.0 * M_PI * cut_off_frequency) / smpl_freq);
		for (int i = 0; i < Coeffs.size(); i++) {
			double fac = Omg * (((double) i) - (((double) filter_order) / 2.0));
			if (fac == 0) {
				Coeffs[i] = Omg / M_PI;
			} else {
				Coeffs[i] = (Omg / M_PI) * sin(fac) / fac;
			}
		}
	} else if (filter_type == "window") {
		Coeffs = itpp::fir1(filter_order, (2.0 * cut_off_frequency) / smpl_freq);
	} else if (filter_type == "remez") {
		std::vector<double> remez_coeffs = remez_coefficients(cut_off_frequency / smpl_freq / 2.0, numtaps);
		for (size_t i = 0; i < numtaps; i++) {
			Coeffs[i] = remez_coeffs.at(i);
		}

	} else {
		throw snSimulationException(
				"Unknown filter coefficient calculation type. Must be either sinc or remez.");
	}

	// calculate normalized equivalent noise bandwidth:
	double NBW_eqn = itpp::sum(itpp::pow(itpp::abs(Coeffs), 2.0)) / pow(fabs(itpp::sum(Coeffs)), 2.0);
	//std::cout << "sum: " << itpp::sum(Coeffs) << ", abs: " << fabs(itpp::sum(Coeffs)) << ", denominator: "
	//		<< pow(fabs(itpp::sum(Coeffs)), 2.0) << "\n";
	std::cout << "normalized equivalent noise bandwidth by filter: " << NBW_eqn << "\n";
	//	ProcSig.norm_eq_noise_bw = NBW_eqn;
	// calculate equivalent noise bandwidth:
	//double NBW_eq = smpl_freq * NBW_eqn;
	ProcSig.norm_eq_noise_bw = 1.;
	//ProcSig.norm_eq_noise_bw = cut_off_frequency;

	// setup FIR filter:
	//CmplxFilter = new snCmplxFIRFilter(conc_buf_in->get_block_size(), filter_order);

#ifdef HAVE_IPP
	FIR_r = new snIPPRealFIRFilter(conc_buf_in->get_block_size(), numtaps);
	FIR_i = new snIPPRealFIRFilter(conc_buf_in->get_block_size(), numtaps);
#endif

	// processing delay caused by LPF
	// filter_order - 1 = number of FIR taps
	ProcSig.ProcessingDelay += static_cast<double> (filter_order) / 2.0 / smpl_freq;

	snw = widget_factory->get_new_widget("Low Pass Filter");

	// modify our widget for the outgoing signal
	curve = new snCurveComplex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
//	snPlot *plot_magn = snw->get_new_plot_complex("FFT, Magnitude", snPlot::FFTMagnitudeDB, curve);
	snPlot *plot_real_imag = snw->get_new_plot_complex("real / imag", snPlot::Linear, curve);
	plot_real_imag->setXAxis(0.0, get_max_xaxis_value());
	plot_real_imag->setYAxis(-2, 2);
	plot_real_imag->setXAxisLabel("t [s]");
	plot_real_imag->setYAxisLabel("A");

	snPlot *plot_magn = snw->get_new_plot_complex("magn", snPlot::FFTMagnitudeDB, curve);
	snPlot *plot_angl = snw->get_new_plot_complex("angl", snPlot::FFTAngle, curve);

	snw->addPlot(plot_real_imag);
	snw->addPlot(plot_magn);
	snw->addPlot(plot_angl);
}

snProcessorLPF::~snProcessorLPF() {
	//delete CmplxFilter;
#ifdef HAVE_IPP
	delete FIR_r;
	delete FIR_i;
#endif
}

void snProcessorLPF::run() {
	itpp::vec state_r(Coeffs.size());
	itpp::vec state_out_r(Coeffs.size());

	itpp::vec state_i(Coeffs.size());
	itpp::vec state_out_i(Coeffs.size());

#ifdef HAVE_IPP
	FIR_r->set_coeffs(Coeffs);
	FIR_i->set_coeffs(Coeffs);
#endif

	while (true) {
		itpp::cvec & BufIn = conc_buf_in->RequestReading();
		//const Samples_Type nof_smpls = BufIn.size();

		itpp::cvec & BufOut = conc_buf->RequestWriting();

		//CmplxFilter->filter(BufIn, BufOut, Coeffs);
#ifdef HAVE_IPP
		itpp::vec BufIn_r =itpp::real(BufIn);
		itpp::vec BufIn_i =itpp::imag(BufIn);

		itpp::vec BufOut_r(block_size), BufOut_i(block_size);
		FIR_r->filter(BufIn_r,BufOut_r);
		FIR_i->filter(BufIn_i,BufOut_i);
#else
		const itpp::vec BufOut_r = itpp::filter(Coeffs, 1, itpp::real(BufIn), state_r, state_out_r);
		const itpp::vec BufOut_i = itpp::filter(Coeffs, 1, itpp::imag(BufIn), state_i, state_out_i);

		state_r = state_out_r;
		state_i = state_out_i;
#endif
		BufOut = to_cvec(BufOut_r, BufOut_i);

		if (conc_buf_in->ReleaseReading()) {
			conc_buf->ReleaseWriting(true);
			snLog("snProcessorLPF: exit.");
			printSummary("snLPF");
			return;
		} else {
			// should we plot?
			if (snw->requestPlotFlag()) {
				for (Samples_Type o = 0; o < get_nof_xaxis_indices(); o++) {
					curve->update_value_X(o, static_cast<double> (o) / smpl_freq); // in s
					curve->update_value_Y(o, BufOut[o]);
				}
				snw->releasePlotFlag();
			}
			conc_buf->ReleaseWriting(false);
		}
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

	std::cout << "LPF: band[0]" << bands[0] << ", band[1]: " << bands[1] << ", band[2]: " << bands[2]
			<< ", band[3]: " << bands[3] << "\n";
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
