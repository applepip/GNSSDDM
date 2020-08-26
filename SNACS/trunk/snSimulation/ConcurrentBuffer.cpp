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

#include "ConcurrentBuffer.h"

const int OUTPUT = 0;

ConcBuf::ConcBuf(Blocks_Type aBuf, Samples_Type sBuf, std::string pbufname) :
	bufname(pbufname) {

	BufSize = sBuf;
	maxBuf = aBuf;
	Buf = new std::complex<double>[maxBuf * BufSize];
	nToBeRead = new Samples_Type[maxBuf]; // number of usable data in block
	if (OUTPUT) {
		std::cout << "\n ConcBuf(" << bufname << ")::ConcBuf: BufSize: " << BufSize << ", maxBuf: " << maxBuf;
		std::cout.flush();
	}

	nBufProducer = 0;
	nBufConsumer = 0;

	WFinished = -1;

	WaitTimeout = 2000;

//	freeBuffers = new QSemaphore(aBuf);
//	usedBuffers = new QSemaphore;

	freeBuffers.release(aBuf);
}

std::complex<double> *ConcBuf::RequestWriting() {
	freeBuffers.acquire();

	if (OUTPUT) {
		std::cout << "\n ConcBuf(" << bufname << ")::RequestWriting, nBufProducer: " << nBufProducer;
		std::cout.flush();
	}

	WFinished = -1;

	// which Buf to write to?
	const Blocks_Type iBuf = nBufProducer % maxBuf;
	return Buf + BufSize * iBuf; // return pointer to which buffer to write to
}

void ConcBuf::ReleaseWriting(Samples_Type aDataWritten, bool WFinishedFlag) {
	if (OUTPUT) {
		std::cout << "\n ConcBuf(" << bufname << ")::ReleaseWriting: aDataWritten: " << aDataWritten;
		std::cout.flush();
	}
	const Blocks_Type iBuf = nBufProducer % maxBuf;
	nToBeRead[iBuf] = aDataWritten;

	nBufProducer++;
	if (WFinishedFlag) {
		WFinished = nBufProducer;
	}
	usedBuffers.release();
}

std::complex<double> *ConcBuf::RequestReading(Samples_Type *nToRead) {
	usedBuffers.acquire();

	// which Buf to read from?
	const Blocks_Type iBuf = nBufConsumer % maxBuf;
	*nToRead = nToBeRead[iBuf];
	if (OUTPUT) {
		std::cout << "\n ConcBuf(" << bufname << ")::RequestReading: nBufConsumer: " << nBufConsumer
				<< ", nToRead: " << *nToRead;
		std::cout.flush();
	}

	if (nBufConsumer > nBufProducer) {
		std::cout << "\n RequestReading(" << bufname
				<< "): nBufConsumer > nBufProducer: This must never happen. exit.";
		std::cout.flush();
		exit(1);
	}

	return Buf + BufSize * iBuf; // return which buffer to read from
}

bool ConcBuf::ReleaseReading() {
	bool ReturnFinished = false;

	if (OUTPUT) {
		std::cout << "\n ConcBuf(" << bufname << ")::ReleaseReading";
		std::cout.flush();
	}

	nBufConsumer++;
	if (WFinished == nBufConsumer) {
		ReturnFinished = true;
	}

	freeBuffers.release();
	return ReturnFinished; // true if there is no more data in buffer;
}

ConcBuf::~ConcBuf() {
	if (OUTPUT) {
		std::cout << "delete buf\n";
		std::cout.flush();
	}
	delete[] Buf;
	if (OUTPUT) {
		std::cout << "delete ntoberead\n";
		std::cout.flush();
	}
	delete[] nToBeRead;
}
