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
 * \file snProcessorADC.cpp
 *
 * \author Frank Schubert
 */
#include <QString>

#include "snProcessorADC.h"

#ifdef HAVE_ACML
#include "acml_mv.h"
#endif

snProcessorADC::snProcessorADC(libconfig::Setting &Settings, SNSignal pSNSignal,
		widget_factory_base *widget_factory, ConcBuf3 *_conc_buf_in) :
		snCreatingModule(pSNSignal, "snADC", _conc_buf_in), conc_buf_in(_conc_buf_in) {

	snLog("snADC: setup.");

	Sig.results_file->createGroup("snADC");
	Sig.results_file->createGroup("snADC/parameters");

	vga_enable = Settings["vga_enable"]; // variable gain amplifier
	Sig.results_file->write("snADC/parameters/vga_enable", vga_enable);

	adc_enable = Settings["adc_enable"]; // variable gain amplifier
	Sig.results_file->write("snADC/parameters/adc_enable", adc_enable);
	/*
	 if (!adc_enable && !vga_enable) {
	 throw snSimulationException("snADC: At least one of ADC or VGA must be enabled!");
	 }
	 */

	// read Intermediate Frequency
	intermed_freq = Settings["intermediate_frequency"]; // variable gain amplifier
	if (intermed_freq < 0) {
		throw snSimulationException("intermediate_frequency must be greater or equal to 0.");
	}

	if (intermed_freq == 0 && adc_enable) {
		throw snSimulationException("if adc is enabled, intermediate frequency must be greater than 0.");
	}

	if (intermed_freq >= smpl_freq / 2.0) {
		throw snSimulationException("intermediate_frequency must be smaller than sampling frequency / 2.");
	}
	//log("Intermediate frequency: " + QString::number(intermed_freq) + " Hz");

	if (adc_enable) {
		adc_bits = static_cast<unsigned int>(Settings["adc_bits"]);
		Sig.results_file->write("snADC/parameters/adc_bits", adc_bits);
		if (adc_bits < 1) {
			throw snSimulationException("snADC: adc_bits must be greater or equal to 1!");
		} else if (adc_bits == 1 && vga_enable == true) {
			throw snSimulationException(
					"snADC: you cannot use an VGA with 1 bit sampling. set vga_enable = false.");
		}
		adc_states = static_cast<int>(pow(2, adc_bits));
		adc_divisor = 2.0 / (adc_states - 1);
		adc_highest_value = static_cast<double>(adc_states - 1) / 2.0;
		adc_lowest_value = -adc_highest_value;
	}

	// setup widget
	snw = widget_factory->get_new_widget("ADC");

	if (vga_enable) {
		vga_lowest_amplification_dB = Settings["vga_lowest_amplification_dB"];
		Sig.results_file->write("snADC/parameters/vga_lowest_amplification_dB", vga_lowest_amplification_dB);

		vga_highest_amplification_dB = Settings["vga_highest_amplification_dB"];
		Sig.results_file->write("snADC/parameters/vga_highest_amplification_dB",
				vga_highest_amplification_dB);

		vga_lowest_amplification = pow(10, vga_lowest_amplification_dB / 10.0);
		Sig.results_file->write("snADC/parameters/vga_lowest_amplification", vga_lowest_amplification);

		vga_highest_amplification = pow(10, vga_highest_amplification_dB / 10.0);

		vga_ampl_curve = new snCurveReal(conc_buf->get_total_blocks(), snCurve::darkMagenta);
		snPlot *vga_ampl_plot = snw->get_new_plot_real("VGA amplification", snPlot::Linear, vga_ampl_curve);
		vga_ampl_plot->setXAxis(0.0, conc_buf->get_signal_length());
		//vga_ampl_plot->setYAxis(-1.5, 1.5);
		vga_ampl_plot->setXAxisLabel("t [s]");
		vga_ampl_plot->setYAxisLabel("A");

		vga_ampl_plot->enableGrid();
		snw->addPlot(vga_ampl_plot);

		for (Blocks_Type i = 0; i < conc_buf->get_total_blocks(); i++) {
			vga_ampl_curve->update_value_X(i, i * conc_buf->get_block_length());
			vga_ampl_curve->update_value_Y(i, 0);
		}
	}

	if (adc_enable) {
		// setup plot for ADC bins histogram
		adc_hist_curve = new snCurveReal(adc_states, snCurve::red);
		adc_hist_curve->setCurveStyle(snCurve::STICKS);
		adc_hist_curve->setCurveSymbol(snCurve::CIRCLE, snCurve::red, snCurve::MEDIUM);

		snPlot *adc_hist_plot = snw->get_new_plot_real("ADC histogramm", snPlot::Linear, adc_hist_curve);
		adc_hist_plot->setXAxis(adc_lowest_value, adc_highest_value);
		adc_hist_plot->setXAxisLabel("bin");
		adc_hist_plot->setYAxisLabel("value");
		snw->addPlot(adc_hist_plot);
	}

	// setup plot for outgoing signal
	curve = new snCurveComplex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
	if (adc_enable) {
		curve->setCurveSymbol(snCurve::CIRCLE, snCurve::blue, snCurve::MEDIUM);
		curve->setCurveStyle(snCurve::DOTS);
		curve->setCurveSymbol(snCurve::CIRCLE, snCurve::red, snCurve::MEDIUM);
		curve->setCurveStyle(snCurve::DOTS);
	}
	snPlot *plot = snw->get_new_plot_complex("real / imag", snPlot::Linear, curve);
	/*
	 if (adc_enable) {
	 plot->setYAxis(adc_lowest_value - 1, adc_highest_value + 1);
	 }
	 */
	plot->setXAxis(0.0, get_max_xaxis_value());

	plot->setXAxisLabel("t [s]");
	plot->setYAxisLabel("bit");

	snPlot *plot_magn = snw->get_new_plot_complex("FFT, Magnitude", snPlot::FFTMagnitudeDB, curve);
	snPlot *plot_angl = snw->get_new_plot_complex("FFT, Angle", snPlot::FFTAngle, curve);

	snw->addPlot(plot);
	snw->addPlot(plot_magn);
	snw->addPlot(plot_angl);

}

snProcessorADC::~snProcessorADC() {

}

void snProcessorADC::run() {
	// save amplifcation values in a vector each and write it to result file at end of simulation
	std::vector<double> log_vga_amplification(conc_buf->get_total_blocks());

	Samples_Type b = 0; // absolute sample number.

	Blocks_Type act_blk = 0;

	const double adc_fac = (static_cast<double>(adc_states) - 1.0) / 2.0;

	while (true) {
		itpp::cvec & BufIn = conc_buf_in->RequestReading();
		const Samples_Type nof_smpls = BufIn.size();

		double vga_amplification = 0.0;
		const double radiant_const = 2.0 * M_PI * intermed_freq / smpl_freq;

		if (vga_enable) {
			// find peak value of samples in current concbuf block
			const double peak = itpp::max(itpp::abs(BufIn));

			vga_amplification = 1.0 / peak;

			// limit vga gain to limits given in configuration
			if (vga_amplification < vga_lowest_amplification) {
				vga_amplification = vga_lowest_amplification;
			}
			if (vga_amplification > vga_highest_amplification) {
				vga_amplification = vga_highest_amplification;
			}

			// multiply samples in current concbuf block with vga amplification
			BufIn *= vga_amplification;
		}

		itpp::cvec & BufOut = conc_buf->RequestWriting();

		if (intermed_freq > 0.0) {
			// build radiant vector:
			itpp::vec radiants(nof_smpls);
			for (Samples_Type i = 0; i < nof_smpls; i++) {
				// double radiant = radiant_const * b;
				// Patch for some libc's buggy sin() (does not change result):
				// radiant = fmod(radiant, 2.0 * M_PI);

				// new:
				radiants[i] = fmod(radiant_const * b, 2.0 * M_PI);
				b++;
			}

			// mix up:
			// double output_value = sqrt(2.0) * (BufIn[i].real() * cos(radiant)
			// + BufIn[i].imag() * sin(radiant));
			// new:

#ifdef HAVE_ACML
			itpp::vec sin_vec(nof_smpls), cos_vec(nof_smpls);
			// void vrda_sincos (int n, double *x, double *ys, double *yc)
			vrda_sincos(nof_smpls, &radiants[0], &sin_vec[0], &cos_vec[0]);
			itpp::vec outputs = (itpp::elem_mult(itpp::real(BufIn),
							cos_vec) + itpp::elem_mult(itpp::imag(BufIn), sin_vec));
#else
			/*itpp::vec outputs = sqrt(2.0) * (itpp::elem_mult(itpp::real(BufIn),
			 itpp::cos(radiants)) + itpp::elem_mult(itpp::imag(BufIn),
			 itpp::sin(radiants)));*/
			itpp::vec outputs = itpp::elem_mult(itpp::real(BufIn), itpp::cos(radiants))
					+ itpp::elem_mult(itpp::imag(BufIn), itpp::sin(radiants));
#endif

			if (adc_enable) {
				for (Samples_Type o = 0; o < nof_smpls; o++) {
					double output_byte = floor(outputs[o] * adc_fac) + 0.5;

					if (output_byte > adc_highest_value)
						output_byte = adc_highest_value;
					if (output_byte < adc_lowest_value)
						output_byte = adc_lowest_value;

					BufOut[o] = std::complex<double>(output_byte / adc_highest_value, 0.0);
				}
			} else {
				// if ADC is disabled, we output the 'continuous' value:
				BufOut = to_cvec(outputs);
			}
		} else { // no upconversion
			BufOut = BufIn;
		}

		// should we plot?
		if (snw->requestPlotFlag()) {
			if (adc_enable) {
				// plot ADC bins histogram:
				std::vector<int> bins(adc_states);
				for (Samples_Type o = 0; o < nof_smpls; o++) {
					size_t bin = floor(
							(BufOut[o].real() * adc_highest_value * 2.0
									+ (static_cast<double>(adc_states) - 1.0)) / 2.0);
					if (bin >= bins.size()) {
						std::cout << "snADC error: bin >= bins.size()! adc_states: " << adc_states
								<< ", bin: " << bin << "\n";
						std::cout.flush();
						std::exit(1);
					}
					bins.at(bin)++;}
				for (int o = 0; o < adc_states; o++) {
					adc_hist_curve->update_value_X(o, adc_lowest_value + o);
					adc_hist_curve->update_value_Y(o, bins.at(o));
				}
			}
			// plot outgoing signal
			for (Samples_Type o = 0; o < get_nof_xaxis_indices(); o++) {
				curve->update_value_X(o, static_cast<double>(o) / smpl_freq); // in s
				curve->update_value_Y(o, BufOut[o]);
			}
			snw->releasePlotFlag();
		}
		if (vga_enable) {
			// save variable in vector:
			log_vga_amplification.at(act_blk) = vga_amplification;

			// plot vga amplification:
			snw->pushData();
			vga_ampl_curve->update_value_X(act_blk, act_blk * conc_buf->get_block_length());
			vga_ampl_curve->update_value_Y(act_blk, vga_amplification);
			snw->releasePlotFlag();
		}
		if (conc_buf_in->ReleaseReading()) {
			conc_buf->ReleaseWriting(true);
			// write logged variables to file:
			Sig.results_file->createGroup("/snADC/output");
			if (vga_enable) {
				Sig.results_file->write("/snADC/output/vga_amplification", log_vga_amplification, act_blk);
			}
			requestStop();
			snLog("snADC: exit.");
			printSummary("snProcessorADC");
			return;
		} else {
			conc_buf->ReleaseWriting(false);
		}
		act_blk++;
	}
}
