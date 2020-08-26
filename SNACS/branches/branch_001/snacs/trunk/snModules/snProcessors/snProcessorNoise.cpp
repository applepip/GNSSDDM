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

#include <boost/random/variate_generator.hpp>
#include <boost/random/normal_distribution.hpp>
#include <ctime> //std::time
#include "snProcessorNoise.h"

snProcessorNoise::snProcessorNoise(libconfig::Setting &Settings, SNSignal pSNSignal,
		widget_factory_base *widget_factory, ConcBuf3 *_circ_buf_in) :
		snCreatingModule(pSNSignal, "snProcessorNoise", _circ_buf_in), circ_buf_in(_circ_buf_in) {

	snLog("snNoise: setup.");

	Sig.results_file->createGroup("snNoise");
	Sig.results_file->createGroup("snNoise/parameters");

	n0_dB = Settings["N0_dB"];
	n0_lin = pow(10.0, (n0_dB / 10.0));

	Sig.results_file->write("snNoise/parameters/n0_dB", n0_dB);

	start_noise_at_sec = Settings["start_sec"];
	Sig.results_file->write("snNoise/parameters/start_sec", start_noise_at_sec);

	std::string seed_type = static_cast<const char *>(Settings["seed_type"]);
	Sig.results_file->write("snNoise/parameters/seed_type", seed_type);
	if (seed_type == "time") {
		const unsigned int seed_time_preset = std::time(0);
		random_number_generator.seed(seed_time_preset);
		Sig.results_file->write("snNoise/parameters/seed_time_preset", seed_time_preset);
	} else if (seed_type == "preset") {
		const unsigned int seed_preset = static_cast<unsigned int>(Settings["seed_preset"]);
		Sig.results_file->write("snNoise/parameters/seed_preset", seed_preset);
		random_number_generator.seed(seed_preset);
	}

	// init AWGN noise generator:
	ready = 0;
	rconst1 = (2.0 / RAND_MAX);
	rconst2 = (RAND_MAX / 2.0);

	// setup widget
	snw = widget_factory->get_new_widget("AWGN");

	curve = new snCurveComplex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
	snPlot *plot = snw->get_new_plot_complex("real / imag", snPlot::Linear, curve);
	plot->setXAxis(0.0, get_max_xaxis_value());
	plot->setYAxis(-2, 2);
	plot->setXAxisLabel("t [s]");
	plot->setYAxisLabel("A");

	snPlot *plot_magn = snw->get_new_plot_complex("FFT, Magnitude", snPlot::FFTMagnitudeDB, curve);
	snPlot *plot_angl = snw->get_new_plot_complex("FFT, Angle", snPlot::FFTAngle, curve);

	snw->addPlot(plot);
	snw->addPlot(plot_magn);
	snw->addPlot(plot_angl);
}

snProcessorNoise::~snProcessorNoise() {
}

void snProcessorNoise::run() {
	snLog("snNoise: start.");

	Samples_Type absolute_sample = 0;

	// generate noise for bandwidth B = 2 * (smpl_freq / 2)
	// sigma^2 = N0 * B
	const double sigma = sqrt(n0_lin * smpl_freq); // noise std deviation for I and Q paths, respectively.

	// expectation = 0.0
	static boost::normal_distribution<double> norm_dist(0.0, sigma);
	static boost::variate_generator<base_generator_type&, boost::normal_distribution<double> > norm_value(
			random_number_generator, norm_dist);

	snLog(
			"snNoise: N0_dB: " + QString::number(n0_dB) + ", N0: " + QString::number(n0_lin) + ", sigma: "
					+ QString::number(sigma), snModule::debug);

	// calculate a factor to scale the whole signal after
	// adding the noise to values which will lay between -1 and 1.
	const double fac = (1.0 + 3.0 * (sigma));
	//const double fac = 1.0;

	// read data from buf:
	while (true) {
		itpp::cvec & BufOut = conc_buf->RequestWriting();
		itpp::cvec & BufIn = circ_buf_in->RequestReading();

		if (static_cast<double>(absolute_sample) / static_cast<double>(smpl_freq) >= start_noise_at_sec) {
			for (Samples_Type i = 0; i < block_size; i++) {
#ifdef WIN32
				// for windows:
				const double Noise1 = gaussian() * sigma;
				const double Noise2 = gaussian() * sigma;
#else
				// this works on linux. don't know why it doesn't work on windows:
				const double Noise1 = norm_value();
				const double Noise2 = norm_value();
#endif
				BufOut[i].real() = (BufIn[i].real() + Noise1) / fac;
				BufOut[i].imag() = (BufIn[i].imag() + Noise2) / fac;

				absolute_sample++;
			}
		} else {
			for (Samples_Type i = 0; i < block_size; i++) {
				BufOut[i] = BufIn[i];
				absolute_sample++;
			}
		}

		if (circ_buf_in->ReleaseReading()) {
			conc_buf->ReleaseWriting(true);
			snLog("snNoise: exit.");
			return;
		} else {
			// should we plot?
			if (snw->requestPlotFlag()) {
				for (Samples_Type o = 0; o < get_nof_xaxis_indices(); o++) {
					curve->update_value_X(o, static_cast<double>(o) / smpl_freq); // in s
					curve->update_value_Y(o, BufOut[o]);
				}
				snw->releasePlotFlag();
			}
			conc_buf->ReleaseWriting(false);
		}
	}
}

double snProcessorNoise::gaussian()
// generate zero mean unit variance Gaussian random numbers using the polar method
{

	double gauss;
	double r;
	double v1, v2;
	if (ready == 0) {
		do {
			v1 = (double) rand() - rconst2;
			v2 = (double) rand() - rconst2;
			v1 *= rconst1;
			v2 *= rconst1;
			r = v1 * v1 + v2 * v2;
		} while (r > 1.0);

		double fac = sqrt(-2.0 * log(r) / r);
		gstore = v1 * fac;
		gauss = v2 * fac;
		ready = 1;
	} else {
		ready = 0;
		gauss = gstore;
	}
	return (gauss);
}
