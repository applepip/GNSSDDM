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
 * \file snSDR.h
 *
 * Software Defined Receiver block include file.
 *
 * \author Frank Schubert
 * \date 2008-08-25
 * \version 0.1
 */

#ifndef DEF_snSDR_H_
#define DEF_snSDR_H_ 1

#include "../snSimulation/snSimulation.h"
#include "../snSimulation/snBlockSerial.h"
#include "../snWidget/snWidget.h"

/**
 * \brief Struct with aquisition and tracking parameters for the software defined receiver.
 *
 * \ingroup sinkBlocks
 *
 */
struct SettingsSDR {
	std::string SignalType;
	bool DoGPSAquisition;
	bool DoGPSTracking;
	bool switch_plot_3d;
	std::string DiscrType;
	int Norm;
	double CorrLen;
	// aq settings
	double AqLen; // time duration for aquisition in sec
	double AqFreqBand;
	double AqFreqStep; // search steps
	double AqThreshold;
	int aq_plot_code_phases;
	// trk settings
	double TrkDLLDampRatio; // DLL damping ration
	double TrkDLLNoiseBW; // DLL noise bandwidth
	double TrkEarlyLateSpacing;
	double TrkPLLDampRatio; // PLL damping ration
	double TrkPLLNoiseBW; // PLL noise bandwidth
	int CorrFuncSamples;
	double CorrFuncStart;
	double CorrFuncDist;
	bool WriteCorrFunc;
	//	std::string SREfile;
	double StartTime;
};

/**
 * \brief Software Defined Receiver Base Class
 *
 * Contains the SDR of SNSignal. Aquisition and tracking are performed.
 *
 * \ingroup sinkBlocks
 */
class snSDR: public snBlockSerial {
public:
	snSDR(SNSignal pSNSignal);

private:

protected:

	void calculate_loop_coefficients(double &tau1, double &tau2, double LBW, double zeta, double k);
	unsigned int next_power_of_two(unsigned int val);
	/**
	 * \brief Aquisition() performs a parallel code phase search aquisition.
	 *
	 * Both the incoming signal and the code are Fourier transformed and then multiplied. The result is inverse Fourier transformed and should contain the aquisition peak. The maximum is searched and returned as aquisition frequency and code phase.
	 * @return
	 */
	bool Aquisition(std::vector<double> aq_signal, signed long long samplesPerChip,
			double search_length, double &AqFreq, Samples_Type &AqCodePhase, snWidget *snw,
			snCurve3D *curve_aq);

	double discriminator_dot_product_normalized(double I_E, double Q_E, double I_P, double Q_P,
			double I_L, double Q_L);
	double discriminator_early_minus_late_power_normalized(double I_E, double Q_E, double I_L,
			double Q_L);

	double costas_discriminator_atan(double I_P, double Q_P);
	SettingsSDR SDR; ///< parameter set for SDR parameters
	double intermed_freq;

};

#endif
