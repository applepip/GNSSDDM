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
/*
 #include <boost/random/linear_congruential.hpp>
 #include <boost/random/uniform_int.hpp>
 #include <boost/random/uniform_real.hpp>
 #include <boost/random/exponential_distribution.hpp>
 #include <boost/random/mersenne_twister.hpp>
 */

#include "snProcessorNoise.h"

snProcessorNoise::snProcessorNoise(libconfig::Setting &Settings, SNSignal pSNSignal, snWidget *psnw,
		ConcBuf* pConcBuf1, ConcBuf* pConcBuf2) :
	snBlock(pSNSignal), snw(psnw), ConcBuf1(pConcBuf1), ConcBuf2(pConcBuf2) {

	snLog("snNoise: setup.");

	Sig.results_file->createGroup("snNoise");
	Sig.results_file->createGroup("snNoise/parameters");

	snr_db = Settings["snr_db"];
	Sig.results_file->write("snNoise/parameters/snr_db", snr_db);

	start_noise_at_sec = Settings["start_sec"];
	Sig.results_file->write("snNoise/parameters/start_sec", start_noise_at_sec);

	std::string seed_type = static_cast<const char *> (Settings["seed_type"]);
	Sig.results_file->write("snNoise/parameters/seed_type", seed_type);
	if (seed_type == "time") {
		unsigned int seed_time_preset = std::time(0);
		random_number_generator.seed(seed_time_preset);
		Sig.results_file->write("snNoise/parameters/seed_time_preset", seed_time_preset);
	} else if (seed_type == "preset") {
		unsigned int seed_preset = static_cast<unsigned int> (Settings["seed_preset"]);
		Sig.results_file->write("snNoise/parameters/seed_preset", seed_preset);
		random_number_generator.seed(seed_preset);
	}

	// init AWGN noise generator:
	ready = 0;
	rconst1 = (2.0 / RAND_MAX);
	rconst2 = (RAND_MAX / 2.0);

	curve = new snCurveComplex(plot_x_max, Qt::blue, Qt::red);
	snPlotComplexRealImag *plot = new snPlotComplexRealImag("real / imag", snPlot::Linear, curve);
	plot->setXAxis(0.0, Sig.plot_buffer_length);
	plot->setYAxis(-8, 8);
	plot->setXAxisLabel("t [s]");
	plot->setYAxisLabel("A");

	snPlotMagn *plot_magn = new snPlotMagn("magn", snPlot::Log20, curve, Sig.SmplFreq);
	snPlotAngl *plot_angl = new snPlotAngl("angl", snPlot::Linear, curve, Sig.SmplFreq);

	snw->addPlot(plot);
	snw->addPlot(plot_magn);
	snw->addPlot(plot_angl);
}

snProcessorNoise::~snProcessorNoise() {
}

void snProcessorNoise::run() {
	snLog("snNoise: start.");

	Samples_Type nToRead;
	std::complex<double> *BufIn, *BufOut;

	double SNR = pow(10.0, snr_db / 10.0);
	//const double variance = (Sig.SmplFreq / 2.0) * (1.0 / SNR);
	const double sigma = sqrt((Sig.SmplFreq / 2.0) * (1.0 / SNR));

	// expectation = 0.0
	static boost::normal_distribution<double> norm_dist(0.0, sigma);
	static boost::variate_generator<base_generator_type&, boost::normal_distribution<double> > norm_value(
			random_number_generator, norm_dist);

	snLogDeb("snNoise: SNRdB: " + QString::number(snr_db) + ", SNR: " + QString::number(SNR) + ", sigma: "
			+ QString::number(sigma), 1);

	// read data from buf:
	while (isActive()) {
		// calculate a factor to scale the whole signal after
		// adding the noise to values which will lay between -1 and 1.
		double fac = (1.0 + 4.0 * (sigma));

		BufOut = ConcBuf2->RequestWriting();
		BufIn = ConcBuf1->RequestReading(&nToRead);

		if (static_cast<double> (getBlks()) * Sig.BufSize / static_cast<double> (Sig.SmplFreq)
				>= start_noise_at_sec) {
			for (Samples_Type i = 0; i < nToRead; i++) {
#ifdef WIN32
				// for windows:
				double Noise1 = gaussian() * sigma;
				double Noise2 = gaussian() * sigma;
#else
				// this works on linux. don't know why it doesn't work on windows:
				double Noise1 = norm_value();
				double Noise2 = norm_value();
#endif
				double INoise = (BufIn[i].real() + Noise1) / fac;
				double QNoise = (BufIn[i].imag() + Noise2) / fac;
				BufOut[i].real() = INoise;
				BufOut[i].imag() = QNoise;
			}
		} else {
			for (Samples_Type i = 0; i < nToRead; i++) {
				BufOut[i] = BufIn[i];
			}
		}

		if (ConcBuf1->ReleaseReading()) {
			ConcBuf2->ReleaseWriting(nToRead, 1);
			snLog("snNoise: exit.");
			printSummary("snProcessorNoise");
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
