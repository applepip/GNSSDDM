/*
 * snModulation.cpp
 *
 *  Created on: Mar 10, 2010
 *      Author: frank
 */

#include "snModulation.h"

#include "snGPS.h"
#include "snCodeGalileo.h"

snModulation::snModulation(snGNSSCode *_code, snGNSSData *_data, double _smpl_freq) :
	code(_code), data(_data), smpl_freq(_smpl_freq) {

}

snModulation::~snModulation() {
	// TODO Auto-generated destructor stub
}
