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

snProcessorADC::snProcessorADC(libconfig::Setting &Settings, SNSignal pSNSignal, snWidget *psnw,
		ConcBuf *pConcBuf1, ConcBuf *pConcBufOut) :
	snBlock(pSNSignal), snw(psnw), ConcBuf1(pConcBuf1), ConcBufOut(pConcBufOut)

{
	snLog("snADC: setup.");

	Sig.results_file->createGroup("snADC");
	Sig.results_file->createGroup("snADC/parameters");

	vga_enable = Settings["vga_enable"]; // variable gain amplifier
	Sig.results_file->write("snADC/parameters/vga_enable", vga_enable);

	adc_enable = Settings["adc_enable"]; // variable gain amplifier
	Sig.results_file->write("snADC/parameters/adc_enable", adc_enable);

	// read Intermediate Frequency
	intermed_freq = Settings["intermediate_frequency"]; // variable gain amplifier
	if (intermed_freq < 0) {
		throw snSimulationException("intermediate_frequency must be greater than 0.");
	}

	if (intermed_freq >= Sig.SmplFreq / 2.0) {
		throw snSimulationException(
				"intermediate_frequency must be smaller than SamplingFrequency/2.");
	}
	//log("Intermediate frequency: " + QString::number(intermed_freq) + " Hz");

	if (adc_enable) {
		adc_bits = static_cast<unsigned int> (Settings["adc_bits"]);
		Sig.results_file->write("snADC/parameters/adc_bits", adc_bits);
		if (adc_bits < 1) {
			throw snSimulationException("snADC: adc_bits must be greater or equal to 1!");
		} else if (adc_bits == 1 && vga_enable == true) {
			throw snSimulationException(
					"snADC: you cannot use an VGA with 1 bit sampling. set vga_enable = false.");
		}
		adc_states = static_cast<int> (pow(2, adc_bits));
		adc_divisor = 2.0 / adc_states;
		adc_highest_value = 1.0 / adc_divisor - 0.5;
		adc_lowest_value = -1.0 / adc_divisor + 0.5;
	}

	if (vga_enable) {
		vga_lowest_amplification_dB = Settings["vga_lowest_amplification_dB"];
		Sig.results_file->write("snADC/parameters/vga_lowest_amplification_dB",
				vga_lowest_amplification_dB);

		vga_highest_amplification_dB = Settings["vga_highest_amplification_dB"];
		Sig.results_file->write("snADC/parameters/vga_highest_amplification_dB",
				vga_highest_amplification_dB);

		vga_lowest_amplification = pow(10, vga_lowest_amplification_dB / 10.0);
		Sig.results_file->write("snADC/parameters/vga_lowest_amplification",
				vga_lowest_amplification);

		vga_highest_amplification = pow(10, vga_highest_amplification_dB / 10.0);

		vga_ampl_curve = new snCurveReal(max_blocks, Qt::darkMagenta);
		snPlotReal *agc_ampl_plot = new snPlotReal("VGA amplification", snPlot::Linear,
				vga_ampl_curve);
		agc_ampl_plot->setXAxis(0.0, max_simulation_length);
		agc_ampl_plot->setYAxis(-1.5, 1.5);
		agc_ampl_plot->setXAxisLabel("t [s]");
		agc_ampl_plot->setYAxisLabel("A");

		agc_ampl_plot->enableGrid();
		snw->addPlot(agc_ampl_plot);

		for (Blocks_Type i = 0; i < max_blocks; i++) {
			vga_ampl_curve->X[i] = i * get_time_per_block();
			vga_ampl_curve->Y[i] = 0;
		}
	}

	if (adc_enable) {
		adc_hist_curve = new snCurveReal(adc_states, Qt::red);
		adc_hist_curve->setCurveStyle(snCurve::CURVEI, QwtPlotCurve::Sticks);
		adc_hist_curve->setCurveSymbol(snCurve::CURVEI, QwtSymbol(QwtSymbol::Ellipse, QBrush(
				Qt::red), QPen(Qt::red), QSize(4, 4)));

		snPlotReal *adc_hist_plot =
				new snPlotReal("ADC histogramm", snPlot::Linear, adc_hist_curve);
		adc_hist_plot->setXAxis(adc_lowest_value, adc_highest_value);
		adc_hist_plot->setXAxisLabel("bin");
		adc_hist_plot->setYAxisLabel("value");
		snw->addPlot(adc_hist_plot);

		curve = new snCurveComplex(plot_x_max, Qt::blue, Qt::red);
		curve->setCurveSymbol(snCurve::CURVEI, QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::blue),
				QPen(Qt::blue), QSize(3, 3)));
		curve->setCurveStyle(snCurve::CURVEI, QwtPlotCurve::Dots);
		curve->setCurveSymbol(snCurve::CURVEQ, QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::red), QPen(
				Qt::red), QSize(3, 3)));
		curve->setCurveStyle(snCurve::CURVEQ, QwtPlotCurve::Dots);
		snPlotComplexReal *plot = new snPlotComplexReal("real / imag", snPlot::Linear, curve);
		plot->setYAxis(adc_lowest_value - 1, adc_highest_value + 1);
		plot->setXAxis(0.0, Sig.plot_buffer_length);
		plot->setXAxisLabel("t [s]");
		plot->setYAxisLabel("bit");

		snPlotMagn *plot_magn = new snPlotMagn("magn", snPlot::Log20, curve, Sig.SmplFreq);
		snPlotAngl *plot_angl = new snPlotAngl("angl", snPlot::Linear, curve, Sig.SmplFreq);

		snw->addPlot(plot);
		snw->addPlot(plot_magn);
		snw->addPlot(plot_angl);
	}
}

snProcessorADC::~snProcessorADC() {

}

void snProcessorADC::run() {

	// save amplifcation values in a vector each and write it to result file at end of simulation
	std::vector<double> log_vga_amplification(getTotalBlks());

	while (isActive()) {
		Samples_Type nToRead;
		BufIn = ConcBuf1->RequestReading(&nToRead);

		double vga_amplification = 0.0;
		if (vga_enable) {
			// find peak value of samples in current concbuf block
			double peak = 0.0;
			for (Samples_Type i = 0; i < nToRead; i++) {
				double abs_amp = std::abs(BufIn[i]);
				if (abs_amp > peak) {
					peak = abs_amp;
				}
			}

			vga_amplification = 1.0 / peak;

			// limit vga gain to limits given in configuration
			if (vga_amplification < vga_lowest_amplification) {
				vga_amplification = vga_lowest_amplification;
			}
			if (vga_amplification > vga_highest_amplification) {
				vga_amplification = vga_highest_amplification;
			}

			// multiply samples in current concbuf block with vga amplification
			for (Samples_Type i = 0; i < nToRead; i++) {
				BufIn[i] = BufIn[i] * vga_amplification;
			}
		}

		BufOut = ConcBufOut->RequestWriting();

		const Samples_Type b = getBlks() * Sig.BufSize; // get absolute sample number.

		const double radiant_const = 2.0 * M_PI * intermed_freq * Sig.Ts;

		for (Samples_Type i = 0; i < nToRead; i++) {

			double output_value = 0.0;
			if (intermed_freq > 0.0) {
				// upconversion to IF:
				const double sample_number = b + i;
				double radiant = radiant_const * sample_number;

				// Patch for some libc's buggy sin() (does not change result):
				radiant = fmod(radiant, 2.0 * M_PI);

				// mix up:
				output_value = sqrt(2.0) * (BufIn[i].real() * cos(radiant) + BufIn[i].imag() * sin(
						radiant));
			} else {
				output_value = BufIn[i].real();
			}

			if (adc_enable) {
				double output_byte = ceil(output_value / adc_divisor) - 0.5;

				if (output_byte > adc_highest_value)
					output_byte = adc_highest_value;
				if (output_byte < adc_lowest_value)
					output_byte = adc_lowest_value;

				BufOut[i] = std::complex<double>(output_byte, 0.0);
			} else {
				// if ADC is disabled, we output the 'continuous' value:
				BufOut[i] = std::complex<double>(output_value, 0.0);
			}
		}

		// should we plot?
		if (adc_enable) {
			if (snw->requestPlotFlag()) {
				// plot ADC bins histogram:
				std::vector<int> bins(static_cast<int> (floor(adc_states)));
				for (Samples_Type o = 0; o < Sig.BufSize; o++) {
					int bin = static_cast<int> ((BufOut[o].real() * 2 + (adc_states - 1)) / 2);
					bins.at(bin)++;
				}
				for (int o = 0; o < adc_states; o++) {
					adc_hist_curve->X.at(o) = adc_lowest_value + o;
					adc_hist_curve->Y.at(o) = bins.at(o);
				}
				for (Samples_Type o = 0; o < plot_x_max; o++) {
					curve->X.at(o) = static_cast<double> (o) / Sig.SmplFreq; // in s
					curve->Y.at(o) = BufOut[o].real();
				}
				snw->releasePlotFlag();
			}
		}
		if (vga_enable) {
			const Blocks_Type act_blk = getBlks();
			//std::cout << "act_blk :" << act_blk << "\n";
			// save variable in vector:
			log_vga_amplification.at(act_blk) = vga_amplification;

			// plot vga amplification:
			snw->pushData();
			vga_ampl_curve->X.at(act_blk) = get_blks_simulation_time();
			vga_ampl_curve->Y.at(act_blk) = vga_amplification;
			snw->releasePlotFlag();
		}
		if (ConcBuf1->ReleaseReading()) {
			ConcBufOut->ReleaseWriting(nToRead, 1);

			// write logged variables to file:
			Sig.results_file->createGroup("/snADC/output");
			Sig.results_file->write("/snADC/output/vga_amplification", log_vga_amplification,
					getBlks());
			requestStop();
			snLog("snADC: exit.");
			printSummary("snProcessorADC");
			return;
		} else {
			ConcBufOut->ReleaseWriting(nToRead, 0);
		}

		increaseBlks();
	}
}
