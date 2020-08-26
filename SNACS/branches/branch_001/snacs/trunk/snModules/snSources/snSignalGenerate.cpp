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

snSignalGenerate::snSignalGenerate(libconfig::Setting &Settings, SNSignal pSNSignal,
		widget_factory_base *widget_factory) :
		snCreatingModule(pSNSignal, "snSignalGenerate", Settings["sampling_frequency"],
				Settings["signal_length"], Settings["block_length"], Settings["number_of_blocks_per_buffer"]) {

	snLog("SignalGenerate: setup.");
	Ai = 0.0;
	Aq = 0.0;

	gnss_signal = new snGNSSSignal(Settings["signal"], smpl_freq, signal_length);
	inphase_enabled = gnss_signal->inphase_enabled();
	quadrature_enabled = gnss_signal->quadrature_enabled();
	if (inphase_enabled) {
		inphase_modulation = gnss_signal->get_inphase_modulation();
		Ci_dB = Settings["signal"]["inphase"]["C_dB"]; // Signal power inphase in dBW
		Ci_lin = pow(10.0, Ci_dB / 10.0);
		Ai = sqrt(2.0 * Ci_lin); // Signal amplitude
	}
	if (quadrature_enabled) {
		quadrature_modulation = gnss_signal->get_quadrature_modulation();
		Cq_dB = Settings["signal"]["quadrature"]["C_dB"]; // Signal power quadrature in dBW
		Cq_lin = pow(10.0, Cq_dB / 10.0);
		Aq = sqrt(2.0 * Cq_lin); // Signal amplitude
	}

	start_sample = static_cast<double>(Settings["start"]) * smpl_freq;

	// setup widget
	snw = widget_factory->get_new_widget("Signal Generator");

	curve = new snCurveComplex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
	snPlot *plot_complex = snw->get_new_plot_complex("real / imag", snPlot::Linear, curve);

	plot_complex->setXAxis(0.0, get_max_xaxis_value());
	plot_complex->setYAxis(-2.5, 2.5);
	plot_complex->setXAxisLabel("t [s]");
	plot_complex->setYAxisLabel("A");
	snw->addPlot(plot_complex);

	snPlot *plot_fft_mag = snw->get_new_plot_complex("FFT, Magnitude", snPlot::FFTMagnitudeDB, curve);
	plot_fft_mag->setXAxisLabel("f [Hz]");
	plot_fft_mag->setYAxisLabel("A");
//	snw->addPlot(plot_fft_mag);

	snPlot *plot_fft_angle = snw->get_new_plot_complex("FFT, Angle", snPlot::FFTAngle, curve);
	plot_fft_angle->setXAxisLabel("f [Hz]");
	plot_fft_angle->setYAxisLabel("A");
//	snw->addPlot(plot_fft_angle);
}

snSignalGenerate::~snSignalGenerate() {
	delete gnss_signal;
}

void snSignalGenerate::run() {
	snLog("SignalGenerate: start.", snModule::info);

	Samples_Type absolute_sample = 0;
	const Samples_Type total_samples = conc_buf->get_total_samples();

	while (true) {
		itpp::cvec& Buf = conc_buf->RequestWriting();

		for (Samples_Type o = 0; o < block_size; o++) {
			if (absolute_sample >= start_sample) {
				// amplitude in baseband with power P is A = sqrt(2.0 * P)
				// here: P = 1
				const Samples_Type sample_number = absolute_sample - start_sample;
				if (inphase_enabled)
					Buf[o].real() = Ai
							* inphase_modulation->get_value_for_absolute_sample_number(sample_number);
				else
					Buf[o].real() = 0.0;

				if (quadrature_enabled)
					Buf[o].imag() = Aq
							* quadrature_modulation->get_value_for_absolute_sample_number(sample_number);
				else
					Buf[o].imag() = 0.0;
			} else {
				Buf[o] = std::complex<double>(0.0, 0.0);
			}
			if ((absolute_sample + 1 == total_samples) || !isActive()) {
				// absolute_sample + 1 because absolute_sample starts at 0 and total_samples gives the absolut number starting at 1
				conc_buf->ReleaseWriting(true);
				requestStop();
				snLog("SignalGenerate: exit.");
				printSummary("snSignalGenerate");
				return;
			}
			absolute_sample++;
		}

		// should we write samples to a txt file?
		const Blocks_Type current_block = conc_buf->get_current_producer_block();
		if (do_write_txt_signals(current_block)) {
			txt_signal txt_file(Sig, "signal-generator.txt", current_block);
			// build time vector
			itpp::vec ts(Sig.nof_txt_samples);
			for (size_t k = 0; k < Sig.nof_txt_samples; k++)
				ts(k) = k * gnss_signal->get_smpl_interval();
			// real vector:
			itpp::vec real_vec(Sig.nof_txt_samples);
			for (size_t k = 0; k < Sig.nof_txt_samples; k++)
				real_vec(k) = Buf[k].real();
			txt_file.write_txt_vectors(ts, real_vec);
		}

		// should we plot?
		if (snw->requestPlotFlag()) {
			for (Samples_Type o = 0; o < get_nof_xaxis_indices(); o++) {
				curve->update_value_X(o, static_cast<double>(o) / smpl_freq); // in s
				curve->update_value_Y(o, Buf[o]);
			}
			snw->releasePlotFlag();
		}
		conc_buf->ReleaseWriting(false);
	}
}
