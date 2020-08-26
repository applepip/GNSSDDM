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
 * \file snGPS.cpp
 *
 * \author Frank Schubert
 */

#include <iostream>
#include <cmath> // floor()
#include <cstdlib> // exit()
#include "snGPS.h"

snGPS::snGPS(int _prn) :
	snGNSSCode(_prn) {

	code = generate_ca_code(prn);
	f_cps = 1.023e6;
}

snGPS::~snGPS() {
}

std::vector<double> snGPS::generate_ca_code(int code_prn) {
	std::vector<double> ca(1023);
	// This function is based on the CA-Code generation function of
	// K. Borre et. al: "A Software-Defined GPS and Galileo Receiver"

	int g2s[1024] = {
			//shifts for GPS satellites
			0, 5, 6, 7, 8, 17, 18, 139, 140, 141, 251, 252, 254, 255, 256, 257, 258, 469, 470, 471, 472, 473,
			474, 509, 512, 513, 514, 515, 516, 859, 860, 861, 862,
			// EGNOS and WAAS satellites:
			145, 175, 52, 21, 237, 235, 886, 657, 634, 762, 355, 1012, 176, 603, 130, 359, 595, 68, 386 };

	int g2shift;
	int reg[10];

	int g1[1023];
	int g2[1023];
	int g22[1023];
	int saveBit = 0;

	for (int i = 0; i < 1023; i++) {
		g1[i] = 0;
		g2[i] = 0;
		g22[i] = 0;
	}

	g2shift = g2s[code_prn];

	//%--- Load shift register ---
	for (int i = 0; i < 10; i++)
		reg[i] = -1;

	// %--- Generate all G1 signal chips based on the G1 feedback polynomial -----
	for (int i = 0; i < 1023; i++) {
		g1[i] = reg[9];
		//    printf("g1[%i] is %i. ", i, g1[i]);
		saveBit = reg[2] * reg[9];
		for (int k = 9; k >= 1; k--) {
			reg[k] = reg[k - 1];
		}
		reg[0] = saveBit;

	}

	// %--- Generate G2 code -----------------------------------------------------

	// %--- Initialize g2 output to speed up the function ---

	// %--- Load shift register ---
	for (int i = 0; i < 10; i++)
		reg[i] = -1; // -1*ones(1, 10);

	// %--- Generate all G2 signal chips based on the G2 feedback polynomial -----
	for (int i = 0; i < 1023; i++) {
		g2[i] = reg[9];
		saveBit = reg[1] * reg[2] * reg[5] * reg[7] * reg[8] * reg[9];
		for (int k = 9; k >= 1; k--) {
			reg[k] = reg[k - 1];
		}
		reg[0] = saveBit;
	}

	//%--- Shift G2 code --------------------------------------------------------
	int o = 0;
	for (int i = 1023 - g2shift + 1 - 1; i <= 1023 - 1; i++) {
		g22[o] = g2[i];
		o++;
	}
	for (int i = 0; i <= 1023 - g2shift - 1; i++) {
		g22[o] = g2[i];
		o++;
	}

	// %--- Form single sample C/A code by multiplying G1 and G2 -----------------
	for (int i = 0; i < 1023; i++) {
		ca.at(i) = -(g1[i] * g22[i]);
	}
	return ca;
}
