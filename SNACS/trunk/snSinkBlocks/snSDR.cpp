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
 * \addtogroup sinkBlocks
 *
 * Software Defined Receiver base class.
 *
 * \author Frank Schubert
 * \date 2008-08-25
 * \version 0.1
 */

#include "snSDR.h"

#include <fftw3.h>
#include <algorithm> // for max_element
/**
 * \brief Software Defined Receiver base class constructor
 */
snSDR::snSDR(SNSignal pSNSignal) :
	snBlockSerial(pSNSignal) {
}

void snSDR::calculate_loop_coefficients(double &tau1, double &tau2, double LBW, double zeta,
		double k) {
	// solve for natural frequency
	double Wn = LBW * 8.0 * zeta / (4.0 * pow(zeta, 2) + 1.0);

	// solve for tau1 and tau2
	tau1 = k / (Wn * Wn);
	tau2 = 2.0 * zeta / Wn;
}

unsigned int snSDR::next_power_of_two(unsigned int val) {
	unsigned int i;
	for (i = 1; i < (1 << 30); i <<= 1) {
		if (val <= i) {
			return i;
		}
	}
	return i;
}

bool snSDR::Aquisition(std::vector<double> aq_signal, signed long long samplesPerChip,
		double search_length, double &AqFreq, Samples_Type &AqCodePhase, snWidget *snw,
		snCurve3D *curve_aq) {

	std::complex<double> *SigInTD1; ///< array for Fourier transform of signal input , time domain
	std::complex<double> *SigInFD1; ///< array for  Fourier transform of signal input, frequency domain
	std::complex<double> *SigInTD2; ///< array for Fourier transform of signal input , time domain
	std::complex<double> *SigInFD2; ///< array for  Fourier transform of signal input, frequency domain
	fftw_plan FFTSigIn1; ///< plan for calling FFTW
	fftw_plan FFTSigIn2; ///< plan for calling FFTW
	std::complex<double> *CAblkTD; ///< array for Fourier transform of code signal , time domain
	std::complex<double> *CAblkFD; ///< array for Fourier transform of code signal, frequency domain
	fftw_plan FFTCode; ///< plan for calling FFTW
	std::complex<double> *AqResFD1; ///< array for Fourier transform of aq result, frequency domain
	std::complex<double> *AqResTD1; ///< array for Fourier transform of aq result, time domain

	std::complex<double> *AqResFD2; ///< array for Fourier transform of aq result, frequency domain
	std::complex<double> *AqResTD2; ///< array for Fourier transform of aq result, time domain
	fftw_plan FFTAqRes1; ///< plan for calling FFTW
	fftw_plan FFTAqRes2; ///< plan for calling FFTW

	Samples_Type samplesPerCode = aq_signal.size();
	Samples_Type samples_per_search_length = search_length * Sig.SmplFreq;

	// FFT for input signal:
	SigInTD1 = new std::complex<double>[samplesPerCode];
	SigInFD1 = new std::complex<double>[samplesPerCode];
	FFTSigIn1 = fftw_plan_dft_1d(samplesPerCode, reinterpret_cast<fftw_complex*> (SigInTD1),
			reinterpret_cast<fftw_complex*> (SigInFD1), FFTW_FORWARD, FFTW_ESTIMATE);
	// FFT for input signal:
	SigInTD2 = new std::complex<double>[samplesPerCode];
	SigInFD2 = new std::complex<double>[samplesPerCode];
	FFTSigIn2 = fftw_plan_dft_1d(samplesPerCode, reinterpret_cast<fftw_complex*> (SigInTD2),
			reinterpret_cast<fftw_complex*> (SigInFD2), FFTW_FORWARD, FFTW_ESTIMATE);

	// FFT for code block:
	CAblkTD = new std::complex<double>[samplesPerCode];
	CAblkFD = new std::complex<double>[samplesPerCode];
	FFTCode = fftw_plan_dft_1d(samplesPerCode, reinterpret_cast<fftw_complex*> (CAblkTD),
			reinterpret_cast<fftw_complex*> (CAblkFD), FFTW_FORWARD, FFTW_ESTIMATE);
	// FFT for aqusition result
	AqResFD1 = new std::complex<double>[samplesPerCode];
	AqResTD1 = new std::complex<double>[samplesPerCode];
	FFTAqRes1 = fftw_plan_dft_1d(samplesPerCode, reinterpret_cast<fftw_complex*> (AqResFD1),
			reinterpret_cast<fftw_complex*> (AqResTD1), FFTW_BACKWARD, FFTW_ESTIMATE);
	// FFT for aqusition result
	AqResFD2 = new std::complex<double>[samplesPerCode];
	AqResTD2 = new std::complex<double>[samplesPerCode];
	FFTAqRes2 = fftw_plan_dft_1d(samplesPerCode, reinterpret_cast<fftw_complex*> (AqResFD2),
			reinterpret_cast<fftw_complex*> (AqResTD2), FFTW_BACKWARD, FFTW_ESTIMATE);

	std::vector<std::complex<double> > longSignal_C(samples_per_search_length); // buffer for 11ms of signal
	std::vector<double> longSignal(samples_per_search_length); // buffer for 11ms of signal
	std::vector<double> Sig0DC(samples_per_search_length); // holds zero DC signal

	std::vector<double> signal1(samplesPerCode); // buffer1 for 1ms of signal
	std::vector<double> signal2(samplesPerCode); // buffer2 for 1ms of signal

	// Aquisition //
	snLogDeb("snSDRGPS: Starting aquisition.", 1);
	//for (Samples_Type i = 0; i < samples_per_search_length; i++)
	longSignal_C = get_next_samples(samples_per_search_length);
	for (Samples_Type i = 0; i < longSignal_C.size(); i++)
		longSignal[i] = longSignal_C[i].real();

	for (Samples_Type i = 0; i < samplesPerCode; i++) {
		signal1.at(i) = longSignal.at(i);
	}

	for (Samples_Type i = samplesPerCode; i < 2 * samplesPerCode; i++) {
		signal2.at(i - samplesPerCode) = longSignal.at(i);
	}

	double Sig11msMean = 0.0;
	for (Samples_Type i = 0; i < samples_per_search_length; i++)
		Sig11msMean += longSignal.at(i);

	Sig11msMean /= samples_per_search_length;
	for (Samples_Type i = 0; i < samples_per_search_length; i++)
		Sig0DC.at(i) = longSignal.at(i) - Sig11msMean;

	// create phase points with input signal:
	std::vector<double> PhasePoints(samplesPerCode); ///< phase points that go into sin() and cos() are pre-calculated

	for (Samples_Type i = 0; i < samplesPerCode; i++)
		PhasePoints.at(i) = static_cast<double> (i) * 2.0 * M_PI * Sig.Ts;

	// FFT code block
	for (Samples_Type i = 0; i < samplesPerCode; i++) {
		CAblkTD[i] = aq_signal.at(i);
	}

	fftw_execute(FFTCode); // F{CAblkTD} = CAblkFD
	// conjugate result
	for (Samples_Type i = 0; i < samplesPerCode; i++) {
		CAblkFD[i] = conj(CAblkFD[i]);
	}

	std::vector<double> AqFreqs(static_cast<int> (floor(SDR.AqFreqBand / SDR.AqFreqStep)));

	for (size_t i = 0; i < AqFreqs.size(); i++) {
		AqFreqs.at(i) = intermed_freq - SDR.AqFreqBand / 2.0 + i * SDR.AqFreqStep;
	}

	std::vector<std::vector<double> > AqResults(AqFreqs.size()); ///< memory for aq results, vector will have size: acFreqSteps x acSmpls
	for (size_t i = 0; i < AqFreqs.size(); i++)
		AqResults.at(i) = std::vector<double>(samplesPerCode);

	std::vector<double> AqRes1(samplesPerCode);
	std::vector<double> AqRes2(samplesPerCode);

	// put input data into input signal to FFT
	for (size_t o = 0; o < AqFreqs.size(); o++) {

		snLogDeb("snSDRGPS: Processing frequency: " + QString::number(AqFreqs.at(o)), 4);
		// mix down signal
		for (Samples_Type i = 0; i < samplesPerCode; i++) {
			SigInTD1[i].real() = sin(PhasePoints.at(i) * AqFreqs.at(o)) * signal1.at(i);
			SigInTD1[i].imag() = cos(PhasePoints.at(i) * AqFreqs.at(o)) * signal1.at(i);
			SigInTD2[i].real() = sin(PhasePoints.at(i) * AqFreqs.at(o)) * signal2.at(i);
			SigInTD2[i].imag() = cos(PhasePoints.at(i) * AqFreqs.at(o)) * signal2.at(i);
		}

		fftw_execute(FFTSigIn1); // F{SigInTD1} = SigInFD1
		fftw_execute(FFTSigIn2); // F{SigInTD2} = SigInFD2

		// multiply FFT'ed SigIn and FFT'ed conj of code:
		for (Samples_Type i = 0; i < samplesPerCode; i++) {
			AqResFD1[i] = SigInFD1[i] * CAblkFD[i];
			AqResFD2[i] = SigInFD2[i] * CAblkFD[i];
		}

		// inverse fft to get aq result
		fftw_execute(FFTAqRes1); // F^(-1){AqResFD1} = AqResTD1
		fftw_execute(FFTAqRes2); // F^(-1){AqResFD2} = AqResTD2

		//		be carefull: ifft in Matlab differs from FFTW ifft!:
		//		"The algorithm for ifft(X) is the same as the algorithm for fft(X), except for a sign change and a scale factor of n = length(X)."

		// store result in AqRes vector
		for (Samples_Type i = 0; i < samplesPerCode; i++) {
			AqResTD1[i] /= samplesPerCode;
			AqResTD2[i] /= samplesPerCode;
			AqRes1.at(i) = std::norm(AqResTD1[i]);
			AqRes2.at(i) = std::norm(AqResTD2[i]);
		}

		double maxAQRes1 = *std::max_element(AqRes1.begin(), AqRes1.end());
		double maxAQRes2 = *std::max_element(AqRes2.begin(), AqRes2.end());

		if (maxAQRes1 > maxAQRes2) {
			for (Samples_Type i = 0; i < samplesPerCode; i++)
				AqResults.at(o).at(i) = AqRes1.at(i);
		} else {
			for (Samples_Type i = 0; i < samplesPerCode; i++)
				AqResults.at(o).at(i) = AqRes2.at(i);
		}

		if (SDR.switch_plot_3d) {
			snw->pushData();
			int fac = static_cast<int> (floor(samplesPerCode / SDR.aq_plot_code_phases));
			for (int i = 0; i < SDR.aq_plot_code_phases; i++) {
				double vMax = 0.0;
				// show maximal value in aq_plot_code_phases interval
				for (int l = i * fac; l < (i + 1) * fac; l++) {
					double tmp = AqResults.at(o).at(l);
					if (tmp > vMax)
						vMax = tmp;
				}
				curve_aq->SXY[i][o] = sqrt(vMax) / samplesPerCode;
			}
			snw->releasePlotFlag();
		}

	} // for (int o = 0; o < acFreqSteps; o++ )

	// find maximum:
	std::vector<double> vMaximums(AqResults.size());
	for (size_t o = 0; o < AqResults.size(); o++) { // for all frequencies
		vMaximums.at(o) = *std::max_element(AqResults.at(o).begin(), AqResults.at(o).end());
	}

	double PeakSize = *std::max_element(vMaximums.begin(), vMaximums.end());

	int iMaxFreq = std::max_element(vMaximums.begin(), vMaximums.end()) - vMaximums.begin();

	for (size_t s = 0; s < AqResults.size(); s++) { // for all frequencies
		Samples_Type CodePhase2 = std::max_element(AqResults.at(s).begin(), AqResults.at(s).end())
				- AqResults.at(s).begin();
		snLogDeb("CodePhase for freq " + QString::number(AqFreqs.at(s)) + ": " + QString::number(
				CodePhase2) + ", value: " + QString::number(*std::max_element(
				AqResults.at(s).begin(), AqResults.at(s).end())), 6);
	}

	Samples_Type CodePhase = std::max_element(AqResults.at(iMaxFreq).begin(),
			AqResults.at(iMaxFreq).end()) - AqResults.at(iMaxFreq).begin();

	// Find one chip wide C/A code phase exclude range around the peak

	std::vector<double> aq_fbin = AqResults[iMaxFreq];
	//	set 2 chip wide bin range around the found max peak in aq_fbin to zero:
	for (signed long long i = -samplesPerChip; i < samplesPerChip; i++) {
		signed long long idx = CodePhase + i;

		if (idx < 0)
			idx += samplesPerCode;

		if (idx >= static_cast<signed long long> (samplesPerCode))
			idx -= samplesPerCode;
		aq_fbin.at(idx) = 0;
	}

	double secPeakSize = *std::max_element(aq_fbin.begin(), aq_fbin.end());

	bool AqSuccess;

	double aq_metric = PeakSize / secPeakSize;

	snLogDeb("snSDRGPS: Aquisition metric: " + QString::number(aq_metric), 1);

	if (aq_metric > SDR.AqThreshold) {
		if (0) {
			// freq refinement
			//			snGPS1.OneCodeBlkSampled(CAblkTD, Sig.SmplFreq, SDR.AqLen, 10);
			double longCA[10 * samplesPerCode]; // 10ms of CA code
			for (int o = 0; o < 10; o++) {
				for (Samples_Type i = 0; i < samplesPerCode; i++)
					longCA[o * samplesPerCode + i] = CAblkTD[i].real();
			}

			// remove code from Sig0DC

			//long CarrN = 8*pow(2, nextpow2 ( 10*samplesPerCode ));
			long CarrN = static_cast<int> (8 * pow(2, 20));

			std::complex<double> *CarrTD;
			std::complex<double> *CarrFD;
			fftw_plan FFTCarr;
			CarrTD = new std::complex<double>[CarrN];
			CarrFD = new std::complex<double>[CarrN];
			FFTCarr = fftw_plan_dft_1d(samplesPerCode, reinterpret_cast<fftw_complex*> (CarrTD),
					reinterpret_cast<fftw_complex*> (CarrFD), FFTW_FORWARD, FFTW_ESTIMATE);

			for (int i = 0; i < CarrN; i++) {
				CarrTD[i].real() = 0;
				CarrFD[i].real() = 0;
				CarrTD[i].imag() = 0;
				CarrFD[i].imag() = 0;
			}

			for (Samples_Type i = 0; i < 10 * samplesPerCode; i++) {
				CarrTD[i].real() = Sig0DC[i + CodePhase] * longCA[i];
			}
			fftw_execute(FFTCarr);

			std::vector<double> FineFreq(CarrN);
			for (int i = 0; i < CarrN; i++) {
				FineFreq[i] = abs(CarrFD[i]);
			}

			//double iRefinedFreq = std::max_element ( FineFreq.begin() + 5, FineFreq.end() - ( CarrN/2-5 ) ) - FineFreq.begin();
			double iRefinedFreq = std::max_element(FineFreq.begin() + FineFreq.size() / 2,
					FineFreq.end()) - FineFreq.begin();
			double vRefinedFreq = iRefinedFreq * (Sig.SmplFreq / (double) CarrN);
			snLogDeb("snSDR: iRefinedFreq: " + QString::number(iRefinedFreq) + ", vRefinedFreq: "
					+ QString::number(vRefinedFreq), 4);

			fftw_destroy_plan(FFTCarr);
			delete[] CarrTD;
			delete[] CarrFD;

		} //refinement

		AqFreq = AqFreqs.at(iMaxFreq);
		AqCodePhase = CodePhase;

		snLogDeb("snSDRGPS: Aquisition successfull. Maximum at freq: " + QString::number(AqFreq)
				+ ", code = " + QString::number(AqCodePhase), 4);

		AqSuccess = true;
	} else {
		snLog("snSDRGPS: Aquisition unsuccessfull.");
		AqSuccess = false;
	}

	fftw_destroy_plan(FFTSigIn1);
	delete[] SigInTD1;
	delete[] SigInFD1;

	// FFT for input signal:
	fftw_destroy_plan(FFTSigIn2);
	delete[] SigInTD2;
	delete[] SigInFD2;

	// FFT for code block:
	fftw_destroy_plan(FFTCode);
	delete[] CAblkTD;
	delete[] CAblkFD;

	// FFT for aqusition result
	fftw_destroy_plan(FFTAqRes1);
	delete[] AqResFD1;
	delete[] AqResTD1;

	// FFT for aqusition result
	fftw_destroy_plan(FFTAqRes2);
	delete[] AqResFD2;
	delete[] AqResTD2;

	return AqSuccess;
}

double snSDR::costas_discriminator_atan(double I_P, double Q_P) {
	return atan(Q_P / I_P) / (2.0 * M_PI);
}

double snSDR::discriminator_dot_product_normalized(double I_E, double Q_E, double I_P, double Q_P,
		double I_L, double Q_L) {
	double code_error = (I_P * (I_E - I_L) + Q_P * (Q_E - Q_L)) / (I_P * I_P + Q_P * Q_P);
	return code_error;
}

double snSDR::discriminator_early_minus_late_power_normalized(double I_E, double Q_E, double I_L,
		double Q_L) {
	const double sqrtIEQE = sqrt(I_E * I_E + Q_E * Q_E);
	const double sqrtILQL = sqrt(I_L * I_L + Q_L * Q_L);
	//const double sqrtIPQP = sqrt(I_P * I_P + Q_P * Q_P);

	double code_error = (sqrtIEQE - sqrtILQL) / (sqrtIEQE + sqrtILQL);
	return code_error;
}

