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
 * \file snProcessorChannel.cpp
 *
 * \author Frank Schubert
 */

#include "snProcessorChannel.h"
#include "../snSimulation/snCmplxFIRFilter.h"

snProcessorChannel::snProcessorChannel(libconfig::Setting &Settings, SNSignal pSNSignal,
		ProcessingParameters &ProcSig, widget_factory_base *widget_factory, ConcBuf3* _circbuf_in) :
		snCreatingModule(pSNSignal, "snChannel", _circbuf_in), circbuf_in(_circbuf_in) {

	snLog("snChannel: channel cir interpolation setup.");

	Sig.results_file->createGroup("snChannel");
	Sig.results_file->createGroup("snChannel/parameters");

	//TODO
	/*
	 if (!isInt(Sig.Len * smpl_freq)) {
	 log("abort.");
	 throw snSimulationException("(Sig.Len * smpl_freq) must be integer.");
	 }
	 if (!isInt(val * smpl_freq)) {
	 log("abort.");
	 throw snSimulationException("(ConcBufSize * sampling frequency) must be integer.");
	 }


	 */
	std::string interpolation_type_str = static_cast<const char *>(Settings["interpolation_type"]);

	if (interpolation_type_str == "SINC") {
		interpolation_type = SINC;
		Sig.results_file->write("snChannel/parameters/interpolation_type", "SINC");
	} else if (interpolation_type_str == "RRC") {
		interpolation_type = RRC;
		Sig.results_file->write("snChannel/parameters/interpolation_type", "RRC");
		rrc_rolloff_factor = Settings["rrc_rolloff_factor"];
		Sig.results_file->write("snChannel/parameters/rrc_rolloff_factor", rrc_rolloff_factor);
	} else {
		throw snSimulationException("Unknown interpolation type.");
	}

	std::string filename = static_cast<const char *>(Settings["file"]);
	Sig.results_file->write("snChannel/parameters/filename", filename);

	write_cdx_file = Settings["write_cdx_file"];

	//interpolation_bandwidth = Settings["interpolation_bandwidth"];
	interpolation_bandwidth = smpl_freq / 2.0;
	Sig.results_file->write("snChannel/parameters/interpolation_bandwidth", interpolation_bandwidth);

	file = new ReadContinuousDelayCDXFile(filename);

	if (file->getC0() != Sig.c0) {
		throw snSimulationException("snChannel: c0 of simulation and c0 of CIR file differ!");
	}

	cir_rate = file->get_cir_rate();
	nof_cirs = file->get_nof_cirs();
	const double transmitter_frequency = file->get_transmitter_frequency();

	snLog(
			"snChannel: CIR amount in file: " + QString::number(nof_cirs) + ", CIR rate: "
					+ QString::number(cir_rate), snModule::debug);

	// are there enough CIRs for signal length?
	if ((static_cast<double>(nof_cirs) / cir_rate) < signal_length) {
		throw snSimulationException("snChannel: Not enought CIRs in file for simulation signal length!");
	}

	// if there are more CIRs in file than simulation length, set CIR amount to simulation length
	if ((static_cast<double>(nof_cirs) / cir_rate) > signal_length) {
		nof_cirs = static_cast<Blocks_Type>(floor(signal_length * cir_rate));
	}

	snLog("snChannel: will use " + QString::number(nof_cirs) + " CIRs from file.", snModule::debug);

	size_t MaxEchoes = 0;

	// scan through CIR file to find max and minimum delay of all MP components
	// and the maximum number of echoes per CIR snapshot.
	// but only for length of simulation (nof_cirs)
	find_max_delay(file, nof_cirs, MaxDelay, MaxEchoes);
	find_min_delay(file, nof_cirs, MinDelay, MaxEchoes);
	if (MinDelay < 0) {
		throw snSimulationException("snChannel: one of the delays of components in the CIR file is below 0!");
	}

	snLog("snProcessorChannel: CIR file: min delay: " + QString::number(MinDelay), snModule::debug);
	snLog("snProcessorChannel: CIR file: max delay: " + QString::number(MaxDelay), snModule::debug);

	// compute length of Dolph-Chebyshev window:
	stop_band_attenuation_dB = Settings["stop_band_attenuation_dB"];
	// calculate ripple ratio
	r = pow(10.0, stop_band_attenuation_dB / 20.0);
	const double theta_s = M_PI / 2.0; //1.0; // stop band edge

	M = ceil(1.0 / 2.0 * acosh(1.0 / r) / acosh(1.0 / cos(theta_s / 2.0)));
	N = 2 * M + 1; // number of filter coefficients

	// store Dolph window function to result file:
	Sig.results_file->createGroup("snChannel/dolph-window");
	Sig.results_file->write_vec("snChannel/dolph-window/window-samples",
			chebyshev_t(M, stop_band_attenuation_dB));

	// store a shifted Dolph window function to result file:
	itpp::vec t_ax(N);
	itpp::vec shifted_dc(N);
	//const double lag = 0.5 * smpl_ts;
	for (size_t k = 0; k < N; k++) {
		t_ax[k] = (-static_cast<double>(M) + static_cast<double>(k)) * smpl_ts;
		//std::cout << "k: " << k << ", t_ax[k]: " << t_ax[k] << "\n";
		shifted_dc[k] = chebyshev_t_shifted(M, stop_band_attenuation_dB, smpl_ts, smpl_ts / 2.0, t_ax[k]);
	}

	Sig.results_file->write_vec("snChannel/dolph-window/shifted-window-samples", shifted_dc);

	// delay offset to add to all delays to obtain a causal filter:
	tau_off = M * smpl_ts;
	ProcSig.ProcessingDelay += tau_off;

	nof_fir_coeffs = static_cast<int>(ceil((MaxDelay + 2.0 * tau_off) * smpl_freq));

	std::cout << "nof_fir_coeffs: " << nof_fir_coeffs << "\n";

	snLog(
			"snProcessorChannel: number of FIR coeffs " + QString::number(nof_fir_coeffs) + " = "
					+ QString::number(nof_fir_coeffs * smpl_ts) + " s", snModule::debug);

	// check if CIRrate is equal to block_size:
	if (static_cast<Samples_Type>((1.0 / cir_rate) * smpl_freq) != block_size) {
		throw snSimulationException(
				QString(
						"snChannel: CIR rate (" + QString::number(cir_rate) + "Hz = "
								+ QString::number(((1.0 / cir_rate) * smpl_freq))
								+ " samples) does not match ConcBuf size (" + QString::number(block_size)
								+ ")! Adjust ConcBuf size to be same length as 1 / cir rate.").toStdString());
	}

	if (write_cdx_file) {
		// open CDX output file for interpolation results:
		cdx_out = new WriteDiscreteDelayCDXFile(Sig.results_directory + "/interpolated-channel.h5",
				file->getC0(), file->get_cir_rate(), nof_fir_coeffs, smpl_freq, transmitter_frequency);
	}

	// setup widgets
	cir_snw = widget_factory->get_new_widget("channel impulse response");
	fir_snw = widget_factory->get_new_widget("FIR coefficients");
	out_snw = widget_factory->get_new_widget("convoluted signal");

	// 1st snWidget for model CIR input:
	cir_curve = new snCurveComplex(MaxEchoes, snCurve::blue, snCurve::red);
	cir_curve->setCurveSymbol(snCurve::CIRCLE, snCurve::blue, snCurve::MEDIUM);
	cir_curve->setCurveStyle(snCurve::STICKS);

	snPlot *cir_plot_magn = cir_snw->get_new_plot_complex("Magnitude", snPlot::Log20, cir_curve);
	cir_plot_magn->setYAxis(-50, 5);
	cir_plot_magn->setXAxis(0, nof_fir_coeffs * (smpl_ts));
	snPlot *cir_plot_angl = cir_snw->get_new_plot_complex("Angle", snPlot::Angle, cir_curve);
	cir_plot_angl->setYAxis(-3.5, 3.5);
	cir_plot_angl->setXAxis(0, nof_fir_coeffs * (smpl_ts));

	cir_snw->addPlot(cir_plot_magn);
	cir_snw->addPlot(cir_plot_angl);

	// 2nd snWidget for interpolated FIR coefficients:
	fir_curve = new snCurveComplex(nof_fir_coeffs, snCurve::blue, snCurve::red);
	fir_curve->setCurveSymbol(snCurve::CIRCLE, snCurve::blue, snCurve::SMALL);
	fir_curve->setCurveStyle(snCurve::LINES);

	snPlot *fir_plot_magn = fir_snw->get_new_plot_complex("magnitude", snPlot::Log20, fir_curve);
//			new snPlotComplexMagn("magnitude", snPlot::Log20, fir_curve);
	fir_plot_magn->setXAxis(0, nof_fir_coeffs * smpl_ts);
	fir_plot_magn->setYAxis(-50, 5);
	snPlot *fir_plot_angl = fir_snw->get_new_plot_complex("angle", snPlot::Angle, fir_curve);
//			new snPlotComplexAngle("angle", snPlot::Linear, fir_curve);
	fir_plot_angl->setXAxis(0, nof_fir_coeffs * smpl_ts);
	fir_plot_angl->setYAxis(-3.5, 3.5);
	snPlot *plot_magn = fir_snw->get_new_plot_complex("FFT, Magnitude", snPlot::FFTMagnitudeDB, fir_curve);
	plot_magn->setYAxis(-50, 5);
	snPlot *plot_angl = fir_snw->get_new_plot_complex("FFT, Angle", snPlot::FFTAngle, fir_curve);
	plot_angl->setYAxis(-3.5, 3.5);

	fir_snw->addPlot(fir_plot_magn);
	fir_snw->addPlot(fir_plot_angl);
	fir_snw->addPlot(plot_magn);
	fir_snw->addPlot(plot_angl);

	// 3rd snWidget for model output of convolution:
	out_curve = new snCurveComplex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
	snPlot *out_plot = out_snw->get_new_plot_complex("angle", snPlot::Linear, out_curve);
//			new snPlot("double / imag", snPlot::Linear, out_curve);
	snPlot *out_plot_magn = out_snw->get_new_plot_complex("FFT, Magnitude", snPlot::FFTMagnitudeDB, out_curve);
	snPlot *out_plot_angl = out_snw->get_new_plot_complex("FFT, Angle", snPlot::FFTAngle, out_curve);
	out_plot->setXAxis(0.0, get_max_xaxis_value());
	out_plot->setYAxis(-8, 8);
	out_plot->setXAxisLabel("t [s]");
	out_plot->setYAxisLabel("A");

	out_snw->addPlot(out_plot);
	out_snw->addPlot(out_plot_magn);
	out_snw->addPlot(out_plot_angl);

	snLog("snChannel: channel cir interpolation setup complete.");
}

snProcessorChannel::~snProcessorChannel() {
	delete file;
}

void snProcessorChannel::run() {

	// for FIR filtering:
	itpp::cvec Coeffs(nof_fir_coeffs);
	itpp::cvec state(Coeffs.size());
	itpp::cvec state_out(Coeffs.size());

	Blocks_Type curNumber = 0;
	snCmplxFIRFilter *cmpl_filter = new snCmplxFIRFilter(block_size, Coeffs.size() - 1);

	while (true) {
		// get next CIR:
		cir_struct act_cir = file->get_cir(0, curNumber);

		snLog(
				"snInterpolator: reference delay: " + QString::number(act_cir.ref_delay) + ", reference distance: "
						+ QString::number(act_cir.ref_delay / Sig.c0), snModule::debug);

		for (size_t o = 0; o < act_cir.components.size(); o++) // for all echos
				{
			act_cir.components.at(o).delay += tau_off;
		}

		// interpolate the read CIR coefficients to FIR coefficients
		Coeffs.zeros();

		if (interpolation_type == SINC) {
			// interpolate FIR coefficients with sinc interpolation. result to BufFIR:
			const double Om = 2.0 * M_PI * (interpolation_bandwidth);
			for (size_t o = 0; o < act_cir.components.size(); o++) // for all echos
					{
				for (int k = 0; k < nof_fir_coeffs; k++) // for all time steps
						{
					// calculate Dolph window

					const double w_dc = chebyshev_t_shifted(M, stop_band_attenuation_dB, smpl_ts,
							act_cir.components.at(o).delay, k * smpl_ts);

					const double fac = Om * (k * smpl_ts - act_cir.components.at(o).delay);
					if (fac == 0) // sin(0)/0 = 1;
							{
						Coeffs[k] += act_cir.components.at(o).weight * 1.0 * w_dc;
					} else {
						Coeffs[k] += act_cir.components.at(o).weight * (sin(fac) / fac) * w_dc;
					}
				}
			}
		} else if (interpolation_type == RRC) {
			for (size_t o = 0; o < act_cir.components.size(); o++) // for all echos
					{
				//				std::cout << "act_cir.echoes.at(o).weight: "
				//						<< abs(act_cir.echoes.at(o).weight) << "\n";
				for (int k = 0; k < nof_fir_coeffs; k++) // for all time steps
						{
					const double t = k * smpl_ts - act_cir.components.at(o).delay;
					Coeffs[k] +=
							act_cir.components.at(o).weight
									* (sin(M_PI * t * interpolation_bandwidth)
											/ (M_PI * t * interpolation_bandwidth))
									* cos(M_PI * rrc_rolloff_factor * t * interpolation_bandwidth)
									/ (1
											- (4 * pow(rrc_rolloff_factor, 2) * t * t
													* pow(interpolation_bandwidth, 2)));
				}
			}
		}
		// interpolation of FIR coefficients complete

		if (write_cdx_file) {
			// append interpolated impuls response to link 0:
			// append_cir_snapshot() expects std::vector<cdouble>, convert the itpp::cvec:
			vector<cdouble> Coeffs_vec(Coeffs.size());
			for (int k = 0; k < Coeffs.size(); k++)
				Coeffs_vec.at(k) = Coeffs(k);
			cdx_out->append_cir_snapshot(0, Coeffs_vec, act_cir.ref_delay + tau_off);
		}

		itpp::cvec & BufIn = circbuf_in->RequestReading();
		itpp::cvec & BufOut = conc_buf->RequestWriting();
		// do the convolution:
		//		BufOut = itpp::filter(Coeffs, 1, BufIn, state, state_out);
		//		state = state_out;

		cmpl_filter->filter(BufIn, BufOut, Coeffs);

		if (circbuf_in->ReleaseReading()) {
			conc_buf->ReleaseWriting(1);
			if (write_cdx_file)
				delete cdx_out;
			snLog("snChannel: exit.");
			printSummary("snProcessorChannel");
			return;
		} else {
			// should we plot?
			if (out_snw->requestPlotFlag()) {
				for (Samples_Type o = 0; o < get_nof_xaxis_indices(); o++) {
					out_curve->update_value_X(o, static_cast<double>(o) / smpl_freq); // in s
					out_curve->update_value_Y(o, BufOut[o]);
				}
				out_snw->releasePlotFlag();
			}

			conc_buf->ReleaseWriting(0);

			// should we plot?
			if (cir_snw->requestPlotFlag()) {
				for (size_t o = 0; o < act_cir.components.size(); o++) {
					cir_curve->update_value_X(o, act_cir.components.at(o).delay);
					cir_curve->update_value_Y(o, act_cir.components.at(o).weight);
				}
				cir_curve->setPointsToPlot(act_cir.components.size());
				cir_curve->setPointsToPlot(act_cir.components.size());
				cir_snw->releasePlotFlag();
			}
			if (fir_snw->requestPlotFlag()) {
				for (int o = 0; o < nof_fir_coeffs; o++) {
					fir_curve->update_value_X(o, o * smpl_ts);
					fir_curve->update_value_Y(o, Coeffs[o]);
				}
				fir_snw->releasePlotFlag();
			}

			// should we write samples to a txt file?
			if (do_write_txt_signals(curNumber)) {
				txt_signal txt_file(Sig, "channel-continuous.txt", curNumber);
				// build time vector
				itpp::vec ts(act_cir.components.size());
				itpp::vec weight(act_cir.components.size());
				for (size_t k = 0; k < act_cir.components.size(); k++) {
					ts(k) = act_cir.components.at(k).delay;
					weight(k) = abs(act_cir.components.at(k).weight);
				}
				// real vector:
				txt_file.write_txt_vectors(ts, weight);
			}

		}
		curNumber++;
	}
}

void snProcessorChannel::find_max_delay(ReadContinuousDelayCDXFile *file, const size_t &nof_cirs,
		double &MaxDelay, size_t &MaxEchoes) {
	MaxDelay = 0.0;
	MaxEchoes = 0;

	for (Blocks_Type i = 0; i < nof_cirs; i++) {
		cir_struct act_cir = file->get_cir(0, i); // CIRs in file start with number 1
		// read Echos
		for (size_t o = 0; o < act_cir.components.size(); o++) {
			if (act_cir.components.at(o).delay > MaxDelay) {
				MaxDelay = act_cir.components.at(o).delay;
			}
		}

		if (act_cir.components.size() > MaxEchoes) {
			MaxEchoes = act_cir.components.size();
		}
	}
}

void snProcessorChannel::find_min_delay(ReadContinuousDelayCDXFile *file, const size_t &nof_cirs,
		double &MinDelay, size_t &MaxEchoes) {
	MinDelay = 0.0;
	MaxEchoes = 0;

	for (Blocks_Type i = 0; i < nof_cirs; i++) {
		cir_struct act_cir = file->get_cir(0, i); // CIRs in file start with number 1
		// read Echos
		for (size_t o = 0; o < act_cir.components.size(); o++) {
			if (act_cir.components.at(o).delay < MinDelay) {
				MinDelay = act_cir.components.at(o).delay;
			}
		}

		if (act_cir.components.size() > MaxEchoes) {
			MaxEchoes = act_cir.components.size();
		}
	}
}
// Chebyshev polynomials
double snProcessorChannel::T(double n, double x) {
	if (fabs(x) <= 1) {
		return cos(n * acos(x));
	} else {
		return cosh(n * acosh(x));
	}
}

// Chebyshev window in time domain:
itpp::vec snProcessorChannel::chebyshev_t(const size_t M, const double at) {
	const size_t N = 2 * M + 1; // filter order
	//  write("filter order for dolph window, N, is ", N);

	// at = r in Lynch paper
	double r = pow(10, at / 20);

	// x_0 in Lynch paper, beta for octave code
	double x_0 = cosh(1 / (2.0 * M) * acosh(1 / r));
	//  write("x_0: ", x_0);

	if (x_0 <= 1) {
		std::cerr << "\nerror: x_0 has to be greater than 1 but it is " << x_0 << "\n";
		std::exit(1);
	}

	itpp::vec wn(N);

	// calc coefficients:
	for (int n = -M; n <= 0; ++n) {
		// calc w_n according to (3) in Lynch paper
		double sum = 0;
		for (size_t m = 1; m <= M; ++m) {
			const double theta_m = (2 * M_PI * m) / N;
			const double theta_n = (2 * M_PI * n) / N;
			sum = sum + T(2 * M, x_0 * cos(theta_m / 2)) * cos(m * theta_n);
		}

		wn[n + M] = (1.0 / N) * (1.0 + 2.0 * r * sum);

		// w_-n = w_n:
		wn[-n + M] = wn[n + M];
	}

	// normalize to coefficient in the center:
	double scale_fact = wn[M];
	for (int n = 0; n < wn.length(); ++n) {
		wn[n] /= scale_fact;
	}

	return wn;
}

//
double snProcessorChannel::wc(const size_t M, const double at, const double Ts, const double t) {
	itpp::vec wd = chebyshev_t(M, at);

	//std::cout << "wd: " << wd << "\n";

	double sum = 0.0;
	for (int k = -M; k <= static_cast<signed int>(M); ++k) {
		sum += wd[k + M] * sinc(t / Ts - static_cast<double>(k));
	}
	return sum;
}

// Chebyshev window in time domain, center shifted to t_0, truncated, at time t
double snProcessorChannel::chebyshev_t_shifted(const size_t M, const double at, const double Ts,
		const double t_0, const double t) {
	//std::cout << "Ts: " << Ts << ", t_0: " << t_0 << ", t: " << t << "\n";
	if ((t < (t_0 - M * Ts)) || (t > (t_0 + M * Ts))) {
		return 0;
	} else {
		return wc(M, at, Ts, t - t_0);
	}
}
