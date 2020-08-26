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
 * \file snProcessorChannel.h
 *
 * \author Frank Schubert
 */

#ifndef DEF_SNPROCCHANNEL_H_
#define DEF_SNPROCCHANNEL_H_ 1

#include "../snWidget/snWidget.h"
#include "../snModules/snCreatingModule.h"
#include "../../../../../cdx_format/CDX_library_cpp/WriteDiscreteDelayCDXFile.h"
#include "../../../../../cdx_format/CDX_library_cpp/ReadContinuousDelayCDXFile.h"

/**
 * \brief snProcessorChannel implements convolution of a base band signal through a linear channel through FIR filtering.
 *
 * \ingroup procBlocks
 *
 */
class snProcessorChannel: public snCreatingModule {

public:
	enum interpolation_types {
		SINC, RRC
	};

	snProcessorChannel(libconfig::Setting &Settings, SNSignal pSNSignal, ProcessingParameters &ProcSig,
			 widget_factory_base *widget_factory, ConcBuf3* _circbuf_in);
	~snProcessorChannel();

	void run();

private:
	void find_max_delay(ReadContinuousDelayCDXFile *file, const size_t &nof_cirs, double &MaxDelay, size_t &MaxEchoes);
	void find_min_delay(ReadContinuousDelayCDXFile *file, const size_t &nof_cirs, double &MinDelay, size_t &MaxEchoes);

	double sinc(double x) {
		if (x == 0) {
			return 1;
		} else {
			double f = M_PI * x;
			return sin(f) / f;
		}
	}

	double T(double n, double x);
	itpp::vec chebyshev_t(const size_t M, const double at);
	double wc(const size_t M, const double at, const double Ts, const double t);
	double chebyshev_t_shifted(const size_t M, const double at, const double Ts, const double t_0,
			const double t);

	bool write_cdx_file;
	WriteDiscreteDelayCDXFile *cdx_out;

	interpolation_types interpolation_type;
	double rrc_rolloff_factor;

	ReadContinuousDelayCDXFile *file;

	// Dolph window parameters
	double stop_band_attenuation_dB; ///< stop_band_attenuation in dB
	double r; ///< ripple ratio
	size_t M, N;

	// interpolator:
	double tau_off;
	Blocks_Type nof_cirs;
	double cir_rate; // CIR rate

	double interpolation_bandwidth;
	double MaxDelay, MinDelay;

	int nof_fir_coeffs;

	// for plotting:
	snWidget *cir_snw, *fir_snw, *out_snw;
	snCurveComplex *fir_curve;
	snCurveComplex *cir_curve;
	snCurveComplex *out_curve;

	ConcBuf3 *circbuf_in;
};

#endif
