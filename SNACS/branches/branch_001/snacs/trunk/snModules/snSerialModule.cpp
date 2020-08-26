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
 * \file snBlockSerial.cpp
 *
 * \author Frank Schubert
 */

#include "snSerialModule.h"

snSerialModule::snSerialModule(SNSignal pSNSignal, ConcBuf3* _circbuf) :
	snConsumingModule(pSNSignal, _circbuf) {

	serbuf_initialized = false;
}

void snSerialModule::InitSerialBuffer(Samples_Type size) {
	SerBufSize = size;
	serBuf.set_size(SerBufSize);

	serbuf_last_sample = 0; // count for serial buffer
	serbuf_actual_sample = 0; // acutal read position for serial buffer
	absoluteSample = 0;
	stop_when_empty = false;

	serbuf_initialized = true;

	akt_blocks = 0;
}

itpp::cvec snSerialModule::get_next_samples(Samples_Type n) {
	if (!serbuf_initialized) {
		std::cout << "\n serial buffer not initialized.\n";
		std::exit(1);
	} /*else {
		std::cout << "\n serial buffer is initialized.\n";
	}*/

	if (!isActive()) {
		std::cout << "\nsnSerialModule: attempt to read after data stream ended: exit.";
		std::exit(1);
	}

	itpp::cvec out(n);

	for (Samples_Type i = 0; i < n; i++) {

		// do we need to read in a new concbuf?:
		if (serbuf_actual_sample == serbuf_last_sample) {
			if (serbuf_last_sample + block_size > SerBufSize) {
				serbuf_last_sample = 0;
				serbuf_actual_sample = 0;
			}
			itpp::cvec& Buf = circbuf->RequestReading();
			/*
			 if (nToRead == 0) {
			 // no more data came in
			 circbuf->ReleaseReading();
			 requestStop();
			 // return the data until now:
			 out.set_size(i,);
			 return out;
			 }
			 */
			for (int i = 0; i < Buf.size(); i++) {
				serBuf[serbuf_last_sample++] = Buf[i];
			}

			if (circbuf->ReleaseReading()) // TODO
				stop_when_empty = true;
			akt_blocks++;
		}

		if (stop_when_empty) {
			if (serbuf_actual_sample + 1 == serbuf_last_sample) {
				requestStop();
				out.set_size(i);
				break;
			}
		}

		//	return serBuf->at(serbuf_actual_sample++);
		out[i] = serBuf[serbuf_actual_sample++];
		absoluteSample++;
	}
	return out;
}

void snSerialModule::set_next_sample_index(Samples_Type index) {
	if (index > serbuf_last_sample) {
		std::cout << "\n attempt to access non-existent serial buffer element.";
		std::exit(1);
	}
	serbuf_actual_sample = index;
	absoluteSample = index;
}
