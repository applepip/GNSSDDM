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

/*
TODO:
- Improve tracking loops
*/

#include <string>

#include "snSDRStandard.h"
#include "../snGNSS/snCN0_Estimators/snCN0_Estimator_Standard.h"
#include "../snGNSS/snCN0_Estimators/snCN0_Estimator_Moment_Method.h"
#include "../snGNSS/snCN0_Estimators/snCN0_Estimator_Beaulieus_Method.h"

#include "../snSimulation/snParallelCorrelation.h"
#include "snParallelBuildCode.h"

#include <QtConcurrentRun>

#include <itpp/signal/filter.h>

#ifdef HAVE_ACML
#include "acml_mv.h"
#endif

#define USEPARALLELISM

const bool debugflag = false;
//const bool debugflag = true;

/**
 * \brief SDRGPS constructor
 * @param pSNSignal1 struct SNSignal with general parameters
 * @param pSNSDR1 SDR parameter set
 * @param psnw pointer to snWidget
 * @param pConcBuf1 input ConcBuf
 * @param pBufSize buffer size
 */
snSDRStandard::snSDRStandard(libconfig::Setting &Settings, SNSignal pSNSignal, ProcessingParameters &pProcSig,
		widget_factory_base *widget_factory, ConcBuf3* _circ_buf) :
		snSDR(pSNSignal, _circ_buf), ProcSig(pProcSig) {

	Sig.results_file->createGroup("snSDRGPS");
	Sig.results_file->createGroup("snSDRGPS/parameters");

	// read parameters:
	intermed_freq = Settings["intermediate_frequency"];
	if (intermed_freq < 0) {
		throw snSimulationException("snSDRGPS: intermediate_frequency must be >= 0.");
	} else if (intermed_freq > 0)
		enable_downmixing = true;
	else
		enable_downmixing = false;

	I_enable = false;
	Q_enable = false;

	cur_block = 0;

	gnss_signal = new snGNSSSignal(Settings["signal"], smpl_freq, signal_length);
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

	correlation_length = Settings["Tracking"]["DLL"]["correlation_length"];
	Sig.results_file->write("snSDRGPS/parameters/tracking/correlation_length", correlation_length);

	// DLL parameters
	Sig.results_file->createGroup("snSDRGPS/parameters/tracking/dll");

	SDR.DiscrType = static_cast<const char *>(Settings["Tracking"]["DLL"]["discriminator_type"]);
	Sig.results_file->write("snSDRGPS/parameters/tracking/dll/discriminator_type", SDR.DiscrType);

	SDR.TrkDLLDampRatio = Settings["Tracking"]["DLL"]["damping_ratio"]; // DLL damping ration
	Sig.results_file->write("snSDRGPS/parameters/tracking/dll/damping_ratio", SDR.TrkDLLDampRatio);

	SDR.TrkDLLNoiseBW = Settings["Tracking"]["DLL"]["noise_bandwidth"]; // DLL noise bandwidth
	Sig.results_file->write("snSDRGPS/parameters/tracking/dll/noise_bandwidth", SDR.TrkDLLNoiseBW);

	SDR.TrkEarlyLateSpacing = Settings["Tracking"]["DLL"]["early_late_spacing"];
	Sig.results_file->write("snSDRGPS/parameters/tracking/dll/early_late_spacing", SDR.TrkEarlyLateSpacing);

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

	aq_samples_number = static_cast<int>(SDR.AqLen * smpl_freq); // 100 000 samples
	Sig.results_file->write("snSDRGPS/parameters/aquisition/aq_samples_number", aq_samples_number);

	// init serial buffer
	InitSerialBuffer(20 * aq_samples_number + block_size);

	SDR.aq_plot_code_phases = 500; // amount of samples for code phases in aq plot

	snLog("snSDR: Processing delay is: " + QString::number(ProcSig.ProcessingDelay));
	Sig.results_file->write("snSDRGPS/parameters/processing_delay", ProcSig.ProcessingDelay);

	// check if correlation length is integer multiple of code length?

	// correlation_length_factor is integer multiple of one code length
	correlation_length_factor = round(correlation_length / gnss_code_I->get_code_length_time());
	snLog("snSDRStandard: correlation_length_factor: " + QString::number(correlation_length_factor));

	if (!dbl_eq(correlation_length / gnss_code_I->get_code_length_time(), correlation_length_factor)) {
		std::cout << "(correlation_length / gnss_code->get_code_length_time()):"
				<< (correlation_length / gnss_code_I->get_code_length_time()) << "\n";
		throw snSimulationException(
				QString(
						"snSDR: error: correlation length ("
								+ QString::number(gnss_code_I->get_code_length_time())
								+ ") is not a multiple of "
								+ QString::number(gnss_code_I->get_code_length_time()) + "!").toStdString());
	}

	snLog(
			"snSDR: correlation length = " + QString::number(correlation_length_factor) + " * "
					+ QString::number(gnss_code_I->get_code_length_time()), snModule::debug);

//	correlation_length = gnss_code_I->get_code_length_time(); wrong!

	expected_corr_intervals = static_cast<int>(signal_length * (1.0 / gnss_code_I->get_code_length_time())
			* 1.1 / correlation_length_factor);

	// create HDF5 group for results:
	Sig.results_file->createGroup("snSDRGPS/output");
	// create HDF5 datasets for results:
	ds_pll_raw = Sig.results_file->initialize_dataset("snSDRGPS/output/pll_raw", expected_corr_intervals);

	ds_pll_filtered = Sig.results_file->initialize_dataset("snSDRGPS/output/pll_filtered",
			expected_corr_intervals);

	ds_dll_raw = Sig.results_file->initialize_dataset("snSDRGPS/output/dll_raw", expected_corr_intervals);

	ds_dll_filtered = Sig.results_file->initialize_dataset("snSDRGPS/output/dll_filtered",
			expected_corr_intervals);

	ds_remcodephase = Sig.results_file->initialize_dataset("snSDRGPS/output/remcodephase",
			expected_corr_intervals);

	ds_pseudoranges = Sig.results_file->initialize_dataset("snSDRGPS/output/pseudoranges",
			expected_corr_intervals);

	ds_corr_early = Sig.results_file->initialize_dataset("snSDRGPS/output/corr_early",
			expected_corr_intervals);

	ds_corr_prompt = Sig.results_file->initialize_dataset("snSDRGPS/output/corr_prompt",
			expected_corr_intervals);

	ds_corr_late = Sig.results_file->initialize_dataset("snSDRGPS/output/corr_late", expected_corr_intervals);

	ds_c_n0 = Sig.results_file->initialize_dataset("snSDRGPS/output/c_n0", expected_corr_intervals);

	snw = widget_factory->get_new_widget("SDR1");
	trkw = widget_factory->get_new_widget("SDR2");
	trkw4 = widget_factory->get_new_widget("SDR3");
	trkw3 = widget_factory->get_new_widget("SDR4");
	trkw2 = widget_factory->get_new_widget("SDR5");
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
		//		itpp::vec aq_signal = gnss_code->OneCodeBlkSampled(smpl_freq);
		itpp::vec aq_signal = modulation_I->get_one_code_block_sampled();

		snLog("prn: " + QString::number(prn) + ", samples per Code " + QString::number(aq_signal.size()),
				snModule::info);
		double samplesPerChip = modulation_I->get_samples_per_chip();
		do {

			aq_success = Aquisition(aq_signal, samplesPerChip, 0.011, aq_frequency, aq_code_phase, snw,
					curve_aq);
			if (!aq_success) {
				snLog("snSDRGPS: aquisition unsuccessful.");
			}
		} while (!aq_success);

		// go back to the sample, where the aquisitioned code starts:
		set_next_sample_index(aq_code_phase);
	} else {
		aq_frequency = intermed_freq;

		aq_code_phase = SDR.StartTime * smpl_freq;
		aq_code_phase += ProcSig.ProcessingDelay * smpl_freq; // add nof samples to compensate for processing delay

		// start tracking
		snLog("snSDRGPS: Not performing aquisition.", snModule::info);

		// read all samples before the signal starts.
		if (debugflag)
			std::cout << "aq_code_phase: " << aq_code_phase << "\n";

		get_next_samples(aq_code_phase);
	}

	snLog(
			"snSDRGPS: Aquisitioned frequency: " + QString::number(aq_frequency)
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
		code_freq_basis_I = gnss_code_I->get_code_freq_cps() / correlation_length_factor; // scaled with correlation_length_factor to account for correlation periods longer than 1 code length
		code_freq_I = code_freq_basis_I; // will get changed by code loop
		samples_per_code_basis_I = modulation_I->get_samples_per_code() * correlation_length_factor;
		code_len_I = gnss_code_I->get_number_of_chips_per_code() * correlation_length_factor;
	}
	if (Q_enable) {
		code_freq_basis_Q = gnss_code_Q->get_code_freq_cps() / correlation_length_factor;
		code_freq_Q = code_freq_basis_Q; // will get changed by code loop
		samples_per_code_basis_Q = modulation_Q->get_samples_per_code() * correlation_length_factor;
		code_len_Q = gnss_code_Q->get_number_of_chips_per_code() * correlation_length_factor;
	}

	// adjust parameters according to correlation length:
	//code_len *= correlation_length_factor;
	double early_late_spacing = SDR.TrkEarlyLateSpacing;
	Samples_Type raw_signal_max = round(SDR.AqLen * smpl_freq * 1.1 * correlation_length_factor); // max buffer size for correlation buffers

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
	snLog(
			"snSDRGPS: code gain inphase: " + QString::number(gain_I) + ", code gain quad.: "
					+ QString::number(gain_Q));

	double carr_freq = aq_frequency;
	double carr_freq_basis = aq_frequency;
	double carrier_phase_remainder = 0.0;

	double prev_code_nco = 0.0;
	double prev_code_error = 0.0;
	double code_phase_remainder = 0.0;

	double prev_carr_nco = 0.0;
	double prev_carr_error = 0.0;

	double triarg = 0;

	double carr_error_I = 0.0, carr_error_Q = 0.0, carr_nco, tau2carr, tau1carr, PDIcarr = correlation_length;
	double code_error_I = 0.0, code_error_Q = 0.0, code_nco, tau2code, tau1code, PDIcode = correlation_length;

	calculate_loop_coefficients(tau1code, tau2code, SDR.TrkDLLNoiseBW, SDR.TrkDLLDampRatio, 1.0);
	calculate_loop_coefficients(tau1carr, tau2carr, SDR.TrkPLLNoiseBW, SDR.TrkPLLDampRatio, 0.25);

	snLog(
			"snSDRGPS: code loop parameters: tau1code: " + QString::number(tau1code) + ", tau2code: "
					+ QString::number(tau2code), snModule::debug);
	snLog(
			"snSDRGPS: carrier loop parameters: tau1carr: " + QString::number(tau1carr) + ", tau2carr: "
					+ QString::number(tau2carr), snModule::debug);

	unsigned long current_trk_point = 0;

	double actual_pseudo_range = 0;

	// setup C/N0 estimator:
	snCN0_Estimator_Standard CN0_estimator(correlation_length, 100);

	// setup moment method SNR estimator:
	itpp::cvec correlation_result_prompt(expected_corr_intervals);
	snCN0_Estimator_Moment_Method CN0_estimator_MM(100);

	// setup Baulieu's SNR estimator:
	snCN0_Estimator_Beaulieus_Method CN0_estimator_BL(100);

	// low pass filter coefficients for filtering after downsampling:
	const size_t down_smpl_len = 33;
	const itpp::vec down_smpl_coeffs = itpp::fir1(down_smpl_len, .5);
	itpp::vec down_smpl_state_r(down_smpl_coeffs.size());
	itpp::vec down_smpl_state_i(down_smpl_coeffs.size());
	/*
	 itpp::vec down_smpl_state_in_r(down_smpl_coeffs.size());
	 itpp::vec down_smpl_state_in_i(down_smpl_coeffs.size());
	 itpp::vec tmp_sig_in((down_smpl_coeffs.size()-1)/2);
	 itpp::filter(down_smpl_coeffs, 1, tmp_sig_in, down_smpl_state_in_r, down_smpl_state_r);
	 itpp::filter(down_smpl_coeffs, 1, tmp_sig_in, down_smpl_state_in_i, down_smpl_state_i);

	 std::cout << "state after init: " << down_smpl_state_r << ", " << down_smpl_state_i << "\n";
	 */
	while (1) {
		// Tracking //

		if (I_enable) {
			modulation_I->get_code()->set_freq_cps(code_freq_I * correlation_length_factor);
		}
		if (Q_enable) {
			modulation_Q->get_code()->set_freq_cps(code_freq_I * correlation_length_factor);
		}

		const double samples_per_chip = modulation_I->get_samples_per_chip();
		const double samples_per_code = modulation_I->get_samples_per_code() * correlation_length_factor;

		const Samples_Type blksize = ceil(
				(static_cast<double>(code_len_I) - code_phase_remainder) * samples_per_chip);
		// we don't need to scale with * correlation_length_factor here, because code_len was scaled already.

		snLog("snSDRStandard: blksize: " + QString::number(blksize), snModule::debug);

		if (debugflag)
			std::cout << "blksize: " << blksize << "\n";

		if (blksize > raw_signal_max)
			throw snSimulationException(
					QString(
							"snSDRGPS: rawSignal[] too small. blksize: " + QString::number(blksize)
									+ ",  rawSignalMax: " + QString::number(raw_signal_max)).toStdString());

		// init tracking

		itpp::cvec rawSignal_cmplx = get_next_samples(blksize);

		// stop if we got less samples than blksize:
		if (static_cast<Samples_Type>(rawSignal_cmplx.size()) != blksize) {
			// simulation is over now.
			snLog("snSDR: exit.");
			printSummary("snSDR");
			return;
		}

		const itpp::vec rawSignal_real = itpp::real(rawSignal_cmplx);

		itpp::vec early_code_I(blksize);
		itpp::vec prompt_code_I(blksize);
		itpp::vec late_code_I(blksize);
		itpp::vec early_code_Q(blksize);
		itpp::vec prompt_code_Q(blksize);
		itpp::vec late_code_Q(blksize);

#ifdef USEPARALLELISM
		QFuture<itpp::vec> f_e_I, f_p_I, f_l_I;
		QFuture<itpp::vec> f_e_Q, f_p_Q, f_l_Q;

		itpp::vec i_indices(blksize);
		int count = 0;
		for (double i = 0; i < blksize; i += 1.0) {
			i_indices[count++] = i;
		}

		// indices early:
		const itpp::vec ind_d_e = (code_phase_remainder - early_late_spacing / 2.0) * samples_per_chip
				+ samples_per_code + i_indices;
		//ind_d_e += samples_per_code;

		// indices late:
		const itpp::vec ind_d_l = (code_phase_remainder + early_late_spacing / 2.0) * samples_per_chip
				+ i_indices;

		// indices prompt:
		const itpp::vec ind_d_p = code_phase_remainder * samples_per_chip + i_indices;

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
			// build early code:
			double ind_d = (code_phase_remainder - early_late_spacing / 2.0)
			* samples_per_chip + static_cast<double> (i);
			if (ind_d < 0) {
				//std::cout << "ind_d < 0 : " << ind_d << ", code_len: " << code_len << "\n";
				ind_d += samples_per_code;
			}
			if (I_enable) {
				early_code_I[i]
				= modulation_I->get_value_for_absolute_sample_number(
						round(ind_d));
			}
			if (Q_enable) {
				early_code_Q[i]
				= modulation_Q->get_value_for_absolute_sample_number(
						round(ind_d));

			}

			// build late code:
			ind_d = (code_phase_remainder + early_late_spacing / 2.0)
			* samples_per_chip + static_cast<double> (i);
			if (I_enable) {
				late_code_I[i]
				= modulation_I->get_value_for_absolute_sample_number(
						round(ind_d));
			}
			if (Q_enable) {
				late_code_Q[i]
				= modulation_Q->get_value_for_absolute_sample_number(
						round(ind_d));
			}

			// build prompt code:
			ind_d = code_phase_remainder * samples_per_chip
			+ static_cast<double> (i);
			if (I_enable) {
				prompt_code_I[i]
				= modulation_I->get_value_for_absolute_sample_number(
						round(ind_d));
			}
			if (Q_enable) {
				prompt_code_Q[i]
				= modulation_Q->get_value_for_absolute_sample_number(
						round(ind_d));
			}
		}
#endif

		// TODO numerical problems in the following formula. even in steady state,
		// code_phase_remainder is growing due to the addition.
		// once cpr is != 0 during the pull-in right at the beginning, ind_d grows and grows.
		double ind_d = code_phase_remainder + static_cast<double>(blksize) / samples_per_chip;
		code_phase_remainder = ind_d - static_cast<double>(code_len_I);

		//code_phase_remainder = 0.0;
		/*
		 std::cout.precision(20);
		 std::cout << "static_cast<double> (blksize): " << static_cast<double> (blksize)
		 << ", samples_per_chip: " << samples_per_chip << ", ind_d: " << ind_d
		 << ", code_phase_remainder: " << code_phase_remainder << "\n";
		 */

		const double twopif = (carr_freq * 2.0 * M_PI);
		//std::cout << "carr_freq: " << carr_freq << "\n";

		itpp::vec sin_vec(blksize), cos_vec(blksize);

		itpp::vec BBSig_I;
		itpp::vec BBSig_Q;
		if (enable_downmixing) {
			// generate carrier
			itpp::vec radiants(blksize);
			for (Samples_Type i = 0; i < blksize; i++) {
				radiants[i] = fmod((twopif * static_cast<double>(i) / smpl_freq) + carrier_phase_remainder,
						2.0 * M_PI);
			}

#ifdef HAVE_ACML
			// void vrda_sincos (int n, double *x, double *ys, double *yc)
			vrda_sincos(blksize, &radiants[0], &sin_vec[0], &cos_vec[0]);
#else
			cos_vec = itpp::cos(radiants);
			sin_vec = itpp::sin(radiants);
#endif

			// mix down to base band:
			BBSig_I = itpp::elem_mult(cos_vec, rawSignal_real);
			BBSig_Q = itpp::elem_mult(-1.0 * sin_vec, rawSignal_real);

			// low-pass filter:
			/* TODO: this introduces a processing delay. compensate.
			 itpp::vec down_smpl_state_out_r(down_smpl_coeffs.size());
			 itpp::vec down_smpl_state_out_i(down_smpl_coeffs.size());

			 BBSig_I = itpp::filter(down_smpl_coeffs, 1, BBSig_I,
			 down_smpl_state_r, down_smpl_state_out_r);
			 BBSig_Q = itpp::filter(down_smpl_coeffs, 1, BBSig_Q,
			 down_smpl_state_i, down_smpl_state_out_i);

			 down_smpl_state_r = down_smpl_state_out_r;
			 down_smpl_state_i = down_smpl_state_out_i;
			 */
		} else { // do not mix down:
			BBSig_I = itpp::real(rawSignal_cmplx);
			BBSig_Q = itpp::imag(rawSignal_cmplx);
		}

		// integer division does not round.  = fix(triarg/2*pi)
		triarg = (twopif * static_cast<double>(blksize) / smpl_freq) + carrier_phase_remainder;
		carrier_phase_remainder = triarg - ((int) (triarg / (2.0 * M_PI))) * (2.0 * M_PI);

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

		// init correlation
		double corr_early_inphase_code_I = 0.0, corr_early_inphase_code_Q = 0.0, corr_prompt_inphase_code_I =
				0.0, corr_prompt_inphase_code_Q = 0.0, corr_late_inphase_code_I = 0.0,
				corr_late_inphase_code_Q = 0.0;
		double corr_early_quadrature_code_I = 0.0, corr_early_quadrature_code_Q = 0.0,
				corr_prompt_quadrature_code_I = 0.0, corr_prompt_quadrature_code_Q = 0.0,
				corr_late_quadrature_code_I = 0.0, corr_late_quadrature_code_Q = 0.0;

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
		if (I_enable) {
			corr_early_inphase_code_I = itpp::elem_mult_sum(early_code_I,
					BBSig_I);
			corr_early_inphase_code_Q = itpp::elem_mult_sum(early_code_I,
					BBSig_Q);
			corr_prompt_inphase_code_I = itpp::elem_mult_sum(prompt_code_I,
					BBSig_I);
			corr_prompt_inphase_code_Q = itpp::elem_mult_sum(prompt_code_I,
					BBSig_Q);
			corr_late_inphase_code_I
			= itpp::elem_mult_sum(late_code_I, BBSig_I);
			corr_late_inphase_code_Q
			= itpp::elem_mult_sum(late_code_I, BBSig_Q);
		}
		if (Q_enable) {
			// for quadrature, the imag channel of BBSIg gives actually the
			// inphase result for the correlation. That's why, .imag() and
			// .real() are swapped here:
			corr_early_quadrature_code_I = itpp::elem_mult_sum(early_code_Q,
					BBSig_Q);
			corr_early_quadrature_code_Q = itpp::elem_mult_sum(early_code_Q,
					BBSig_I);
			corr_prompt_quadrature_code_I = itpp::elem_mult_sum(prompt_code_Q,
					BBSig_Q);
			corr_prompt_quadrature_code_Q = itpp::elem_mult_sum(prompt_code_Q,
					BBSig_I);
			corr_late_quadrature_code_I = itpp::elem_mult_sum(late_code_Q,
					BBSig_Q);
			corr_late_quadrature_code_Q = itpp::elem_mult_sum(late_code_Q,
					BBSig_I);
		}
#endif

#ifdef USEPARALLELISM
		// now we need the results for the inphase prompt codes:
		if (I_enable) {
			corr_prompt_inphase_code_I = future_piI.result();
			corr_prompt_inphase_code_Q = future_piQ.result();
		}
#endif

		correlation_result_prompt(current_trk_point) = std::complex<double>(corr_prompt_inphase_code_I,
				corr_prompt_inphase_code_Q);

		// get PLL error
		// carrier loop I discriminator:
		carr_error_I = costas_discriminator_atan(corr_prompt_inphase_code_I, corr_prompt_inphase_code_Q);

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
				code_error_I = (1.0 / gain_I)
						* discriminator_dot_product_normalized(corr_early_inphase_code_I,
								corr_early_inphase_code_Q, corr_prompt_inphase_code_I,
								corr_prompt_inphase_code_Q, corr_late_inphase_code_I,
								corr_late_inphase_code_Q);
			}
			if (Q_enable) {
				code_error_Q = (1.0 / gain_Q)
						* discriminator_dot_product_normalized(corr_early_quadrature_code_I,
								corr_early_quadrature_code_Q, corr_prompt_quadrature_code_I,
								corr_prompt_quadrature_code_Q, corr_late_quadrature_code_I,
								corr_late_quadrature_code_Q);
			}
		} else if (SDR.DiscrType == "EML") { // early minus late power normalized
			if (I_enable) {
				code_error_I = (1.0 / gain_I)
						* discriminator_early_minus_late_power_normalized(corr_early_inphase_code_I,
								corr_early_inphase_code_Q, corr_late_inphase_code_I,
								corr_late_inphase_code_Q);
			}
			if (Q_enable) {
				code_error_Q = (1.0 / gain_Q)
						* discriminator_early_minus_late_power_normalized(corr_early_quadrature_code_I,
								corr_early_quadrature_code_Q, corr_late_quadrature_code_I,
								corr_late_quadrature_code_Q);
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
		const double travel_time = static_cast<double>(absolute_sample) / samples_per_code_basis_I;
		//std::cout << "absolute_sample: " << absolute_sample
		//		<< ", travel_time: " << travel_time << "\n";
		//const double travel_time = static_cast<double> (absolute_sample) / modulation->get_samples_per_code();

		// PR calculation.
		const double fact1 = code_freq_basis_I / static_cast<double>(code_len_I);
		actual_pseudo_range = (travel_time - floor(travel_time)) * Sig.c0 / fact1;
		//std::cout << "absolute_sample: " << absolute_sample
		//		<< ", travel_time: " << travel_time
		//		<< ", (travel_time - floor(travel_time)): " << (travel_time
		//		- floor(travel_time)) << ", actual_pseudo_range: "
		//		<< actual_pseudo_range << "\n";
		actual_pseudo_range -= code_phase_remainder * (Sig.c0 / code_freq_basis_I); // also use code phase remainder to improve PR calculation

		// if no aquisition was done and the start of the tracking is defined in the config file,
		// we need to substract the processing delay that was introduced by previous filters:
		/*
		 if (!SDR.DoGPSAquisition) {
		 actual_pseudo_range -= ProcSig.ProcessingDelay * Sig.c0; // compensate for processing delays
		 }
		 */

		if ((travel_time - floor(travel_time)) > .5) {
			// pr is negative. add one code length in meter to avoid flipping to 300000m for gps e.g.:
			actual_pseudo_range -= Sig.c0 / fact1;
		}
		//actual_pseudo_range -= code_nco * (Sig.c0 / code_freq_basis_I);

		snLog("snSDRGPS: actualPr: " + QString::number(actual_pseudo_range, 'f', 2), snModule::debug);

		// plot the correlation results:
		const double time = current_trk_point * correlation_length;

		trkw4->pushData();
		// inphase
		if (I_enable) {
			curve_corr_e_I->update_value_X(current_trk_point, time);
			curve_corr_e_I->update_value_Y(current_trk_point,
					std::complex<double>(corr_early_inphase_code_I, corr_early_inphase_code_Q));
			curve_corr_e_I->setPointsToPlot(current_trk_point);
			curve_corr_p_I->update_value_X(current_trk_point, time);
			curve_corr_p_I->update_value_Y(current_trk_point,
					std::complex<double>(corr_prompt_inphase_code_I, corr_prompt_inphase_code_Q));
			curve_corr_p_I->setPointsToPlot(current_trk_point);
			curve_corr_l_I->update_value_X(current_trk_point, time);
			curve_corr_l_I->update_value_Y(current_trk_point,
					std::complex<double>(corr_late_inphase_code_I, corr_late_inphase_code_Q));
			curve_corr_l_I->setPointsToPlot(current_trk_point);

			// adjust axes of corr result plot
			int minindex = current_trk_point - 5000;
			if (minindex < 0)
				minindex = 0;
			double prmin = 200000;
			double prmax = 0;
			for (unsigned int k = minindex; k < current_trk_point; k++) {
				if (curve_corr_e_I->get_Y(k).real() > prmax)
					prmax = curve_corr_e_I->get_Y(k).real();
				if (curve_corr_e_I->get_Y(k).real() < prmin)
					prmin = curve_corr_e_I->get_Y(k).real();
			}
			for (unsigned int k = minindex; k < current_trk_point; k++) {
				if (curve_corr_l_I->get_Y(k).real() > prmax)
					prmax = curve_corr_l_I->get_Y(k).real();
				if (curve_corr_l_I->get_Y(k).real() < prmin)
					prmin = curve_corr_l_I->get_Y(k).real();
			}
			plot_corrs_I->setYAxis(prmin - 5, prmax + 5);
		}
		// quadrature:
		if (Q_enable) {
			curve_corr_e_Q->update_value_X(current_trk_point, time);
			curve_corr_e_Q->update_value_Y(current_trk_point,
					std::complex<double>(corr_early_quadrature_code_I, corr_early_quadrature_code_Q));
			curve_corr_p_Q->update_value_X(current_trk_point, time);
			curve_corr_p_Q->update_value_Y(current_trk_point,
					std::complex<double>(corr_prompt_quadrature_code_I, corr_prompt_quadrature_code_Q));
			curve_corr_l_Q->update_value_X(current_trk_point, time);
			curve_corr_l_Q->update_value_Y(current_trk_point,
					std::complex<double>(corr_late_quadrature_code_I, corr_late_quadrature_code_Q));
		}
		// plot C/N0:
		curve_cn0->update_value_X(current_trk_point, time);
		const double c_n0 = CN0_estimator.get_estimation(correlation_result_prompt, current_trk_point);
		curve_cn0->update_value_Y(current_trk_point, c_n0);
		curve_cn0->setPointsToPlot(current_trk_point);

		curve_cn0_mm->update_value_X(current_trk_point, time);
		curve_cn0_mm->update_value_Y(current_trk_point,
				CN0_estimator_MM.get_estimation(correlation_result_prompt, current_trk_point)
						* (1. / correlation_length));
		curve_cn0_mm->setPointsToPlot(current_trk_point);

		curve_cn0_bl->update_value_X(current_trk_point, time);
		curve_cn0_bl->update_value_Y(current_trk_point,
				CN0_estimator_BL.get_estimation(correlation_result_prompt, current_trk_point)
						* (1. / correlation_length));
		curve_cn0_bl->setPointsToPlot(current_trk_point);

		trkw4->releasePlotFlag();

		trkw3->pushData();
		curve_pll_raw_I->update_value_X(current_trk_point, time);
		curve_pll_raw_I->update_value_Y(current_trk_point, carr_error_I);
		curve_pll_raw_I->setPointsToPlot(current_trk_point);

		curve_pll_raw_Q->update_value_X(current_trk_point, time);
		curve_pll_raw_Q->update_value_Y(current_trk_point, carr_error_Q);
		curve_pll_raw_Q->setPointsToPlot(current_trk_point);

		curve_pll_filtered->update_value_X(current_trk_point, time);
		curve_pll_filtered->update_value_Y(current_trk_point, carr_nco);
		curve_pll_filtered->setPointsToPlot(current_trk_point);

		curve_dll_raw_I->update_value_X(current_trk_point, time);
		curve_dll_raw_I->update_value_Y(current_trk_point, code_error_I);
		curve_dll_raw_I->setPointsToPlot(current_trk_point);

		curve_dll_raw_Q->update_value_X(current_trk_point, time);
		curve_dll_raw_Q->update_value_Y(current_trk_point, code_error_Q);
		curve_dll_raw_Q->setPointsToPlot(current_trk_point);

		curve_dll_raw_IQ->update_value_X(current_trk_point, time);
		curve_dll_raw_IQ->update_value_Y(current_trk_point, code_error);
		curve_dll_raw_IQ->setPointsToPlot(current_trk_point);

		curve_dll_filtered->update_value_X(current_trk_point, time);
		curve_dll_filtered->update_value_Y(current_trk_point, code_nco);
		curve_dll_filtered->setPointsToPlot(current_trk_point);

		curve_remcodephase->update_value_X(current_trk_point, time);
		curve_remcodephase->update_value_Y(current_trk_point, code_phase_remainder);
		curve_remcodephase->setPointsToPlot(current_trk_point);

		trkw3->releasePlotFlag();

		// plot additional correlation function samples:
		itpp::vec CorrResults_I(SDR.CorrFuncSamples);
		itpp::vec CorrResults_Q(SDR.CorrFuncSamples);
		itpp::vec CorrFuncX(SDR.CorrFuncSamples);
		int plotting = trkw2->requestPlotFlag();
		if (SDR.WriteCorrFunc || plotting || do_write_txt_signals(cur_block)) {
			for (int o = 0; o < SDR.CorrFuncSamples; o++) {
				CorrFuncX(o) = SDR.CorrFuncStart + o * SDR.CorrFuncDist;
				if (I_enable) {
					itpp::vec CorrFuncCode = build_shifted_code(code_phase_remainder + CorrFuncX(o),
							modulation_I, blksize);
					CorrResults_I(o) = correlation(CorrFuncCode, BBSig_I);
				}
				if (Q_enable) {
					itpp::vec CorrFuncCode = build_shifted_code(code_phase_remainder + CorrFuncX(o),
							modulation_Q, blksize);
					CorrResults_Q(o) = correlation(CorrFuncCode, BBSig_Q);
				}
			}
			if (plotting) { // plot correlation function if needed:
				if (I_enable) {
					for (int o = 0; o < SDR.CorrFuncSamples; o++) {
						curve_corrfunc1_I->update_value_X(o, CorrFuncX(o));
						curve_corrfunc1_I->update_value_Y(o, CorrResults_I(o));
					}
					curve_corrfunc2_I->update_value_X(0, -SDR.TrkEarlyLateSpacing / 2.0);
					curve_corrfunc2_I->update_value_Y(0, corr_early_inphase_code_I);
					curve_corrfunc2_I->update_value_X(1, 0);
					curve_corrfunc2_I->update_value_Y(1, corr_prompt_inphase_code_I);
					curve_corrfunc2_I->update_value_X(2, SDR.TrkEarlyLateSpacing / 2.0);
					curve_corrfunc2_I->update_value_Y(2, corr_late_inphase_code_I);
				}
				if (Q_enable) {
					for (int o = 0; o < SDR.CorrFuncSamples; o++) {
						curve_corrfunc1_Q->update_value_X(o, CorrFuncX(o));
						curve_corrfunc1_Q->update_value_Y(o, CorrResults_Q(o));
					}
					curve_corrfunc2_Q->update_value_X(0, -SDR.TrkEarlyLateSpacing / 2.0);
					curve_corrfunc2_Q->update_value_Y(0, corr_early_quadrature_code_I);
					curve_corrfunc2_Q->update_value_X(1, .0);
					curve_corrfunc2_Q->update_value_Y(1, corr_prompt_quadrature_code_I);
					curve_corrfunc2_Q->update_value_X(2, SDR.TrkEarlyLateSpacing / 2.0);
					curve_corrfunc2_Q->update_value_Y(2, corr_late_quadrature_code_I);
				}

				double ymin_I = itpp::min(CorrResults_I);
				double ymax_I = itpp::max(CorrResults_I);
				plot_corrfunc1->setYAxis(ymin_I, ymax_I);
				plot_corrfunc2->setYAxis(ymin_I, ymax_I);

				// adjust axes of pr plot according to the last 50 values
				//size_t prend = current_trk_point;
				/*
				 size_t pstart;
				 if (current_trk_point < 50)
				 pstart = 0;
				 else
				 pstart = current_trk_point - 50;
				 */

				//double prmin = *std::min_element(curve_pr->Y.begin() + pstart, curve_pr->Y.begin() + prend);
				//double prmax = *std::max_element(curve_pr->Y.begin() + pstart, curve_pr->Y.begin() + prend);
				//std::cout << "prmax: " << prmax << ", prmin: " << prmin << "\n";
				//plot_pr->setYAxis(prmin - 7, prmax + 7);
				trkw2->releasePlotFlag();
			}

		}
		trkw2->pushData();
		curve_pr->update_value_X(current_trk_point, time);
		curve_pr->update_value_Y(current_trk_point, actual_pseudo_range);
		curve_pr->setPointsToPlot(current_trk_point);
		trkw2->releasePlotFlag();

		// plot Codes and incoming signals
		if (trkw->requestPlotFlag()) {
			if (enable_downmixing) {
				for (Samples_Type i = 0; i < get_nof_xaxis_indices(); i++) {
					const double time = static_cast<double>(i) / smpl_freq; // in s
					curve_carrier_replica->update_value_X(i, time);
					curve_carrier_replica->update_value_Y(i, std::complex<double>(cos_vec[i], sin_vec[i]));
					curve_downmixed->update_value_X(i, time);
					curve_downmixed->update_value_Y(i, std::complex<double>(BBSig_I[i], BBSig_Q[i]));
				}
			}

			for (Samples_Type i = 0; i < get_nof_xaxis_indices(); i++) {
				const double xtime = static_cast<double>(i) / smpl_freq; // in s
				curve_incoming->update_value_X(i, xtime);
				curve_incoming->update_value_Y(i, rawSignal_cmplx[i]);
				if (I_enable) {
					curve_code_e_I->update_value_X(i, xtime);
					curve_code_e_I->update_value_Y(i, early_code_I[i]);
					curve_code_p_I->update_value_X(i, xtime);
					curve_code_p_I->update_value_Y(i, prompt_code_I[i]);
					curve_code_l_I->update_value_X(i, xtime);
					curve_code_l_I->update_value_Y(i, late_code_I[i]);
				}
				if (Q_enable) {
					curve_code_e_Q->update_value_X(i, xtime);
					curve_code_e_Q->update_value_Y(i, early_code_Q[i]);
					curve_code_p_Q->update_value_X(i, xtime);
					curve_code_p_Q->update_value_Y(i, prompt_code_Q[i]);
					curve_code_l_Q->update_value_X(i, xtime);
					curve_code_l_Q->update_value_Y(i, late_code_Q[i]);
				}
			}
			trkw->releasePlotFlag();
		}

		// write results to file:
		Sig.results_file->write_to_dataset(ds_pll_raw, cur_block, carr_error_I);
		Sig.results_file->write_to_dataset(ds_pll_filtered, cur_block, carr_nco);
		Sig.results_file->write_to_dataset(ds_dll_raw, cur_block, code_error_I);
		Sig.results_file->write_to_dataset(ds_dll_filtered, cur_block, code_nco);
		Sig.results_file->write_to_dataset(ds_remcodephase, cur_block, code_phase_remainder);
		Sig.results_file->write_to_dataset(ds_pseudoranges, cur_block, actual_pseudo_range);
		// TODO: complex values
		/*
		 Sig.results_file->write_to_dataset(ds_corr_early, cur_block, );
		 Sig.results_file->write_to_dataset(ds_corr_prompt, cur_block, );
		 Sig.results_file->write_to_dataset(ds_corr_late, cur_block, );
		 */
		Sig.results_file->write_to_dataset(ds_c_n0, cur_block, c_n0);

		// should we write samples to a txt file?
		if (do_write_txt_signals(cur_block)) {
			txt_signal txt_file_early(Sig, "sdr_code_early.txt", cur_block);
			txt_signal txt_file_late(Sig, "sdr_code_late.txt", cur_block);
			// build time vector
			itpp::vec ts(Sig.nof_txt_samples);
			for (size_t k = 0; k < Sig.nof_txt_samples; k++)
				ts(k) = k * gnss_signal->get_smpl_interval();
			// early code vector:
			txt_file_early.write_txt_vectors(ts, early_code_I);
			// late code vector:
			txt_file_late.write_txt_vectors(ts, late_code_I);

			// write correlation points
			txt_signal txt_file_corr(Sig, "sdr_correlation_points.txt", cur_block);
			txt_file_corr.write_txt_vectors(CorrFuncX, CorrResults_I);

			// early, prompt, late, separately:
			itpp::vec corr_points(3);
			corr_points(0) = -SDR.TrkEarlyLateSpacing / 2.0;
			corr_points(1) = 0.0;
			corr_points(2) = SDR.TrkEarlyLateSpacing / 2.0;
			itpp::vec corr_result(3);
			corr_result(0) = corr_early_inphase_code_I;
			corr_result(1) = corr_prompt_inphase_code_I;
			corr_result(2) = corr_late_inphase_code_I;
			txt_signal txt_file_corr_points(Sig, "sdr_correlation_epl.txt", cur_block);
			txt_file_corr_points.write_txt_vectors(corr_points, corr_result);
		}

		cur_block++;
		current_trk_point++;
		// set the current simulation time
		set_current_simulation_time(
				circbuf->get_current_consumer_block() / static_cast<double>(circbuf->get_total_blocks())
						* signal_length);
	}
}

void snSDRStandard::setup_plots() {
	const double expected_lenth = expected_corr_intervals * correlation_length;

//	std::cout << "snSDR: expected correlation intervals: " << expected_corr_intervals
//			<< ", expected signal length: " << expected_lenth << "\n";

	// setup aqusition widget
	if (SDR.switch_plot_3d) {
		curve_aq = new snCurve3D(SDR.aq_plot_code_phases,
				static_cast<int>(floor(SDR.AqFreqBand / SDR.AqFreqStep)));
		snPlot *plot_aq = snw->get_new_plot_surface("Aquisition", snPlot::Linear, curve_aq);
		snw->addPlot(plot_aq);
	}

	// setup correlation result curves
	curve_corr_e_I = new snCurveComplex(expected_corr_intervals, snCurve::green, snCurve::darkGreen);
	curve_corr_p_I = new snCurveComplex(expected_corr_intervals, snCurve::red, snCurve::darkRed);
	curve_corr_l_I = new snCurveComplex(expected_corr_intervals, snCurve::blue, snCurve::darkBlue);

	curve_corr_e_Q = new snCurveComplex(expected_corr_intervals, snCurve::green, snCurve::darkGreen);
	curve_corr_p_Q = new snCurveComplex(expected_corr_intervals, snCurve::red, snCurve::darkRed);
	curve_corr_l_Q = new snCurveComplex(expected_corr_intervals, snCurve::blue, snCurve::darkBlue);

	curve_remcodephase = new snCurveReal(expected_corr_intervals, snCurve::blue);

	curve_cn0 = new snCurveReal(expected_corr_intervals, snCurve::blue, "Standard Method");
	curve_cn0_mm = new snCurveReal(expected_corr_intervals, snCurve::red, "Moment Method");
	curve_cn0_bl = new snCurveReal(expected_corr_intervals, snCurve::green, "Beaulieu Method");

	// init corr curves:
	for (size_t i = 0; i < curve_corr_e_I->get_max_points(); i++) {
		double time = i * correlation_length;
		curve_corr_e_I->update_value_X(i, time);
		curve_corr_e_I->update_value_Y(i, 0.0);
		curve_corr_p_I->update_value_X(i, time);
		curve_corr_p_I->update_value_Y(i, 0.0);
		curve_corr_l_I->update_value_X(i, time);
		curve_corr_l_I->update_value_Y(i, 0.0);
		curve_corr_e_Q->update_value_X(i, time);
		curve_corr_e_Q->update_value_Y(i, 0.0);
		curve_corr_p_Q->update_value_X(i, time);
		curve_corr_p_Q->update_value_Y(i, 0.0);
		curve_corr_l_Q->update_value_X(i, time);
		curve_corr_l_Q->update_value_Y(i, 0.0);
		curve_cn0->update_value_X(i, time);
		curve_cn0->update_value_Y(i, .0);
		curve_cn0_mm->update_value_X(i, time);
		curve_cn0_mm->update_value_Y(i, .0);
		curve_cn0_bl->update_value_X(i, time);
		curve_cn0_bl->update_value_Y(i, .0);
	}

	if (I_enable) {
		plot_corrs_I = trkw4->get_new_plot_complex("Inphase code corr. results", snPlot::Linear,
				curve_corr_e_I);
		plot_corrs_I->setXAxis(0.0, expected_lenth);
		plot_corrs_I->setXAxisLabel("t [s]");
		plot_corrs_I->setYAxisLabel("Correlation");

		plot_corrs_I->enableGrid();
		plot_corrs_I->addCurve(curve_corr_p_I);
		plot_corrs_I->addCurve(curve_corr_l_I);
		trkw4->addPlot(plot_corrs_I);
	}

	if (Q_enable) {
		snPlot *plot_corrs_Q = trkw4->get_new_plot_complex("Quad. code corr. results", snPlot::Linear,
				curve_corr_e_Q);
		plot_corrs_Q->setXAxis(0.0, expected_lenth);
		plot_corrs_Q->setXAxisLabel("t [s]");
		plot_corrs_Q->setYAxisLabel("Correlation");
		plot_corrs_Q->enableGrid();
		plot_corrs_Q->addCurve(curve_corr_p_Q);
		plot_corrs_Q->addCurve(curve_corr_l_Q);
		trkw4->addPlot(plot_corrs_Q);
	}
	// setup C/N0 plot:
	snPlot *plot_cn0 = trkw4->get_new_plot_real("C/N0 estimation", snPlot::Log10, curve_cn0);
	plot_cn0->addCurve(curve_cn0_mm);
	plot_cn0->addCurve(curve_cn0_bl);
	plot_cn0->setXAxis(0.0, expected_lenth);
	plot_cn0->setXAxisLabel("t [s]");
	plot_cn0->setYAxisLabel("C/N_0 [dBHz]");
	plot_cn0->enableGrid();
	plot_cn0->enableLegend();
	plot_cn0->setYAxis(0, 60);

	trkw4->addPlot(plot_cn0);

	// setup incoming signal
	curve_incoming = new snCurveComplex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
	snPlot *plot_incoming = trkw->get_new_plot_complex("incoming signal", snPlot::Linear, curve_incoming);

	plot_incoming->setXAxis(0.0, get_max_xaxis_value());
	plot_incoming->setXAxisLabel("t [s]");
	plot_incoming->setYAxisLabel("A");
	trkw->addPlot(plot_incoming);

//	for (size_t k = 0; k < get_nof_xaxis_indices(); k++) {
//		curve_incoming->update_value_X(k, .0);
//		curve_incoming->update_value_Y(k, .0);
//	}

	// carrier replica
	if (enable_downmixing) {
		curve_carrier_replica = new snCurveComplex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
		snPlot *plot_carrier_replica = trkw->get_new_plot_complex("carrier replica", snPlot::Linear,
				curve_carrier_replica);
		plot_carrier_replica->setXAxis(0.0, get_max_xaxis_value());
		plot_carrier_replica->setXAxisLabel("t [s]");
		plot_carrier_replica->setYAxisLabel("A");
		trkw->addPlot(plot_carrier_replica);

		// setup downmixed signal curves
		curve_downmixed = new snCurveComplex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
		snPlot *plot_downmixed_signal = trkw->get_new_plot_complex("signal mixed down", snPlot::Linear,
				curve_downmixed);
		plot_downmixed_signal->setXAxis(0.0, get_max_xaxis_value());
		plot_downmixed_signal->setXAxisLabel("t [s]");
		plot_downmixed_signal->setYAxisLabel("A");
		trkw->addPlot(plot_downmixed_signal);
	}

	// setup code replica curves inphase
	if (I_enable) {
		curve_code_e_I = new snCurveReal(get_nof_xaxis_indices(), snCurve::green);
		curve_code_p_I = new snCurveReal(get_nof_xaxis_indices(), snCurve::red);
		curve_code_l_I = new snCurveReal(get_nof_xaxis_indices(), snCurve::blue);
		snPlot *plot_code_e_I = trkw->get_new_plot_real("early/prompt/late inphase code replicas",
				snPlot::Linear, curve_code_e_I);
		plot_code_e_I->addCurve(curve_code_p_I);
		plot_code_e_I->addCurve(curve_code_l_I);

		plot_code_e_I->setXAxis(0.0, get_max_xaxis_value());
		plot_code_e_I->setXAxisLabel("t [s]");
		plot_code_e_I->setYAxisLabel("A");

		trkw->addPlot(plot_code_e_I);
	}

	// setup code replica curves quadrature
	if (Q_enable) {
		curve_code_e_Q = new snCurveReal(get_nof_xaxis_indices(), snCurve::green);
		curve_code_p_Q = new snCurveReal(get_nof_xaxis_indices(), snCurve::red);
		curve_code_l_Q = new snCurveReal(get_nof_xaxis_indices(), snCurve::blue);
		snPlot *plot_code_e_Q = trkw->get_new_plot_real("early/prompt/late quadrature code replicas",
				snPlot::Linear, curve_code_e_Q);
		plot_code_e_Q->addCurve(curve_code_p_Q);
		plot_code_e_Q->addCurve(curve_code_l_Q);

		plot_code_e_Q->setXAxis(0.0, get_max_xaxis_value());
		plot_code_e_Q->setXAxisLabel("t [s]");
		plot_code_e_Q->setYAxisLabel("A");

		trkw->addPlot(plot_code_e_Q);
	}
	// setup curve pll, dll outputs
	curve_pll_raw_I = new snCurveReal(expected_corr_intervals, snCurve::blue);
	curve_pll_raw_Q = new snCurveReal(expected_corr_intervals, snCurve::red);
	curve_pll_filtered = new snCurveReal(expected_corr_intervals, snCurve::blue);
	curve_dll_raw_I = new snCurveReal(expected_corr_intervals, snCurve::blue);
	curve_dll_raw_Q = new snCurveReal(expected_corr_intervals, snCurve::red);
	curve_dll_raw_IQ = new snCurveReal(expected_corr_intervals, snCurve::green);
	curve_dll_filtered = new snCurveReal(expected_corr_intervals, snCurve::blue);
	curve_remcodephase = new snCurveReal(expected_corr_intervals, snCurve::blue);
	snPlot *plot_pll_raw = trkw->get_new_plot_real("PLL raw output", snPlot::Linear, curve_pll_raw_I);
	if (Q_enable) {
		plot_pll_raw->addCurve(curve_pll_raw_Q);
	}
	plot_pll_raw->setXAxis(0.0, expected_lenth);
//	plot_pll_raw->setXAxisLabel("t [s]");
	plot_pll_raw->setYAxisLabel("");
	plot_pll_raw->enableGrid();
	snPlot *plot_pll_filtered = trkw3->get_new_plot_real("PLL filtered output", snPlot::Linear,
			curve_pll_filtered);
	plot_pll_filtered->setXAxis(0.0, expected_lenth);
//	plot_pll_filtered->setXAxisLabel("t [s]");
	plot_pll_filtered->setYAxisLabel("");
	plot_pll_filtered->enableGrid();
	snPlot *plot_dll_raw = trkw3->get_new_plot_real("DLL raw output", snPlot::Linear, curve_dll_raw_I);
	if (Q_enable) {
		plot_dll_raw->addCurve(curve_dll_raw_Q);
		plot_dll_raw->addCurve(curve_dll_raw_IQ);
	}
	plot_dll_raw->setXAxis(0.0, expected_lenth);
//	plot_dll_raw->setXAxisLabel("t [s]");
	plot_dll_raw->setYAxisLabel("");
	plot_dll_raw->enableGrid();
	snPlot *plot_dll_filtered = trkw3->get_new_plot_real("DLL filtered output", snPlot::Linear,
			curve_dll_filtered);
	plot_dll_filtered->setXAxis(0.0, expected_lenth);
//	plot_dll_filtered->setXAxisLabel("t [s]");
	plot_dll_filtered->setYAxisLabel("");
	plot_dll_filtered->enableGrid();
	snPlot *plot_remcodephase = trkw3->get_new_plot_real("code phase remainder", snPlot::Linear,
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
	curve_corrfunc1_I = new snCurveReal(SDR.CorrFuncSamples, snCurve::blue);
	curve_corrfunc1_I->setCurveSymbol(snCurve::CIRCLE, snCurve::blue, snCurve::MEDIUM);
	curve_corrfunc2_I = new snCurveReal(3, snCurve::blue);
	curve_corrfunc2_I->setCurveSymbol(snCurve::CIRCLE, snCurve::blue, snCurve::MEDIUM);

	curve_corrfunc1_Q = new snCurveReal(SDR.CorrFuncSamples, snCurve::red);
	curve_corrfunc1_Q->setCurveSymbol(snCurve::CIRCLE, snCurve::red, snCurve::MEDIUM);
	curve_corrfunc2_Q = new snCurveReal(3, snCurve::red);
	curve_corrfunc2_Q->setCurveSymbol(snCurve::CIRCLE, snCurve::red, snCurve::MEDIUM);

	plot_corrfunc1 = trkw2->get_new_plot_real("corr. func. samples", snPlot::Linear, curve_corrfunc1_I);
	plot_corrfunc1->setXAxisLabel("chips");
	plot_corrfunc1->enableGrid();
	plot_corrfunc1->setXAxis(SDR.CorrFuncStart, SDR.CorrFuncStart + SDR.CorrFuncSamples * SDR.CorrFuncDist);
	plot_corrfunc2 = trkw2->get_new_plot_real("E-P-L corr. func. samples", snPlot::Linear, curve_corrfunc2_I);
	plot_corrfunc2->setXAxisLabel("chips");
	plot_corrfunc2->enableGrid();
	plot_corrfunc2->setXAxis(SDR.CorrFuncStart, SDR.CorrFuncStart + SDR.CorrFuncSamples * SDR.CorrFuncDist);

	plot_corrfunc1->addCurve(curve_corrfunc1_Q);
	plot_corrfunc2->addCurve(curve_corrfunc2_Q);
	trkw2->addPlot(plot_corrfunc1);
	trkw2->addPlot(plot_corrfunc2);

	// pseudorange result plot:
	curve_pr = new snCurveReal(expected_corr_intervals, snCurve::blue);
	plot_pr = trkw2->get_new_plot_real("pseudorange result", snPlot::Linear, curve_pr);
	plot_pr->setXAxis(0.0, expected_lenth);
	plot_pr->setXAxisLabel("t [s]");
	plot_pr->setYAxisLabel("d [m]");
	plot_pr->enableGrid();
	trkw2->addPlot(plot_pr);

	// init result curves:
	for (int i = 0; i < expected_corr_intervals; i++) {
		const double time = i * correlation_length;
		curve_pll_raw_I->update_value_X(i, time);
		curve_pll_raw_I->update_value_Y(i, 0.0);
		curve_pll_raw_Q->update_value_X(i, time);
		curve_pll_raw_Q->update_value_Y(i, 0.0);
		curve_pll_filtered->update_value_X(i, time);
		curve_pll_filtered->update_value_Y(i, 0.0);
		curve_dll_raw_I->update_value_X(i, time);
		curve_dll_raw_I->update_value_Y(i, 0.0);
		curve_dll_raw_Q->update_value_X(i, time);
		curve_dll_raw_Q->update_value_Y(i, 0.0);
		curve_dll_raw_IQ->update_value_X(i, time);
		curve_dll_raw_IQ->update_value_Y(i, 0.0);
		curve_dll_filtered->update_value_X(i, time);
		curve_dll_filtered->update_value_Y(i, 0.0);
		curve_remcodephase->update_value_X(i, time);
		curve_remcodephase->update_value_Y(i, 0.0);
		curve_pr->update_value_X(i, time);
		curve_pr->update_value_Y(i, 0.0);
	}
}

double snSDRStandard::correlation(const itpp::vec &a, const itpp::vec &b) {
	/*	if (a.size() != b.size()) {
	 throw snSimulationException(
	 "snSDRGPS::correlation: error: vectors have different lengths.");
	 }
	 */
	//	return itpp::elem_mult_sum(a, b);
	return itpp::dot(a, b);
}

itpp::vec snSDRStandard::build_code(const itpp::vec &indices, snModulation *m) {
	size_t len = indices.size();
	itpp::vec out(len);

	for (size_t i = 0; i < len; i++) {
		out[i] = m->get_value_for_absolute_sample_number(round(indices[i]));
	}
	return out;
}

itpp::vec snSDRStandard::build_shifted_code(double shift, snModulation *m, Samples_Type samples) {
	itpp::vec out(samples);

	const double samples_per_chip = m->get_samples_per_chip();
	const double samples_per_code = m->get_samples_per_code();

	for (size_t i = 0; i < samples; i++) {
		double ind_d = shift * samples_per_chip + static_cast<double>(i);
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
	//	const itpp::vec early = build_shifted_code(-0.01, m, samples);
	//	const itpp::vec late = build_shifted_code(0.01, m, samples);

	const itpp::vec prompt = build_shifted_code(0.0, m, samples);

	// extremes: early is 2* -el_spacing/2.0 away from prompt

	itpp::vec early = build_shifted_code(2.0 * (-el_spacing / 2.0), m, samples);
	itpp::vec late = build_shifted_code(0.0, m, samples);

	const double ec_1 = correlation(early, prompt) / static_cast<double>(samples);
	const double lc_1 = correlation(late, prompt) / static_cast<double>(samples);

	early = build_shifted_code(0.0, m, samples);
	late = build_shifted_code(2.0 * (+el_spacing / 2.0), m, samples);

	const double ec_2 = correlation(early, prompt) / static_cast<double>(samples);
	const double lc_2 = correlation(late, prompt) / static_cast<double>(samples);

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
