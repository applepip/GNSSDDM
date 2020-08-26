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

#include "ConcurrentBuffer3.h"
#include <QString>

const bool OUTPUT = false;
const bool OUTPUT2 = false;

ConcBuf3::ConcBuf3(Blocks_Type _nof_blocks_per_concbuf, double _block_length, Freq_Type _smpl_freq,
		double _signal_length, std::string pbufname) :
	nof_blocks_per_concbuf(_nof_blocks_per_concbuf), block_length(_block_length), smpl_freq(_smpl_freq),
			signal_length(_signal_length), bufname(pbufname) {

	// amount of blocks per concbuf:
	if (nof_blocks_per_concbuf < 2) {
		throw snSimulationException("blocks_per_concbuf must be >= 2!");
	}

	nof_blocks_per_concbuf1 = nof_blocks_per_concbuf;
	nof_blocks_per_concbuf2 = nof_blocks_per_concbuf;

	// check sampling frequency
	if (smpl_freq < 0) {
		throw snSimulationException("sampling_frequency must be greater than 0.");
	}

	// check signal length
	if (signal_length < 0) {
		throw snSimulationException("signal_length must be greater than 0.");
	}

	// calculate total amount of samples
	if (signal_length * smpl_freq > ULLONG_MAX) { // Samples_Type is ULLONG
		throw snSimulationException(QString("Variable overflow: Total amount of samples must be smaller than"
				+ QString::number(ULLONG_MAX) + " samples").toStdString());
	}
	total_samples = signal_length * smpl_freq;

	// calculate block size
	if (block_length * smpl_freq > ULLONG_MAX) { // Samples_Type is ULLONG
		throw snSimulationException(QString("Variable overflow: Total amount of samples must be smaller than"
				+ QString::number(ULLONG_MAX) + " samples").toStdString());
	}
	block_size = block_length * smpl_freq;

	// calculate total amount of data blocks:
	if (signal_length * smpl_freq / block_size > LONG_MAX) { // Blocks_Type is LONG
		throw snSimulationException(QString(
				"error: variable overflow: total amount of samples must be smaller than " + QString::number(
						ULONG_MAX)).toStdString());
	} else if (!isInt(signal_length / block_length)) {
		throw snSimulationException("(signal_length / block_length) must be integer.");
	} else {
		total_blocks = total_samples / block_size;
	}

	if (OUTPUT) {
		std::cout << "\nconcbuf3: total blocks: " << total_blocks << "\n";
		std::cout << "\n ConcBuf3(" << bufname << ")::ConcBuf3: smpl_freq: " << smpl_freq
				<< ", signal_length: " << signal_length << ", block_length: " << block_length
				<< ", block_size: " << block_size << ", nof_blocks_per_concbuf: " << nof_blocks_per_concbuf
				<< "\n";
		std::cout.flush();
	}
	// setup the blocks:
	Buf.resize(nof_blocks_per_concbuf);
	for (Blocks_Type i = 0; i < nof_blocks_per_concbuf; i++) {
		Buf.at(i).set_size(block_size);
	}

	nBufProducer = 0;
	nBufConsumer = 0;

	WFinished = -1;

	WaitTimeout = 2000;

	//	freeBuffers = new QSemaphore(aBuf);
	//	usedBuffers = new QSemaphore;

	freeBuffers.release(nof_blocks_per_concbuf);
}

itpp::cvec &ConcBuf3::RequestWriting() {
	freeBuffers.acquire();

	// which Buf to write to?
	const Blocks_Type iBuf = nBufProducer % nof_blocks_per_concbuf1;

	if (OUTPUT) {
		std::cout << "\n ConcBuf3(" << bufname << ")::RequestWriting, nBufProducer: " << nBufProducer
				<< ", iBuf: " << iBuf << "\n";
		std::cout.flush();
	}

	WFinished = -1;

	return Buf.at(iBuf); // return buffer to write to
}

void ConcBuf3::ReleaseWriting(bool WFinishedFlag) {
	const Blocks_Type iBuf = nBufProducer % nof_blocks_per_concbuf1;

	if (OUTPUT) {
		std::cout << "\n ConcBuf3(" << bufname << ")::ReleaseWriting, iBuf: " << iBuf << "\n";
		std::cout.flush();
	}

	// check if buffer still has the same size:

	if (static_cast<Samples_Type> (Buf.at(iBuf).size()) != block_size) {
		std::cerr << "\n ConcBuf3(" << bufname
				<< ")::ReleaseWriting: error: you are not allowed to change the buffer size!\n";
		std::cerr.flush();
		std::exit(1);
	}
	nBufProducer++;
	if (WFinishedFlag) {
		WFinished = nBufProducer;
	}
	usedBuffers.release();
}

itpp::cvec& ConcBuf3::RequestReading() {
	// which Buf to read from?
	const Blocks_Type iBuf = nBufConsumer % nof_blocks_per_concbuf2;

	if (OUTPUT2) {
		std::cout << "ConcBuf3(" << bufname << ")::RequestReading will acquire now, iBuf: " << iBuf << "\n";
		std::cout.flush();
	}
	usedBuffers.acquire();

	if (OUTPUT2) {
		std::cout << "ConcBuf3(" << bufname << ") acquired, iBuf: " << iBuf << "\n";
		std::cout.flush();
	}

	if (OUTPUT) {
		std::cout << "\n ConcBuf3(" << bufname << "): nBufConsumer: " << nBufConsumer << ", iBuf: " << iBuf
				<< "\n";
		std::cout.flush();
	}

	if (nBufConsumer > nBufProducer) {
		std::cout << "\n RequestReading(" << bufname
				<< "): nBufConsumer > nBufProducer: This must never happen. exit.\n";
		std::cout.flush();
		exit(1);
	}
	return Buf.at(iBuf); // return buffer to read from
}

bool ConcBuf3::ReleaseReading() {
	bool ReturnFinished = false;

	if (OUTPUT) {
		std::cout << "\n ConcBuf3(" << bufname << ")::ReleaseReading\n";
		std::cout.flush();
	}

	nBufConsumer++;
	if (WFinished == nBufConsumer) {
		ReturnFinished = true;
	}

	freeBuffers.release();
	return ReturnFinished; // true if there is no more data in buffer;
}

ConcBuf3::~ConcBuf3() {
}
