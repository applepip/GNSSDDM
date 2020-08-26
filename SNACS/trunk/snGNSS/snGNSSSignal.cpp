/*
 * snGNSSSignal.cpp
 *
 *  Created on: Mar 10, 2010
 *      Author: frank
 */

#include <QString>

#include "snGNSSSignal.h"
#include "../snSimulation/snSimulation.h"
#include "snGNSSData.h"
#include "snGPS.h"
#include "snCodeGalileo.h"
#include "snModulationBPSK.h"
#include "snModulationBOC.h"
#include "snModulationCBOC.h"
#include "snModulationAltBOC.h"

snGNSSSignal::snGNSSSignal(libconfig::Setting &Settings, double _smpl_freq, double _len) :
	smpl_freq(_smpl_freq), len(_len) {

	inphase_enabled_flag = false;
	quadrature_enabled_flag = false;

	// AltBOC is a special case, it inherently has an inphase and a quadrature component:
	if (Settings.exists("modulation")) {
		std::string modulation_type = static_cast<const char *> (Settings["modulation"]);
		if (modulation_type == "AltBOC(n,m)") {
			inphase_enabled_flag = true;
			quadrature_enabled_flag = true;
			inphase_modulation = generate_modulation_from_settings(Settings, INPHASE);
			quadrature_modulation = generate_modulation_from_settings(Settings, QUADRATURE);
		} else {
			throw snSimulationException(
					"Signal generation: Only AltBOC modulation can be defined without inphase/quadrature sections.");
		}
	} else { // all other modulations:
		if (Settings.exists("inphase")) {
			inphase_modulation = generate_modulation_from_settings(Settings["inphase"], INPHASE);
			inphase_enabled_flag = true;
		}

		if (Settings.exists("quadrature")) {
			quadrature_modulation = generate_modulation_from_settings(Settings["quadrature"], QUADRATURE);
			quadrature_enabled_flag = true;
		}
	}

	if ((inphase_enabled_flag == false) && (quadrature_enabled_flag == false)) {
		throw snSimulationException(
				"Signal generation: Neither an inphase nor a quadrature signal has been defined!");
	}

}

snModulation *snGNSSSignal::generate_modulation_from_settings(libconfig::Setting &Settings, phase_type pt) {

	// generate code first:
	QString code_type = static_cast<const char*> (Settings["code"]);
	snGNSSCode *code = 0;
	snGNSSCode *E5aI_code = 0, *E5aQ_code = 0, *E5bI_code = 0, *E5bQ_code = 0;

	if (code_type == "C/A") {
		code = new snGPS(Settings["prn"]);
	} else if (code_type == "E1B") {
		code = new snCodeGalileo(snCodeGalileo::E1B, Settings["prn"]);
	} else if (code_type == "E1C") {
		code = new snCodeGalileo(snCodeGalileo::E1C, Settings["prn"]);
	} else if (code_type == "E5") {
		E5aI_code = new snCodeGalileo(snCodeGalileo::E5aI, Settings["E5a-I_prn"]);
		E5aQ_code = new snCodeGalileo(snCodeGalileo::E5aQ, Settings["E5a-Q_prn"]);
		E5bI_code = new snCodeGalileo(snCodeGalileo::E5bI, Settings["E5b-I_prn"]);
		E5bQ_code = new snCodeGalileo(snCodeGalileo::E5bQ, Settings["E5b-Q_prn"]);
	} else if (code_type == "E5aI") {
		code = new snCodeGalileo(snCodeGalileo::E5aI, Settings["prn"]);
	} else if (code_type == "E5aQ") {
		code = new snCodeGalileo(snCodeGalileo::E5aQ, Settings["prn"]);
	} else if (code_type == "E5bI") {
		code = new snCodeGalileo(snCodeGalileo::E5bI, Settings["prn"]);
	} else if (code_type == "E5bQ") {
		code = new snCodeGalileo(snCodeGalileo::E5bQ, Settings["prn"]);
	} else {
		throw snSimulationException(QString("Unknown code type" + code_type).toStdString());
	}

	// generate data bits:
	QString data_type = static_cast<const char*> (Settings["data_type"]);
	snGNSSData *gnss_data = 0;
	if (data_type == "random") {
		gnss_data = new snGNSSData(snGNSSData::RANDOM, len, Settings["data_bps"]);
	} else if (data_type == "none") {
		// no data
		gnss_data = new snGNSSData(snGNSSData::NONE, len, 1);
	} else {
		throw snSimulationException(QString("Unknown data type " + data_type).toStdString());
	}

	// generate modulation with code:
	QString modulation_type = static_cast<const char*> (Settings["modulation"]);
	snModulation *modulation;

	// setup BPSK modulation
	if (modulation_type == "BPSK(n)") {
		modulation = new snModulationBPSK(code, gnss_data, smpl_freq, Settings["n"]);

		// setup BOC modulation
	} else if (modulation_type == "BOC(n,m)") {
		QString phasing = static_cast<const char*> (Settings["subcarrier_phasing"]);
		snModulationBOC::boc_phasing_type boc_phasing;
		if (phasing == "sin") {
			boc_phasing = snModulationBOC::SIN;
		} else if (phasing == "cos") {
			boc_phasing = snModulationBOC::COS;
		} else {
			throw snSimulationException(QString("Unknown subcarrier phasing: " + phasing).toStdString());
		}
		modulation = new snModulationBOC(code, gnss_data, smpl_freq, Settings["n"], Settings["m"],
				boc_phasing);

		// setup CBOC modulation
	} else if (modulation_type == "CBOC(n1,n2,p)") {
		double pr_n = Settings["p_numerator"];
		double pr_d = Settings["p_denominator"];
		modulation = new snModulationCBOC(code, gnss_data, smpl_freq, Settings["n1"], Settings["n2"], pr_n
				/ pr_d);

		// setup AltBOC modulation
	} else if (modulation_type == "AltBOC(n,m)") {
		snModulationAltBOC::path_type path;
		switch (pt) {
		case INPHASE:
			path = snModulationAltBOC::INPHASE;
			break;
		case QUADRATURE:
			path = snModulationAltBOC::QUADRATURE;
			break;
		default:
			std::cout << "\nAltBOC: wrong path type:" << pt << "\n";
			std::abort();
			break;
		}
		modulation = new snModulationAltBOC(E5aI_code, E5aQ_code, E5bI_code, E5bQ_code, gnss_data, smpl_freq,
				Settings["n"], Settings["m"], path);
	} else {
		throw snSimulationException(QString("Unknown modulation type " + modulation_type).toStdString());
	}

	return modulation;
}

snGNSSSignal::~snGNSSSignal() {
	if (inphase_enabled_flag)
		delete inphase_modulation;
	if (quadrature_enabled_flag)
		delete quadrature_modulation;
	//	delete code;
}
