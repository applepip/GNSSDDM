/*
 * snGNSSSignal.h
 *
 *  Created on: Mar 10, 2010
 *      Author: frank
 */

#ifndef SNGNSSSIGNAL_H_
#define SNGNSSSIGNAL_H_

#include <cstdlib>
#include <iostream>

#include "snModulation.h"

class snGNSSSignal {
public:
	snGNSSSignal(libconfig::Setting &Settings, double _smpl_freq, double _len);
	virtual ~snGNSSSignal();

	bool inphase_enabled() {
		return inphase_enabled_flag;
	}

	bool quadrature_enabled() {
		return quadrature_enabled_flag;
	}

	snModulation* get_inphase_modulation() {
		if (!inphase_enabled())
			throw snSimulationException(
					"snGNSSSignal: inphase signal not enabled but request for inphase modulation received.");

		return inphase_modulation;
	}

	snModulation* get_quadrature_modulation() {
		if (!quadrature_enabled())
			throw snSimulationException(
					"snGNSSSignal: quadrature signal not enabled but request for quadrature modulation received.");

		return quadrature_modulation;
	}

	double get_smpl_freq() {
		return smpl_freq;
	}

	double get_smpl_interval() {
		return smpl_interval;
	}

private:
	enum phase_type {
		INPHASE, QUADRATURE
	};

	double smpl_freq, smpl_interval, len;
	bool inphase_enabled_flag, quadrature_enabled_flag;

	snModulation *inphase_modulation, *quadrature_modulation;

	snModulation *generate_modulation_from_settings(libconfig::Setting &Settings, phase_type pt);

};

#endif /* SNGNSSSIGNAL_H_ */
