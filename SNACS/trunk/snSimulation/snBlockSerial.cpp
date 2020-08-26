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

#include "snBlockSerial.h"

snBlockSerial::snBlockSerial(SNSignal pSNSignal) :
	snBlock(pSNSignal) {
}

void snBlockSerial::InitSerialBuffer(Samples_Type size, ConcBuf *pConcBuf) {
	SerBufSize = size;
	serBuf = new std::vector<std::complex<double> >(SerBufSize);
	ConcBuf1 = pConcBuf;

	serbuf_last_sample = 0; // count for serial buffer
	serbuf_actual_sample = 0; // acutal read position for serial buffer
	absoluteSample = 0;
	stop_when_empty = false;

	serbuf_initialized = true;
}

std::vector<std::complex<double> > snBlockSerial::get_next_samples(Samples_Type n) {
	if (!serbuf_initialized) {
		std::cout << "\n serial buffer not initialized.";
		std::exit(1);
	}

	if (!isActive()) {
		std::cout << "\n attempt to read after data stream ended: exit.";
		std::exit(1);
	}

	std::vector<std::complex<double> > out(n);

	for (Samples_Type i = 0; i < n; i++) {

		// do we need to read in a new concbuf?:
		if (serbuf_actual_sample == serbuf_last_sample) {
			if (serbuf_last_sample + Sig.BufSize > SerBufSize) {
				serbuf_last_sample = 0;
				serbuf_actual_sample = 0;
			}
			Samples_Type nToRead = 0;
			std::complex<double> *Buf = ConcBuf1->RequestReading(&nToRead);

			if (nToRead == 0) {
				// no more data came in
				ConcBuf1->ReleaseReading();
				requestStop();
				// return the data until now:
				out.resize(i);
				return out;
			}

			for (Samples_Type i = 0; i < nToRead; i++) {
				serBuf->at(serbuf_last_sample++) = Buf[i];
			}

			if (ConcBuf1->ReleaseReading()) // TODO
				stop_when_empty = true;

			increaseBlks();
			increaseOutputBlks();
		}

		absoluteSample++;

		if (stop_when_empty) {
			if (serbuf_actual_sample + 1 == serbuf_last_sample) {
				requestStop();
			}
		}

		//	return serBuf->at(serbuf_actual_sample++);
		out.at(i) = serBuf->at(serbuf_actual_sample++);
	}
	return out;
}

void snBlockSerial::set_next_sample_index(Samples_Type index) {
	if (index > serbuf_last_sample) {
		std::cout << "\n attempt to access non-existent serial buffer element.";
		std::exit(1);
	}
	serbuf_actual_sample = index;
	absoluteSample = index;
}
