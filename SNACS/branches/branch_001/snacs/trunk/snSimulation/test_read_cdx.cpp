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

// compile with
// g++ -o test_read_cdx test_read_cdx.cpp snReadCIRFile.cpp snSimulation.cpp -lhdf5 -lhdf5_cpp


/*!
 * \file	test_read_cdx.cpp
 * \brief
 *
 * \author	Frank Schubert
 */

#include "snReadCIRFile.h"

int main() {
	ReadContinuousDelayCDXFile cdx_file(
			"/home/frank/workspace/snacs/cdx_format/Channel-Data-Examples/DLRLMS_urban_continuous-delay.h5");

	std::cout << "file has " << cdx_file.get_nof_cirs() << " CIRs\n";
	std::cout << "cir_rate: " << cdx_file.get_cir_rate() << "\n";

	const int cir_num = 1501;

	cir_struct cir = cdx_file.get_cir(0, cir_num);

	std::cout << "cir #" << cir_num << " has " << cir.components.size() << " components:\n";
	for (size_t k = 0; k < cir.components.size(); k++) {
		std::cout << "component " << k << "\t delay: " << cir.components.at(k).delay << "\t weight: "
				<< cir.components.at(k).weight << "\n";
	}
	std::cout << "ref delay: " << cir.ref_delay << "\n";

	return 0;
}
