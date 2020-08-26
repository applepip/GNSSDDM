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
 * \file snSDR.cpp
 *
 * Software Defined Receiver block source file.
 *
 * \author Frank Schubert
 * \date 2008-08-25
 * \version 0.1
 */

#include "snSDRStandard.h"
#include "../snSimulation/snFIRFilter.h"
#include "../snProcessorBlocks/snProcessorLPF.h"
#include "../snSimulation/snParallelCorrelation.h"
#include "snParallelBuildCode.h"

#include <QtConcurrentRun>

#define USEPARALLELISM

//const bool debugflag = true;
const bool debugflag = false;

/**
 * \brief SDRGPS constructor
 * @param pSNSignal1 struct SNSignal with general parameters
 * @param pSNSDR1 SDR parameter set
 * @param psnw pointer to snWidget
 * @param pConcBuf1 input ConcBuf
 * @param pBufSize buffer size
 */
snSDRStandard::snSDRStandard(libconfig::Setting &Settings, SNSignal pSNSignal,
		ProcessingParameters &pProcSig, snWidget *psnw, snWidget *ptrkw, snWidget *ptrkw4,
		snWidget *ptrkw3, snWidget *ptrkw2, ConcBuf* pConcBuf1) :
	snSDR(pSNSignal), ProcSig(pProcSig), snw(psnw), trkw(ptrkw), trkw2(ptrkw2), trkw3(ptrkw3),
			trkw4(ptrkw4), ConcBuf1(pConcBuf1) {

	Sig.results_file->createGroup("snSDRGPS");
	Sig.results_file->createGroup("snSDRGPS/parameters");

	intermed_freq = Settings["intermediate_frequency"]; // variable gain amplifier

	I_enable = false;
	Q_enable = false;

	gnss_signal = new snGNSSSignal(Settings["signal"], Sig.SmplFreq, Sig.Len);
	if (gnss_signal->inphase_enabled()) {
		if (debugflag)
			std::cout << "setting up I signal\n";
		I_enable = true;
		modulation_I = gnss_signal->get_inphase_modulation();
		gnss_code_I = modulation_I->get_code();
		//throw snSimulationException("snSDRGPS: no inphase signal defined");
	}

	if (gnss_signal->quadrature_enabled()) {
		if (debugflag)
			std::cout << "setting up Q signal\n";
		Q_enable = true;
		modulation_Q = gnss_signal->get_quadrature_modulation();
		gnss_code_Q = modulation_Q->get_code();
		//snLog("snSDRGPS: quadrature signal defined. Cannot track quadrature signal yet.");
	}

	if (!Q_enable && !I_enable) {
		throw snSimulationException("snSDRGPS: no inphase and no quadrature signal defined");
	}

	SDR.DoGPSAquisition = (Settings["aquisition_enable"]);
	Sig.results_file->write("snSDRGPS/parameters/aquisition_enable", SDR.DoGPSAquisition);

	// read the sample where the code is supposed to start from config file:
	if (!SDR.DoGPSAquisition) {
		SDR.StartTime = Settings["start_tracking_at_time"];
		Sig.results_file->write("snSDRGPS/parameters/start_tracking_at_time", SDR.StartTime);
	}
	SDR.DoGPSTracking = (Settings["tracking_enable"]);
	Sig.results_file->write("snSDRGPS/parameters/tracking_enable", SDR.DoGPSTracking);

	// tracking parameters
	Sig.results_file->createGroup("snSDRGPS/parameters/tracking");

	SDR.CorrLen = Settings["Tracking"]["DLL"]["correlation_length"];
	Sig.results_file->write("snSDRGPS/parameters/tracking/correlation_length", SDR.CorrLen);

	// DLL parameters
	Sig.results_file->createGroup("snSDRGPS/parameters/tracking/dll");

	SDR.DiscrType = static_cast<const char *> (Settings["Tracking"]["DLL"]["discriminator_type"]);
	Sig.results_file->write("snSDRGPS/parameters/tracking/dll/discriminator_type", SDR.DiscrType);

	SDR.TrkDLLDampRatio = Settings["Tracking"]["DLL"]["damping_ratio"]; // DLL damping ration
	Sig.results_file->write("snSDRGPS/parameters/tracking/dll/damping_ratio", SDR.TrkDLLDampRatio);

	SDR.TrkDLLNoiseBW = Settings["Tracking"]["DLL"]["noise_bandwidth"]; // DLL noise bandwidth
	Sig.results_file->write("snSDRGPS/parameters/tracking/dll/noise_bandwidth", SDR.TrkDLLNoiseBW);

	SDR.TrkEarlyLateSpacing = Settings["Tracking"]["DLL"]["early_late_spacing"];
	Sig.results_file->write("snSDRGPS/parameters/tracking/dll/early_late_spacing",
			SDR.TrkEarlyLateSpacing);

	// PLL parameters
	Sig.results_file->createGroup("snSDRGPS/parameters/tracking/pll");

	SDR.TrkPLLDampRatio = Settings["Tracking"]["PLL"]["damping_ratio"]; // PLL damping ration
	Sig.results_file->write("snSDRGPS/parameters/tracking/pll/damping_ratio", SDR.TrkPLLDampRatio);

	SDR.TrkPLLNoiseBW = Settings["Tracking"]["PLL"]["noise_bandwidth"]; // PLL noise bandwidth
	Sig.results_file->write("snSDRGPS/parameters/tracking/pll/noise_bandwidth", SDR.TrkPLLNoiseBW);

	SDR.CorrFuncSamples = Settings["Tracking"]["DLL"]["corr_func_samples"];
	Sig.results_file->write("snSDRGPS/parameters/tracking/corr_func_samples", SDR.CorrFuncSamples);

	SDR.CorrFuncStart = Settings["Tracking"]["DLL"]["corr_func_start"];
	Sig.results_file->write("snSDRGPS/parameters/tracking/corr_func_start", SDR.CorrFuncStart);

	SDR.CorrFuncDist = Settings["Tracking"]["DLL"]["corr_func_dist"];
	Sig.results_file->write("snSDRGPS/parameters/tracking/corr_func_dist", SDR.CorrFuncDist);

	SDR.WriteCorrFunc = Settings["Tracking"]["DLL"]["write_corr_func"];
	Sig.results_file->write("snSDRGPS/parameters/tracking/dll/write_corr_func", SDR.WriteCorrFunc);

	// aquisition parameters
	Sig.results_file->createGroup("snSDRGPS/parameters/aquisition");

	SDR.switch_plot_3d = Settings["Aquisition"]["plot_3d"];
	Sig.results_file->write("snSDRGPS/parameters/aquisition/plot_3d", SDR.switch_plot_3d);

	SDR.AqLen = Settings["Aquisition"]["length"]; // time duration for aquisition in sec
	Sig.results_file->write("snSDRGPS/parameters/aquisition/length", SDR.AqLen);

	SDR.AqFreqBand = Settings["Aquisition"]["aq_freq_band"];
	Sig.results_file->write("snSDRGPS/parameters/aquisition/aq_freq_band", SDR.AqFreqBand);

	SDR.AqFreqStep = Settings["Aquisition"]["aq_freq_step"];
	Sig.results_file->write("snSDRGPS/parameters/aquisition/aq_freq_step", SDR.AqFreqStep);

	SDR.AqThreshold = Settings["Aquisition"]["threshold"];
	Sig.results_file->write("snSDRGPS/parameters/aquisition/threshold", SDR.AqThreshold);

	aq_samples_number = static_cast<int> (SDR.AqLen * Sig.SmplFreq); // 100 000 samples
	Sig.results_file->write("snSDRGPS/parameters/aquisition/aq_samples_number", aq_samples_number);

	// init serial buffer
	InitSerialBuffer(12 * aq_samples_number + Sig.BufSize, ConcBuf1);

	SDR.aq_plot_code_phases = 500; // amount of samples for code phases in aq plot

	snLog("snSDR: Processing delay is: " + QString::number(ProcSig.ProcessingDelay));
	Sig.results_file->write("snSDRGPS/parameters/processing_delay", ProcSig.ProcessingDelay);

	// check if correlation length is integer multiple of code length?
	correlation_length_factor
			= static_cast<int> (SDR.CorrLen / gnss_code_I->get_code_length_time());
	//std::cout << "correlation_length_factor: " << correlation_length_factor << "\n";
	if (!dbl_eq(SDR.CorrLen / gnss_code_I->get_code_length_time(), correlation_length_factor)) {
		std::cout << "(SDR.CorrLen / gnss_code->get_code_length_time()):" << (SDR.CorrLen
				/ gnss_code_I->get_code_length_time()) << "\n";
		throw snSimulationException(QString("snSDR: error: correlation length (" + QString::number(
				gnss_code_I->get_code_length_time()) + ") is not a multiple of " + QString::number(
				gnss_code_I->get_code_length_time()) + "!").toStdString());
	}

	snLogDeb("snSDR: correlation length = " + QString::number(correlation_length_factor) + " * "
			+ QString::number(gnss_code_I->get_code_length_time()), 2);

	correlation_length = gnss_code_I->get_code_length_time();

	setup_plots();
}

snSDRStandard::~snSDRStandard() {
	delete gnss_signal;
}

/**
 * \brief starts the software receiver.
 */
void snSDRStandard::run() {
	snLog("snSDRGPS: start.");

	if (SDR.DoGPSAquisition) {
		bool aq_success = false;

		// build 1 GNSS code period
		//		std::vector<double> aq_signal = gnss_code->OneCodeBlkSampled(Sig.SmplFreq);
		std::vector<double> aq_signal = modulation_I->get_one_code_block_sampled();

		snLogDeb("prn: " + QString::number(prn) + ", samples per Code " + QString::number(
				aq_signal.size()), 1);
		double samplesPerChip = modulation_I->get_samples_per_chip();
		do {

			aq_success = Aquisition(aq_signal, samplesPerChip, 0.011, aq_frequency, aq_code_phase,
					snw, curve_aq);
			if (!aq_success) {
				snLog("snSDRGPS: aquisition unsuccessful.");
			}
		} while (!aq_success);

		// go back to the sample, where the aquisitioned code starts:
		set_next_sample_index(aq_code_phase);
	} else {
		aq_frequency = intermed_freq;

		aq_code_phase = SDR.StartTime * Sig.SmplFreq;
		aq_code_phase += round(ProcSig.ProcessingDelay * Sig.SmplFreq);

		// start tracking
		snLogDeb("snSDRGPS: Not performing aquisition.", 2);

		if (!isActive()) {
			snLog("snSDRGPS: exit.");
			printSummary("snSDR");
			return;
		}
		// read all samples before the signal starts. //canceled: substract 1, this way, the tracking will start at the right sample.
		if (debugflag)
			std::cout << "aq_code_phase: " << aq_code_phase << "\n";

		//for (Samples_Type i = 0; i < aq_code_phase; i++)
		get_next_samples(aq_code_phase);
	}

	snLog("snSDRGPS: Aquisitioned frequency: " + QString::number(aq_frequency)
			+ ", aquisitioned code phase: " + QString::number(aq_code_phase));

	if (!SDR.DoGPSTracking) {
		snLog("snSDR: not performing tracking. exit.");
		return;
	} else {
		snLog("snSDRGPS: starting tracking at sample: " + QString::number(get_absolute_sample()));

	}

	double code_freq_basis_I = 0.0, code_freq_I = 0.0, samples_per_code_basis_I = 0.0;
	int code_len_I = 0, code_len_Q = 0;
	double code_freq_basis_Q = 0.0, code_freq_Q = 0.0, samples_per_code_basis_Q = 0.0;

	if (I_enable) {
		code_freq_basis_I = gnss_code_I->get_code_freq_cps();
		code_freq_I = code_freq_basis_I; // will get changed by code loop
		samples_per_code_basis_I = modulation_I->get_samples_per_code();
		code_len_I = gnss_code_I->get_number_of_chips_per_code();
	}
	if (Q_enable) {
		code_freq_basis_Q = gnss_code_Q->get_code_freq_cps();
		code_freq_Q = code_freq_basis_Q; // will get changed by code loop
		samples_per_code_basis_Q = modulation_Q->get_samples_per_code();
		code_len_Q = gnss_code_Q->get_number_of_chips_per_code();
	}

	// adjust parameters according to correlation length:
	//code_len *= correlation_length_factor;
	double early_late_spacing = SDR.TrkEarlyLateSpacing;
	int raw_signal_max = static_cast<int> (SDR.AqLen * Sig.SmplFreq * 1.1
			* correlation_length_factor); // max buffer size for correlation buffers

	double gain_I = 0.0, gain_Q = 0.0;
	// get code gain:
	if (I_enable) {
		//gain_I = determine_gain_of_code(modulation_I, SDR.TrkEarlyLateSpacing);
		gain_I = modulation_I->get_discriminator_gain();
	}
	if (Q_enable) {
		//gain_Q = determine_gain_of_code(modulation_Q, SDR.TrkEarlyLateSpacing);
		gain_Q = modulation_Q->get_discriminator_gain();
	}
	snLog("snSDRGPS: info: code gain inphase: " + QString::number(gain_I) + ", code gain quad.: "
			+ QString::number(gain_Q));
	std::cout << "snSDRGPS: info: code gainI: " << gain_I << "\n";

	double carr_freq = aq_frequency;
	double carr_freq_basis = aq_frequency;
	double carrier_phase_remainder = 0.0;

	double prev_code_nco = 0.0;
	double prev_code_error = 0.0;
	double code_phase_remainder = 0.0;

	double prev_carr_nco = 0.0;
	double prev_carr_error = 0.0;

	double triarg = 0;

	double carr_error_I = 0.0, carr_error_Q = 0.0, carr_nco, tau2carr, tau1carr, PDIcarr =
			SDR.CorrLen;
	double code_error_I = 0.0, code_error_Q = 0.0, code_nco, tau2code, tau1code, PDIcode =
			SDR.CorrLen;

	calculate_loop_coefficients(tau1code, tau2code, SDR.TrkDLLNoiseBW, SDR.TrkDLLDampRatio, 1.0);
	calculate_loop_coefficients(tau1carr, tau2carr, SDR.TrkPLLNoiseBW, SDR.TrkPLLDampRatio, 0.25);

	snLogDeb("snSDRGPS: code loop parameters: tau1code: " + QString::number(tau1code)
			+ ", tau2code: " + QString::number(tau2code), 3);
	snLogDeb("snSDRGPS: carrier loop parameters: tau1carr: " + QString::number(tau1carr)
			+ ", tau2carr: " + QString::number(tau2carr), 3);

	unsigned long current_trk_point = 0;

	double actual_pseudo_range = 0;
	int result_counter = 0;

	// for C/N0 estimator:
	std::vector<double> vI_P(getTotalBlks());
	std::vector<double> vQ_P(getTotalBlks());
	std::vector<double> vNP(getTotalBlks());
	int aIQSamples = 20;
	int CN0EstAvg = 50; // average over 50 CN0 values


	//calculate filter coeffs for low pass filtering after down-mixing
	const size_t numtaps = 64;
	const double cut_off_frequency = 8e6;
	if (debugflag)
		std::cout << "calculating coefficients for lpf...";
	std::vector<double> coeffs = snProcessorLPF::remez_coefficients(cut_off_frequency
			/ Sig.SmplFreq / 2.0, numtaps);
	// for saving filter state for next block:
	std::vector<double> saved_state_I(numtaps), saved_state_Q(numtaps);
	if (debugflag)
		std::cout << "done.\n";

	while (1) {
		// Tracking //

		if (I_enable) {
			modulation_I->get_code()->set_freq_cps(code_freq_I);
		}
		if (Q_enable) {
			modulation_Q->get_code()->set_freq_cps(code_freq_I);
		}

		const double samples_per_chip = modulation_I->get_samples_per_chip();
		const double samples_per_code = modulation_I->get_samples_per_code();

		const int blksize = static_cast<int> (ceil((static_cast<double> (code_len_I)
				- code_phase_remainder) * samples_per_chip));

		if (debugflag)
			std::cout << "blksize: " << blksize << "\n";

		if (blksize > raw_signal_max) {
			snLog("snSDRGPS: rawSignal[] too small. blksize: " + QString::number(blksize)
					+ ",  rawSignalMax: " + QString::number(raw_signal_max) + ". exit.");
			return;
		}

		// init tracking
		std::vector<double> rawSignal(blksize);
		std::vector<std::complex<double> > rawSignal_C(blksize);
		std::vector<double> early_code_I(blksize);
		std::vector<double> prompt_code_I(blksize);
		std::vector<double> late_code_I(blksize);
		std::vector<double> early_code_Q(blksize);
		std::vector<double> prompt_code_Q(blksize);
		std::vector<double> late_code_Q(blksize);

		//		for (int i = 0; i < blksize; i++) {
		rawSignal_C = get_next_samples(blksize);

		for (int i = 0; i < blksize; i++)
			rawSignal[i] = rawSignal_C[i].real();

		if (!isActive()) {
			// simulation is over now.
			// write parameters which were saved in the curves to file
			Sig.results_file->createGroup("snSDRGPS/output");

			trkw3->pushData();
			Sig.results_file->write("snSDRGPS/output/pll_raw", curve_pll_raw_I->Y,
					current_trk_point);
			Sig.results_file->write("snSDRGPS/output/pll_filtered", curve_pll_filtered->Y,
					current_trk_point);
			Sig.results_file->write("snSDRGPS/output/dll_raw", curve_dll_raw_I->Y,
					current_trk_point);
			Sig.results_file->write("snSDRGPS/output/dll_filtered", curve_dll_filtered->Y,
					current_trk_point);
			Sig.results_file->write("snSDRGPS/output/remcodephase", curve_remcodephase->Y,
					current_trk_point);
			trkw3->releasePlotFlag();

			trkw2->pushData();
			Sig.results_file->write("snSDRGPS/output/pr", curve_pr->Y, current_trk_point);
			trkw2->releasePlotFlag();

			trkw4->pushData();
			Sig.results_file->write("snSDRGPS/output/corr_early", curve_corr_e_I->Y,
					current_trk_point);
			Sig.results_file->write("snSDRGPS/output/corr_prompt", curve_corr_p_I->Y,
					current_trk_point);
			Sig.results_file->write("snSDRGPS/output/corr_late", curve_corr_l_I->Y,
					current_trk_point);
			Sig.results_file->write("snSDRGPS/output/c_n0", curve_cn0->Y, current_trk_point);
			trkw4->releasePlotFlag();

			snLog("snSDR: exit.");
			printSummary("snSDR");
			return;
		}
		//}

#ifdef USEPARALLELISM
		QFuture<std::vector<double> > f_e_I, f_p_I, f_l_I;
		QFuture<std::vector<double> > f_e_Q, f_p_Q, f_l_Q;

		std::vector<double> ind_d_e(blksize), ind_d_p(blksize), ind_d_l(blksize);
		for (int i = 0; i < blksize; i++) {
			// indices early:
			ind_d_e[i] = (code_phase_remainder - early_late_spacing / 2.0) * samples_per_chip
					+ static_cast<double> (i);
			//if (ind_d_e[i] < 0) {
			ind_d_e[i] += samples_per_code;
			//				ind_d_e[i] += samples_per_chip * 1023.0;
			//}
			// indices late:
			ind_d_l[i] = (code_phase_remainder + early_late_spacing / 2.0) * samples_per_chip
					+ static_cast<double> (i);
			/*
			 if (ind_d_l[i] >= samples_per_code) {
			 ind_d_l[i] -= samples_per_code;
			 }
			 */
			// indices prompt:
			ind_d_p[i] = code_phase_remainder * samples_per_chip + static_cast<double> (i);
		}

		// build early code:
		if (I_enable) {
			f_e_I = QtConcurrent::run(build_code, ind_d_e, modulation_I);
			f_p_I = QtConcurrent::run(build_code, ind_d_p, modulation_I);
			f_l_I = QtConcurrent::run(build_code, ind_d_l, modulation_I);
		}

		if (Q_enable) {
			f_e_Q = QtConcurrent::run(build_code, ind_d_e, modulation_Q);
			f_p_Q = QtConcurrent::run(build_code, ind_d_p, modulation_Q);
			f_l_Q = QtConcurrent::run(build_code, ind_d_l, modulation_Q);
		}
#else
		for (int i = 0; i < blksize; i++) {
			double ind_d = (code_phase_remainder - early_late_spacing / 2.0) * samples_per_chip
			+ static_cast<double> (i);

			// build early code:

			if (ind_d < 0) {
				//std::cout << "ind_d < 0 : " << ind_d << ", code_len: " << code_len << "\n";
				ind_d += samples_per_code;
			}
			if (I_enable) {
				early_code_I.at(i) = modulation_I->get_value_for_absolute_sample_number(
						round(ind_d));
			}
			if (Q_enable) {
				early_code_Q.at(i) = modulation_Q->get_value_for_absolute_sample_number(
						round(ind_d));

			}

			// build late code:
			ind_d = (code_phase_remainder + early_late_spacing / 2.0) * samples_per_chip
			+ static_cast<double> (i);
			if (I_enable) {
				late_code_I.at(i)
				= modulation_I->get_value_for_absolute_sample_number(round(ind_d));
			}
			if (Q_enable) {
				late_code_Q.at(i)
				= modulation_Q->get_value_for_absolute_sample_number(round(ind_d));
			}

			// build prompt code:
			ind_d = code_phase_remainder * samples_per_chip + static_cast<double> (i);
			if (I_enable) {
				prompt_code_I.at(i) = modulation_I->get_value_for_absolute_sample_number(round(
								ind_d));
			}
			if (Q_enable) {
				prompt_code_Q.at(i) = modulation_Q->get_value_for_absolute_sample_number(round(
								ind_d));
			}
		}
#endif

		//		const int blksize = static_cast<int> (ceil((static_cast<double> (code_len_I)
		//				- code_phase_remainder) * samples_per_chip));


		// calculate new code phase remainder
		//		double ind_d = code_phase_remainder * samples_per_chip + static_cast<double> (blksize);
		//		code_phase_remainder = ind_d / samples_per_chip - static_cast<double> (code_len_I);


		// TODO numerical problems in the following formula. even in steady state,
		// code_phase_remainder is growing due to the addition.
		// once cpr is != 0 during the pull-in right at the beginning, ind_d grows and grows.
		double ind_d = code_phase_remainder + static_cast<double> (blksize) / samples_per_chip;
		code_phase_remainder = ind_d - static_cast<double> (code_len_I);

		//code_phase_remainder = 0.0;
		/*
		 std::cout.precision(20);
		 std::cout << "static_cast<double> (blksize): " << static_cast<double> (blksize)
		 << ", samples_per_chip: " << samples_per_chip << ", ind_d: " << ind_d
		 << ", code_phase_remainder: " << code_phase_remainder << "\n";
		 */
		// generate carrier
		const double twopif = (carr_freq * 2.0 * M_PI);
		//std::cout << "carr_freq: " << carr_freq << "\n";

		std::vector<double> carrCos(blksize);
		std::vector<double> carrSin(blksize);
		std::vector<double> BBSig_I2(blksize);
		std::vector<double> BBSig_Q2(blksize);

		for (int i = 0; i < blksize; i++) {
			triarg = (twopif * static_cast<double> (i) / Sig.SmplFreq) + carrier_phase_remainder;
			//triarg = fmod(triarg, 2.0 * M_PI);
			carrCos[i] = cos(triarg);
			carrSin[i] = -sin(triarg);
			//carr.at(i) = std::complex<double>(cos(triarg), -sin(triarg));
			// mix down to base band
			BBSig_I2[i] = carrCos[i] * rawSignal[i];
			BBSig_Q2[i] = carrSin[i] * rawSignal[i];
		}

		// integer division does not round.  = fix(triarg/2*pi)
		triarg = (twopif * static_cast<double> (blksize) / Sig.SmplFreq) + carrier_phase_remainder;
		carrier_phase_remainder = triarg - ((int) (triarg / (2.0 * M_PI))) * (2.0 * M_PI);

		// filter out high frequency:
		/*
		 if (blksize > init_blksize) {
		 // TODO in this case we need to create new filters. but with keeping the state. difficult.
		 throw snSimulationException(
		 "snSDRGPS::correlation: error: blksize > init_blksize. this must not happen.");
		 }
		 filtI.set_datalen(blksize);
		 filtQ.set_datalen(blksize);
		 */
		// data BBSIg_I/Q need to be static do be filtered
		/*
		 // calc initial blksize for size estimate
		 const int init_blksize = static_cast<int> (ceil((static_cast<double> (code_len_I))
		 * modulation_I->get_samples_per_chip()));
		 // input to the filters:
		 double BBSig_I[2 * init_blksize];
		 double BBSig_Q[2 * init_blksize];
		 // for the results:
		 double BBSig_I2[2 * init_blksize], BBSig_Q2[2 * init_blksize];
		 */
		std::vector<double> BBSig_I(blksize);
		std::vector<double> BBSig_Q(blksize);

		/*
		 snFIRFilter filtI(&BBSig_I2[0], &BBSig_I[0], blksize, &coeffs[0], numtaps);
		 snFIRFilter filtQ(&BBSig_Q2[0], &BBSig_Q[0], blksize, &coeffs[0], numtaps);

		 filtI.set_state(saved_state_I);
		 filtQ.set_state(saved_state_Q);
		 filtI.start();
		 filtQ.start();
		 filtI.wait();
		 filtQ.wait();
		 saved_state_I = filtI.get_state();
		 saved_state_Q = filtQ.get_state();
		 */
		for (int i = 0; i < blksize; i++) {
			BBSig_I[i] = BBSig_I2[i];
			BBSig_Q[i] = BBSig_Q2[i];
		}
		// the downmixed and filtered signal is now in BBSig_I/Q

#ifdef USEPARALLELISM
		// now we need the results from the parallel code build:
		if (I_enable) {
			early_code_I = f_e_I.result();
			prompt_code_I = f_p_I.result();
			late_code_I = f_l_I.result();
		}
		if (Q_enable) {
			early_code_Q = f_e_Q.result();
			prompt_code_Q = f_p_Q.result();
			late_code_Q = f_l_Q.result();
		}
#endif

#ifdef USEPARALLELISM
		// parallel correlation
		QFuture<double> future_eiI, future_eiQ, future_piI, future_piQ, future_liI, future_liQ;
		QFuture<double> future_eqI, future_eqQ, future_pqI, future_pqQ, future_lqI, future_lqQ;
		if (I_enable) {
			future_piI = QtConcurrent::run(correlation, prompt_code_I, BBSig_I);
			future_piQ = QtConcurrent::run(correlation, prompt_code_I, BBSig_Q);
			future_eiI = QtConcurrent::run(correlation, early_code_I, BBSig_I);
			future_eiQ = QtConcurrent::run(correlation, early_code_I, BBSig_Q);
			future_liI = QtConcurrent::run(correlation, late_code_I, BBSig_I);
			future_liQ = QtConcurrent::run(correlation, late_code_I, BBSig_Q);
		}
		if (Q_enable) {
			// for quadrature, the imag channel of BBSIg gives actually the
			// inphase result for the correlation. That's why, .imag() and
			// .real() are swapped here:
			future_pqI = QtConcurrent::run(correlation, prompt_code_Q, BBSig_Q);
			future_pqQ = QtConcurrent::run(correlation, prompt_code_Q, BBSig_I);
			future_eqI = QtConcurrent::run(correlation, early_code_Q, BBSig_Q);
			future_eqQ = QtConcurrent::run(correlation, early_code_Q, BBSig_I);
			future_lqI = QtConcurrent::run(correlation, late_code_Q, BBSig_Q);
			future_lqQ = QtConcurrent::run(correlation, late_code_Q, BBSig_I);
		}
#else
		for (int i = 0; i < blksize; i++) {
			if (I_enable) {
				corr_early_inphase_code_I += early_code_I[i] * BBSig_I[i];
				corr_early_inphase_code_Q += early_code_I[i] * BBSig_Q[i];
				corr_prompt_inphase_code_I += prompt_code_I[i] * BBSig_I[i];
				corr_prompt_inphase_code_Q += prompt_code_I[i] * BBSig_Q[i];
				corr_late_inphase_code_I += late_code_I[i] * BBSig_I[i];
				corr_late_inphase_code_Q += late_code_I[i] * BBSig_Q[i];
			}
			if (Q_enable) {
				// for quadrature, the imag channel of BBSIg gives actually the
				// inphase result for the correlation. That's why, .imag() and
				// .real() are swapped here:
				corr_early_quadrature_code_I += early_code_Q[i] * BBSig_Q[i];
				corr_early_quadrature_code_Q += early_code_Q[i] * BBSig_I[i];
				corr_prompt_quadrature_code_I += prompt_code_Q[i] * BBSig_Q[i];
				corr_prompt_quadrature_code_Q += prompt_code_Q[i] * BBSig_I[i];
				corr_late_quadrature_code_I += late_code_Q[i] * BBSig_Q[i];
				corr_late_quadrature_code_Q += late_code_Q[i] * BBSig_I[i];
			}
		}
#endif

		// init correlation
		double corr_early_inphase_code_I = 0.0, corr_early_inphase_code_Q = 0.0,
				corr_prompt_inphase_code_I = 0.0, corr_prompt_inphase_code_Q = 0.0,
				corr_late_inphase_code_I = 0.0, corr_late_inphase_code_Q = 0.0;
		double corr_early_quadrature_code_I = 0.0, corr_early_quadrature_code_Q = 0.0,
				corr_prompt_quadrature_code_I = 0.0, corr_prompt_quadrature_code_Q = 0.0,
				corr_late_quadrature_code_I = 0.0, corr_late_quadrature_code_Q = 0.0;

#ifdef USEPARALLELISM
		// now we need the results for the inphase prompt codes:
		if (I_enable) {
			corr_prompt_inphase_code_I = future_piI.result();
			corr_prompt_inphase_code_Q = future_piQ.result();
		}
#endif

		// get PLL error
		// carrier loop I discriminator:
		carr_error_I = costas_discriminator_atan(corr_prompt_inphase_code_I,
				corr_prompt_inphase_code_Q);

#ifdef USEPARALLELISM
		// now we need the results for the quadrature prompt codes:
		if (Q_enable) {
			corr_prompt_quadrature_code_I = future_pqI.result();
			corr_prompt_quadrature_code_Q = future_pqQ.result();
		}
#endif

		// carrier loop Q discriminator:
		carr_error_Q = costas_discriminator_atan(corr_prompt_quadrature_code_I,
				corr_prompt_quadrature_code_Q);

		// carrier loop filter, NCO
		carr_nco = prev_carr_nco + (tau2carr / tau1carr) * (carr_error_I - prev_carr_error)
				+ carr_error_I * (PDIcarr / tau1carr);
		prev_carr_nco = carr_nco;
		prev_carr_error = carr_error_I;

		// new carrier frequency
		carr_freq = carr_freq_basis + carr_nco;

#ifdef USEPARALLELISM
		// now we need the results for all remaining codes:
		if (I_enable) {
			corr_early_inphase_code_I = future_eiI.result();
			corr_early_inphase_code_Q = future_eiQ.result();
			corr_late_inphase_code_I = future_liI.result();
			corr_late_inphase_code_Q = future_liQ.result();
		}
		if (Q_enable) {
			corr_early_quadrature_code_I = future_eiI.result();
			corr_early_quadrature_code_Q = future_eqQ.result();
			corr_late_quadrature_code_I = future_lqI.result();
			corr_late_quadrature_code_Q = future_lqQ.result();
		}
#endif
		/*
		 std::cout << "Early: " << abs(std::complex<double>(corr_early_inphase_code_I,
		 corr_early_inphase_code_Q)) << ", Late: " << abs(std::complex<double>(
		 corr_late_inphase_code_I, corr_late_inphase_code_Q)) << "\n";
		 */
		//		try {
		if (SDR.DiscrType == "DP") { // dot product
			if (I_enable) {
				code_error_I = (1.0 / gain_I) * discriminator_dot_product_normalized(
						corr_early_inphase_code_I, corr_early_inphase_code_Q,
						corr_prompt_inphase_code_I, corr_prompt_inphase_code_Q,
						corr_late_inphase_code_I, corr_late_inphase_code_Q);
			}
			if (Q_enable) {
				code_error_Q = (1.0 / gain_Q) * discriminator_dot_product_normalized(
						corr_early_quadrature_code_I, corr_early_quadrature_code_Q,
						corr_prompt_quadrature_code_I, corr_prompt_quadrature_code_Q,
						corr_late_quadrature_code_I, corr_late_quadrature_code_Q);
			}
		} else if (SDR.DiscrType == "EML") { // early minus late power normalized
			if (I_enable) {
				code_error_I = (1.0 / gain_I) * discriminator_early_minus_late_power_normalized(
						corr_early_inphase_code_I, corr_early_inphase_code_Q,
						corr_late_inphase_code_I, corr_late_inphase_code_Q);
			}
			if (Q_enable) {
				code_error_Q = (1.0 / gain_Q) * discriminator_early_minus_late_power_normalized(
						corr_early_quadrature_code_I, corr_early_quadrature_code_Q,
						corr_late_quadrature_code_I, corr_late_quadrature_code_Q);
			}

		}
		//		} catch (...) {
		// TODO
		//code_error_I = 0.0;
		//		}

		if (std::isnan(code_error_I)) {
			// TODO
			if (debugflag)
				std::cout << "code_error_I is nan. resetting to 0.\n";

			snLog("snSDR: code_error_I is nan. resetting to 0. probably out of lock.");
			code_error_I = 0.0;
		}

		double code_error = 0.0;

		if (I_enable && Q_enable) {
			code_error = .5 * code_error_I + .5 * code_error_Q;
		}
		if (I_enable && !Q_enable) { // only I correlator active
			code_error = code_error_I;
		}
		if (!I_enable && Q_enable) { // only Q correlator active
			code_error = code_error_Q;
		}

		//std::cout << "code_error: " << code_error << "\n";

		// code loop filter, NCO
		code_nco = prev_code_nco + (tau2code / tau1code) * (code_error - prev_code_error)
				+ code_error * (PDIcode / tau1code);
		prev_code_nco = code_nco;
		prev_code_error = code_error;

		// new code freq
		//codeNco = 0; // this for open loop
		code_freq_I = code_freq_basis_I - code_nco;

		// just use result for I also for Q
		// TODO improve
		code_freq_Q = code_freq_I;

		// PSEUDORANGE CALCULATION

		const Samples_Type absolute_sample = get_absolute_sample() - aq_code_phase;
		const double travel_time = static_cast<double> (absolute_sample) / samples_per_code_basis_I;
		//std::cout << "absolute_sample: " << absolute_sample
		//		<< ", travel_time: " << travel_time << "\n";
		//const double travel_time = static_cast<double> (absolute_sample) / modulation->get_samples_per_code();

		// PR calculation.
		const double fact1 = code_freq_basis_I / static_cast<double> (code_len_I);
		actual_pseudo_range = (travel_time - floor(travel_time)) * Sig.c0 / fact1;
		//std::cout << "absolute_sample: " << absolute_sample
		//		<< ", travel_time: " << travel_time
		//		<< ", (travel_time - floor(travel_time)): " << (travel_time
		//		- floor(travel_time)) << ", actual_pseudo_range: "
		//		<< actual_pseudo_range << "\n";
		actual_pseudo_range -= code_phase_remainder * (Sig.c0 / code_freq_basis_I); // also use code phase remainder to improve PR calculation

		if ((travel_time - floor(travel_time)) > .5) {
			// pr is negative. add one code length in meter to avoid flipping to 300000m for gps e.g.:
			actual_pseudo_range -= Sig.c0 / fact1;
		}
		//actual_pseudo_range -= code_nco * (Sig.c0 / code_freq_basis_I);

		// if no aquisition was done and the start of the tracking is defined in the config file,
		// we need to substract the processing delay that was introduced by previous filters:
		/*
		 if (!SDR.DoGPSAquisition) {
		 actual_pseudo_range -= ProcSig.ProcessingDelay * Sig.c0; // compensate for processing delays
		 }
		 */

		snLogDeb("snSDRGPS: actualPr: " + QString::number(actual_pseudo_range, 'f', 2), 2);

		// C/N0 estimator:
		double CN0 = 0.0;
		Blocks_Type cur_block = getBlks();
		vI_P.at(cur_block) = corr_prompt_inphase_code_I;
		vQ_P.at(cur_block) = corr_prompt_inphase_code_Q;

		// average over 10 CN0 values
		if (cur_block > aIQSamples) {
			vNP.at(cur_block) = cn0_estimation(std::vector<double>(vI_P.begin() + cur_block
					- aIQSamples, vI_P.begin() + cur_block), std::vector<double>(vQ_P.begin()
					+ cur_block - aIQSamples, vQ_P.begin() + cur_block));

			// average:
			if (cur_block > CN0EstAvg) {
				double muNP = 0;
				for (int i = 0; i < CN0EstAvg; i++) {
					muNP += vNP.at(cur_block - i);
				}
				muNP = muNP / CN0EstAvg;
				CN0 = 2.0 * (1.0 / SDR.CorrLen) * (muNP - 1.0) / (aIQSamples - muNP);
				//				CN0 = (1.0 / SDR.CorrLen) * (muNP - 1.0) / (aIQSamples - muNP);
				// factor 2 because this "N0 is 2*No".
				snLogDeb("snSDR: estimated C/N0: " + QString::number(CN0), 3);
			}
		}

		// plot the correlation results:
		trkw4->pushData();
		// inphase
		const double time = current_trk_point * correlation_length;
		curve_corr_e_I->X[current_trk_point] = time;
		curve_corr_e_I->Y[current_trk_point] = std::complex<double>(corr_early_inphase_code_I,
				corr_early_inphase_code_Q);
		curve_corr_p_I->X[current_trk_point] = time;
		curve_corr_p_I->Y[current_trk_point] = std::complex<double>(corr_prompt_inphase_code_I,
				corr_prompt_inphase_code_Q);
		curve_corr_l_I->X[current_trk_point] = time;
		curve_corr_l_I->Y[current_trk_point] = std::complex<double>(corr_late_inphase_code_I,
				corr_late_inphase_code_Q);

		// adjust axes of corr result plot
		int minindex = current_trk_point - 5000;
		if (minindex < 0)
			minindex = 0;
		double prmin = 200000;
		double prmax = 0;
		for (unsigned int k = minindex; k < current_trk_point; k++) {
			if (curve_corr_e_I->Y[k].real() > prmax)
				prmax = curve_corr_e_I->Y[k].real();
			if (curve_corr_e_I->Y[k].real() < prmin)
				prmin = curve_corr_e_I->Y[k].real();
		}
		for (unsigned int k = minindex; k < current_trk_point; k++) {
			if (curve_corr_l_I->Y[k].real() > prmax)
				prmax = curve_corr_l_I->Y[k].real();
			if (curve_corr_l_I->Y[k].real() < prmin)
				prmin = curve_corr_l_I->Y[k].real();
		}
		plot_corrs_I->setYAxis(prmin - 5, prmax + 5);

		// quadrature:
		curve_corr_e_Q->X[current_trk_point] = time;
		curve_corr_e_Q->Y[current_trk_point] = std::complex<double>(corr_early_quadrature_code_I,
				corr_early_quadrature_code_Q);
		curve_corr_p_Q->X[current_trk_point] = time;
		curve_corr_p_Q->Y[current_trk_point] = 0;//std::complex<double>(corr_prompt_quadrature_code_I, corr_prompt_quadrature_code_Q);
		curve_corr_l_Q->X[current_trk_point] = time;
		curve_corr_l_Q->Y[current_trk_point] = std::complex<double>(corr_late_quadrature_code_I,
				corr_late_quadrature_code_Q);
		// plot C/N0:
		curve_cn0->X[current_trk_point] = time;
		curve_cn0->Y[current_trk_point] = CN0;
		trkw4->releasePlotFlag();

		trkw3->pushData();
		curve_pll_raw_I->X[current_trk_point] = time;
		curve_pll_raw_I->Y[current_trk_point] = carr_error_I;
		curve_pll_raw_Q->X[current_trk_point] = time;
		curve_pll_raw_Q->Y[current_trk_point] = carr_error_Q;
		curve_pll_filtered->X[current_trk_point] = time;
		curve_pll_filtered->Y[current_trk_point] = carr_nco;
		curve_dll_raw_I->X[current_trk_point] = time;
		curve_dll_raw_I->Y[current_trk_point] = code_error_I;
		curve_dll_raw_Q->X[current_trk_point] = time;
		curve_dll_raw_Q->Y[current_trk_point] = code_error_Q;
		curve_dll_raw_IQ->X[current_trk_point] = time;
		curve_dll_raw_IQ->Y[current_trk_point] = code_error;
		curve_dll_filtered->X[current_trk_point] = time;
		curve_dll_filtered->Y[current_trk_point] = code_nco;
		curve_remcodephase->X[current_trk_point] = time;
		curve_remcodephase->Y[current_trk_point] = code_phase_remainder;
		trkw3->releasePlotFlag();

		// plot additional correlation function samples:
		std::vector<double> CorrResults_I(SDR.CorrFuncSamples);
		std::vector<double> CorrResults_Q(SDR.CorrFuncSamples);
		std::vector<double> CorrFuncX(SDR.CorrFuncSamples);
		int plotting = trkw2->requestPlotFlag();
		if (SDR.WriteCorrFunc || plotting) {
			for (int o = 0; o < SDR.CorrFuncSamples; o++) {
				CorrFuncX.at(o) = SDR.CorrFuncStart + o * SDR.CorrFuncDist;
				if (I_enable) {
					std::vector<double> CorrFuncCode = build_shifted_code(code_phase_remainder
							+ CorrFuncX.at(o), modulation_I, blksize);
					CorrResults_I.at(o) = correlation(CorrFuncCode, BBSig_I);
				}
				if (Q_enable) {
					std::vector<double> CorrFuncCode = build_shifted_code(code_phase_remainder
							+ CorrFuncX.at(o), modulation_Q, blksize);
					CorrResults_Q.at(o) = correlation(CorrFuncCode, BBSig_Q);
				}
			}
			if (plotting) { // plot correlation function if needed:
				if (I_enable) {
					for (int o = 0; o < SDR.CorrFuncSamples; o++) {
						curve_corrfunc1_I->X.at(o) = CorrFuncX.at(o);
						curve_corrfunc1_I->Y.at(o) = CorrResults_I.at(o);
					}
					curve_corrfunc2_I->X.at(0) = -SDR.TrkEarlyLateSpacing / 2.0;
					curve_corrfunc2_I->Y.at(0) = corr_early_inphase_code_I;
					curve_corrfunc2_I->X.at(1) = 0;
					curve_corrfunc2_I->Y.at(1) = (corr_prompt_inphase_code_I);
					curve_corrfunc2_I->X.at(2) = SDR.TrkEarlyLateSpacing / 2.0;
					curve_corrfunc2_I->Y.at(2) = (corr_late_inphase_code_I);
				}
				if (Q_enable) {
					for (int o = 0; o < SDR.CorrFuncSamples; o++) {
						curve_corrfunc1_Q->X.at(o) = CorrFuncX.at(o);
						curve_corrfunc1_Q->Y.at(o) = CorrResults_Q.at(o);
					}
					curve_corrfunc2_Q->X.at(0) = -SDR.TrkEarlyLateSpacing / 2.0;
					curve_corrfunc2_Q->Y.at(0) = (corr_early_quadrature_code_I);
					curve_corrfunc2_Q->X.at(1) = 0;
					curve_corrfunc2_Q->Y.at(1) = (corr_prompt_quadrature_code_I);
					curve_corrfunc2_Q->X.at(2) = SDR.TrkEarlyLateSpacing / 2.0;
					curve_corrfunc2_Q->Y.at(2) = (corr_late_quadrature_code_I);
				}

				double ymin_I = *std::min_element(CorrResults_I.begin(), CorrResults_I.end());
				double ymax_I = *std::max_element(CorrResults_I.begin(), CorrResults_I.end());
				plot_corrfunc1->setYAxis(ymin_I, ymax_I);
				plot_corrfunc2->setYAxis(ymin_I, ymax_I);

				// adjust axes of pr plot according to the last 50 values
				//size_t prend = current_trk_point;
				size_t pstart = 0;
				if (current_trk_point < 50)
					pstart = 0;
				else
					pstart = current_trk_point - 50;

				//double prmin = *std::min_element(curve_pr->Y.begin() + pstart, curve_pr->Y.begin() + prend);
				//double prmax = *std::max_element(curve_pr->Y.begin() + pstart, curve_pr->Y.begin() + prend);
				//std::cout << "prmax: " << prmax << ", prmin: " << prmin << "\n";
				//plot_pr->setYAxis(prmin - 7, prmax + 7);

				trkw2->releasePlotFlag();
			}

		}
		trkw2->pushData();
		curve_pr->X[current_trk_point] = time;
		curve_pr->Y[current_trk_point] = actual_pseudo_range;
		trkw2->releasePlotFlag();

		// plot Codes and incoming signals
		if (trkw->requestPlotFlag()) {
			for (Samples_Type i = 0; i < plot_x_max; i++) {
				const double time = static_cast<double> (i) / Sig.SmplFreq; // in s
				curve_incoming->X.at(i) = time;
				curve_incoming->Y.at(i) = rawSignal.at(i);
				curve_carrier_replica->X.at(i) = time;
				curve_carrier_replica->Y.at(i) = std::complex<double>(carrCos.at(i), carrSin.at(i));
				curve_downmixed->X.at(i) = time;
				curve_downmixed->Y.at(i) = std::complex<double>(BBSig_I[i], BBSig_Q[i]);//std::complex<double>(BBSigI.at(i), BBSigQ.at(i));
				curve_code_e_I->X.at(i) = time;
				curve_code_e_I->Y.at(i) = early_code_I.at(i);
				curve_code_p_I->X.at(i) = time;
				curve_code_p_I->Y.at(i) = prompt_code_I.at(i);
				curve_code_l_I->X.at(i) = time;
				curve_code_l_I->Y.at(i) = late_code_I.at(i);
				curve_code_e_Q->X.at(i) = time;
				curve_code_e_Q->Y.at(i) = early_code_Q.at(i);
				curve_code_p_Q->X.at(i) = time;
				curve_code_p_Q->Y.at(i) = prompt_code_Q.at(i);
				curve_code_l_Q->X.at(i) = time;
				curve_code_l_Q->Y.at(i) = late_code_Q.at(i);
			}
			trkw->releasePlotFlag();
		}

		current_trk_point++;
		result_counter++;
	}
}

double snSDRStandard::cn0_estimation(const std::vector<double> &vI_P,
		const std::vector<double> &vQ_P) {
	double WBP = 0, NBP = 0;
	double ISum = 0, QSum = 0;

	for (Samples_Type i = 0; i < vI_P.size(); i++) {
		WBP += vI_P.at(i) * vI_P.at(i) + vQ_P.at(i) * vQ_P.at(i);
		ISum += vI_P.at(i);
		QSum += vQ_P.at(i);
	}
	NBP = ISum * ISum + QSum * QSum;

	return (NBP / WBP);
}

void snSDRStandard::setup_plots() {
	int expected_corr_intervals = static_cast<int> (Sig.Len * (1.0
			/ gnss_code_I->get_code_length_time()) * 1.1 / correlation_length_factor);
	double expected_lenth = expected_corr_intervals * correlation_length;

	// setup aqusition widget
	if (SDR.switch_plot_3d) {
		curve_aq = new snCurve3D(SDR.aq_plot_code_phases, static_cast<int> (floor(SDR.AqFreqBand
				/ SDR.AqFreqStep)));
		snPlotSurface *plot_aq = new snPlotSurface("Aquisition", snPlot::Linear, curve_aq);
		snw->addPlot(plot_aq);
	}

	// setup correlation result curves
	curve_corr_e_I = new snCurveComplex(expected_corr_intervals, Qt::green, Qt::darkGreen);
	curve_corr_p_I = new snCurveComplex(expected_corr_intervals, Qt::red, Qt::darkRed);
	curve_corr_l_I = new snCurveComplex(expected_corr_intervals, Qt::blue, Qt::darkBlue);

	curve_corr_e_Q = new snCurveComplex(expected_corr_intervals, Qt::green, Qt::darkGreen);
	curve_corr_p_Q = new snCurveComplex(expected_corr_intervals, Qt::red, Qt::darkRed);
	curve_corr_l_Q = new snCurveComplex(expected_corr_intervals, Qt::blue, Qt::darkBlue);

	curve_remcodephase = new snCurveReal(expected_corr_intervals, Qt::blue);
	curve_cn0 = new snCurveReal(expected_corr_intervals, Qt::blue);

	// init corr curves:
	for (int i = 0; i < curve_corr_e_I->nx; i++) {
		double time = i * correlation_length;
		curve_corr_e_I->X.at(i) = time;
		curve_corr_e_I->Y.at(i) = 0.0;
		curve_corr_p_I->X.at(i) = time;
		curve_corr_p_I->Y.at(i) = 0.0;
		curve_corr_l_I->X.at(i) = time;
		curve_corr_l_I->Y.at(i) = 0.0;
		curve_corr_e_Q->X.at(i) = time;
		curve_corr_e_Q->Y.at(i) = 0.0;
		curve_corr_p_Q->X.at(i) = time;
		curve_corr_p_Q->Y.at(i) = 0.0;
		curve_corr_l_Q->X.at(i) = time;
		curve_corr_l_Q->Y.at(i) = 0.0;
		curve_cn0->X.at(i) = time;
		curve_cn0->Y.at(i) = 0;
	}

	plot_corrs_I = new snPlotComplexRealImag("Inphase code corr. results", snPlot::Linear,
			curve_corr_e_I);
	plot_corrs_I->setXAxis(0.0, expected_lenth);
	plot_corrs_I->setXAxisLabel("t [s]");
	plot_corrs_I->setYAxisLabel("Correlation");

	plot_corrs_I->enableGrid();
	plot_corrs_I->addCurve(curve_corr_p_I);
	plot_corrs_I->addCurve(curve_corr_l_I);
	snPlotComplexRealImag *plot_corrs_Q = new snPlotComplexRealImag("Quad. code corr. results",
			snPlot::Linear, curve_corr_e_Q);
	plot_corrs_Q->setXAxis(0.0, expected_lenth);
	plot_corrs_Q->setXAxisLabel("t [s]");
	plot_corrs_Q->setYAxisLabel("Correlation");

	plot_corrs_Q->enableGrid();
	plot_corrs_Q->addCurve(curve_corr_p_Q);
	plot_corrs_Q->addCurve(curve_corr_l_Q);
	trkw4->addPlot(plot_corrs_I);
	trkw4->addPlot(plot_corrs_Q);

	// setup C/N0 plot:
	snPlotReal *plot_cn0 = new snPlotReal("C/N0 estimation", snPlot::Log10, curve_cn0);
	plot_cn0->setXAxis(0.0, expected_lenth);
	plot_cn0->setXAxisLabel("t [s]");
	plot_cn0->setYAxisLabel("C/N_0 [dBHz]");
	plot_cn0->enableGrid();
	plot_cn0->setYAxis(0, 60);

	trkw4->addPlot(plot_cn0);

	// setup incoming signal and carrier replica
	curve_incoming = new snCurveReal(plot_x_max, Qt::blue);
	snPlotReal *plot_incoming = new snPlotReal("incoming signal", snPlot::Linear, curve_incoming);
	plot_incoming->setXAxis(0.0, Sig.plot_buffer_length);
	plot_incoming->setXAxisLabel("t [s]");
	plot_incoming->setYAxisLabel("A");
	trkw->addPlot(plot_incoming);

	curve_carrier_replica = new snCurveComplex(plot_x_max, Qt::blue, Qt::red);
	snPlotComplexRealImag *plot_carrier_replica = new snPlotComplexRealImag("carrier replica",
			snPlot::Linear, curve_carrier_replica);
	plot_carrier_replica->setXAxis(0.0, Sig.plot_buffer_length);
	plot_carrier_replica->setXAxisLabel("t [s]");
	plot_carrier_replica->setYAxisLabel("A");
	trkw->addPlot(plot_carrier_replica);

	// setup downmixed signal curves
	curve_downmixed = new snCurveComplex(plot_x_max, Qt::blue, Qt::red);
	snPlotComplexRealImag *plot_downmixed_signal = new snPlotComplexRealImag("signal mixed down",
			snPlot::Linear, curve_downmixed);
	plot_downmixed_signal->setXAxis(0.0, Sig.plot_buffer_length);
	plot_downmixed_signal->setXAxisLabel("t [s]");
	plot_downmixed_signal->setYAxisLabel("A");
	trkw->addPlot(plot_downmixed_signal);

	// setup code replica curves inphase
	curve_code_e_I = new snCurveReal(plot_x_max, Qt::green);
	curve_code_p_I = new snCurveReal(plot_x_max, Qt::red);
	curve_code_l_I = new snCurveReal(plot_x_max, Qt::blue);
	snPlotReal *plot_code_e_I = new snPlotReal("early/prompt/late inphase code replicas",
			snPlot::Linear, curve_code_e_I);
	plot_code_e_I->addCurve(curve_code_p_I);
	plot_code_e_I->addCurve(curve_code_l_I);

	plot_code_e_I->setXAxis(0.0, Sig.plot_buffer_length);
	plot_code_e_I->setXAxisLabel("t [s]");
	plot_code_e_I->setYAxisLabel("A");

	trkw->addPlot(plot_code_e_I);

	// setup code replica curves quadrature
	curve_code_e_Q = new snCurveReal(plot_x_max, Qt::green);
	curve_code_p_Q = new snCurveReal(plot_x_max, Qt::red);
	curve_code_l_Q = new snCurveReal(plot_x_max, Qt::blue);
	snPlotReal *plot_code_e_Q = new snPlotReal("early/prompt/late quadrature code replicas",
			snPlot::Linear, curve_code_e_Q);
	plot_code_e_Q->addCurve(curve_code_p_Q);
	plot_code_e_Q->addCurve(curve_code_l_Q);

	plot_code_e_Q->setXAxis(0.0, Sig.plot_buffer_length);
	plot_code_e_Q->setXAxisLabel("t [s]");
	plot_code_e_Q->setYAxisLabel("A");

	trkw->addPlot(plot_code_e_Q);

	// setup curve pll, dll outputs
	curve_pll_raw_I = new snCurveReal(expected_corr_intervals, Qt::blue);
	curve_pll_raw_Q = new snCurveReal(expected_corr_intervals, Qt::red);
	curve_pll_filtered = new snCurveReal(expected_corr_intervals, Qt::blue);
	curve_dll_raw_I = new snCurveReal(expected_corr_intervals, Qt::blue);
	curve_dll_raw_Q = new snCurveReal(expected_corr_intervals, Qt::red);
	curve_dll_raw_IQ = new snCurveReal(expected_corr_intervals, Qt::green);
	curve_dll_filtered = new snCurveReal(expected_corr_intervals, Qt::blue);
	curve_remcodephase = new snCurveReal(expected_corr_intervals, Qt::blue);
	snPlotReal *plot_pll_raw = new snPlotReal("PLL raw output", snPlot::Linear, curve_pll_raw_I);
	if (Q_enable) {
		plot_pll_raw->addCurve(curve_pll_raw_Q);
	}
	plot_pll_raw->setXAxis(0.0, expected_lenth);
	plot_pll_raw->setXAxisLabel("t [s]");
	plot_pll_raw->setYAxisLabel("");
	plot_pll_raw->enableGrid();
	snPlotReal *plot_pll_filtered = new snPlotReal("PLL filtered output", snPlot::Linear,
			curve_pll_filtered);
	plot_pll_filtered->setXAxis(0.0, expected_lenth);
	plot_pll_filtered->setXAxisLabel("t [s]");
	plot_pll_filtered->setYAxisLabel("");
	plot_pll_filtered->enableGrid();
	snPlotReal *plot_dll_raw = new snPlotReal("DLL raw output", snPlot::Linear, curve_dll_raw_I);
	if (Q_enable) {
		plot_dll_raw->addCurve(curve_dll_raw_Q);
		plot_dll_raw->addCurve(curve_dll_raw_IQ);
	}
	plot_dll_raw->setXAxis(0.0, expected_lenth);
	plot_dll_raw->setXAxisLabel("t [s]");
	plot_dll_raw->setYAxisLabel("");
	plot_dll_raw->enableGrid();
	snPlotReal *plot_dll_filtered = new snPlotReal("DLL filtered output", snPlot::Linear,
			curve_dll_filtered);
	plot_dll_filtered->setXAxis(0.0, expected_lenth);
	plot_dll_filtered->setXAxisLabel("t [s]");
	plot_dll_filtered->setYAxisLabel("");
	plot_dll_filtered->enableGrid();
	snPlotReal *plot_remcodephase = new snPlotReal("code phase remainder", snPlot::Linear,
			curve_remcodephase);
	plot_remcodephase->setXAxis(0.0, expected_lenth);
	plot_remcodephase->setXAxisLabel("t [s]");
	plot_remcodephase->setYAxisLabel("");
	plot_remcodephase->enableGrid();
	trkw3->addPlot(plot_pll_raw);
	trkw3->addPlot(plot_pll_filtered);
	trkw3->addPlot(plot_dll_raw);
	trkw3->addPlot(plot_dll_filtered);
	trkw3->addPlot(plot_remcodephase);

	// set up correlation function samples
	curve_corrfunc1_I = new snCurveReal(SDR.CorrFuncSamples, Qt::blue);
	curve_corrfunc1_I->setCurveSymbol(snCurve::CURVEI, QwtSymbol(QwtSymbol::Ellipse, QBrush(
			Qt::blue), QPen(Qt::blue), QSize(8, 8)));
	curve_corrfunc2_I = new snCurveReal(3, Qt::blue);
	curve_corrfunc2_I->setCurveSymbol(snCurve::CURVEI, QwtSymbol(QwtSymbol::Ellipse, QBrush(
			Qt::blue), QPen(Qt::blue), QSize(8, 8)));

	curve_corrfunc1_Q = new snCurveReal(SDR.CorrFuncSamples, Qt::red);
	curve_corrfunc1_Q->setCurveSymbol(snCurve::CURVEI, QwtSymbol(QwtSymbol::Ellipse,
			QBrush(Qt::red), QPen(Qt::red), QSize(8, 8)));
	curve_corrfunc2_Q = new snCurveReal(3, Qt::red);
	curve_corrfunc2_Q->setCurveSymbol(snCurve::CURVEI, QwtSymbol(QwtSymbol::Ellipse,
			QBrush(Qt::red), QPen(Qt::red), QSize(8, 8)));

	plot_corrfunc1 = new snPlotReal("corr. func. samples", snPlot::Linear,
			curve_corrfunc1_I);
	plot_corrfunc1->setXAxisLabel("chips");
	plot_corrfunc1->enableGrid();
	plot_corrfunc1->setXAxis(SDR.CorrFuncStart, SDR.CorrFuncStart + SDR.CorrFuncSamples
			* SDR.CorrFuncDist);
	plot_corrfunc2 = new snPlotReal("E-P-L corr. func. samples", snPlot::Linear,
			curve_corrfunc2_I);
	plot_corrfunc2->setXAxisLabel("chips");
	plot_corrfunc2->enableGrid();
	plot_corrfunc2->setXAxis(SDR.CorrFuncStart, SDR.CorrFuncStart + SDR.CorrFuncSamples
			* SDR.CorrFuncDist);

	plot_corrfunc1->addCurve(curve_corrfunc1_Q);
	plot_corrfunc2->addCurve(curve_corrfunc2_Q);
	trkw2->addPlot(plot_corrfunc1);
	trkw2->addPlot(plot_corrfunc2);

	// pseudorange result plot:
	curve_pr = new snCurveReal(expected_corr_intervals, Qt::blue);
	plot_pr = new snPlotReal("pseudorange result", snPlot::Linear, curve_pr);
	plot_pr->setXAxis(0.0, expected_lenth);
	plot_pr->setXAxisLabel("t [s]");
	plot_pr->setYAxisLabel("d [m]");
	plot_pr->enableGrid();
	trkw2->addPlot(plot_pr);

	// init result curves:
	for (int i = 0; i < expected_corr_intervals; i++) {
		const double time = i * correlation_length;
		curve_pll_raw_I->X.at(i) = time;
		curve_pll_raw_I->Y.at(i) = 0.0;
		curve_pll_raw_Q->X.at(i) = time;
		curve_pll_raw_Q->Y.at(i) = 0.0;
		curve_pll_filtered->X.at(i) = time;
		curve_pll_filtered->Y.at(i) = 0.0;
		curve_dll_raw_I->X.at(i) = time;
		curve_dll_raw_I->Y.at(i) = 0.0;
		curve_dll_raw_Q->X.at(i) = time;
		curve_dll_raw_Q->Y.at(i) = 0.0;
		curve_dll_raw_IQ->X.at(i) = time;
		curve_dll_raw_IQ->Y.at(i) = 0.0;
		curve_dll_filtered->X.at(i) = time;
		curve_dll_filtered->Y.at(i) = 0.0;
		curve_remcodephase->X.at(i) = time;
		curve_remcodephase->Y.at(i) = 0.0;
		curve_pr->X.at(i) = time;
		curve_pr->Y.at(i) = 0.0;
	}
}

double snSDRStandard::correlation(const std::vector<double> &a, const std::vector<double> &b) {
	size_t len_a = a.size();
	if (len_a != b.size()) {
		throw snSimulationException("snSDRGPS::correlation: error: vectors have different lengths.");
	}

	double out = 0.0;
	for (size_t i = 0; i < len_a; i++) {
		out += a[i] * b[i];
	}
	return out;// / static_cast<double> (len_a);
}

std::vector<double> snSDRStandard::build_code(const std::vector<double> &indices, snModulation *m) {
	size_t len = indices.size();
	std::vector<double> out(len);

	for (size_t i = 0; i < len; i++) {
		out[i] = m->get_value_for_absolute_sample_number(round(indices[i]));
	}
	return out;
}

std::vector<double> snSDRStandard::build_shifted_code(double shift, snModulation *m,
		Samples_Type samples) {
	std::vector<double> out(samples);

	const double samples_per_chip = m->get_samples_per_chip();
	const double samples_per_code = m->get_samples_per_code();

	for (size_t i = 0; i < samples; i++) {
		double ind_d = shift * samples_per_chip + static_cast<double> (i);
		if (ind_d < 0) {
			ind_d += samples_per_code;
		}
		out[i] = m->get_value_for_absolute_sample_number(round(ind_d));
	}

	return out;
}

double snSDRStandard::determine_gain_of_code(snModulation *m, double el_spacing) {

	const Samples_Type samples = m->get_samples_per_code();
	// we go .1 chips back and forth around 0 to determine the gain:
	//	const std::vector<double> early = build_shifted_code(-0.01, m, samples);
	//	const std::vector<double> late = build_shifted_code(0.01, m, samples);

	const std::vector<double> prompt = build_shifted_code(0.0, m, samples);

	// extremes: early is 2* -el_spacing/2.0 away from prompt

	std::vector<double> early = build_shifted_code(2.0 * (-el_spacing / 2.0), m, samples);
	std::vector<double> late = build_shifted_code(0.0, m, samples);

	const double ec_1 = correlation(early, prompt) / static_cast<double> (samples);
	const double lc_1 = correlation(late, prompt) / static_cast<double> (samples);

	early = build_shifted_code(0.0, m, samples);
	late = build_shifted_code(2.0 * (+el_spacing / 2.0), m, samples);

	const double ec_2 = correlation(early, prompt) / static_cast<double> (samples);
	const double lc_2 = correlation(late, prompt) / static_cast<double> (samples);

	double d_out_1 = discriminator_early_minus_late_power_normalized(ec_1, 0, lc_1, 0);
	double d_out_2 = discriminator_early_minus_late_power_normalized(ec_2, 0, lc_2, 0);
	/*
	 std::cout << "d_out_1: " << d_out_1 << ", ec_1: " << ec_1 << ", lc_1: " << lc_1 << ", space: "
	 << el_spacing << "\n";
	 std::cout << "d_out_2: " << d_out_2 << ", ec_2: " << ec_2 << ", lc_2: " << lc_2 << ", space: "
	 << el_spacing << "\n";
	 */
	//const double gain = (lc - (-ec)) / (el_spacing);
	const double gain = (d_out_2 - d_out_1) / (el_spacing);

	return gain;
}
