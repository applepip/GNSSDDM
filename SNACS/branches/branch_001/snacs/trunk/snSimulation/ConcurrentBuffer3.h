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
 * \file ConcurrentBuffer3.h
 *
 * \author Frank Schubert
 */

#ifndef DEF_ConcBuf3
#define DEF_ConcBuf3

#include <cstdlib>
#include <iostream>
#include <complex>

#include <QSemaphore>
#include <QReadWriteLock>
//#include <QReadLocker>

#include "snGlobal.h"

/**
 * \brief ConcBuf3 is a class that implements a circular buffer for concurrent access from two distinct threads.
 *
 * The constructor sets up a ConcBuf3 consisting of an amount of arrays, called concbuf blocks. The concbuffer is
 * shared between two snBlocks. One snBlock starts to write to the concbuffer and fills up the concbuf blocks one after another.
 * If all concbuf blocks are full, the process has to wait. The successive snBlock in the simulation chain starts to read from the
 * concbuffer until there are no new concbuf blocks left.
 *
 * The writing process requests its need to write to a concbuffer with the function RequestWriting. This function returns
 * a pointer to the first element of the respective concbuf block. After filling this concbuf block, the writing process
 * terminates the writing with ReleaseWriting.
 *
 * The reading process requests its wish for reading from the concbuffer by calling RequestReading. It gets a pointer to the
 * concbuf block and and amount of elements to read. It releases the read lock with ReleaseReading.
 *
 */
class ConcBuf3 {

public:
	/**
	 * \brief Concurrent buffer (ConcBuf3) constructor
	 *
	 * \param <long aBuf> amount of concbuf blocks (arrays) per concbuf
	 * \param <Samples_Type sBuf> length of one concbuf block
	 * \param <std::string pbufname> name of the concbuffer
	 */
	ConcBuf3(Blocks_Type _nof_blocks_per_concbuf, double _block_length,
			Freq_Type _smpl_freq, double _signal_length, std::string pbufname);
	~ConcBuf3();

	/**
	 * \brief snBlocks call RequestWriting to block the next concbuf block for write access
	 * \return pointer to the first element of a concbuf block which is being locked for writing.
	 */
	itpp::cvec &RequestWriting();

	/**
	 * \brief releases write lock to a concbuf block
	 * \param <long aDataWritten> amount of data written
	 * \param <long WFinished> 0 for continued operation. 1 for indication that this
	 * concbuf block was the last one to write to.
	 */
	void ReleaseWriting(bool WFinished);

	/**
	 * \brief request read lock to next concbuf block
	 * \param <Samples_Type *nToRead> this variable will contain the amount of elements
	 * that can be read from the concbuf block
	 * \return pointer to the first element of the concbuf block
	 */
	itpp::cvec &RequestReading();

	/**
	 * \brief releases read lock
	 *
	 */
	bool ReleaseReading();

	Freq_Type get_smpl_freq() {
		QReadLocker _locker(&m_smpl_freq);
		return smpl_freq;
	}

	double get_signal_length() {
		QReadLocker _locker(&m_signal_length);
		return signal_length;
	}

	double get_block_length() {
		QReadLocker _locker(&m_block_length);
		return block_length;
	}

	Samples_Type get_block_size() {
		QReadLocker _locker(&m_block_size);
		return block_size;
	}

	Blocks_Type get_total_blocks() {
		QReadLocker _locker(&m_total_blocks);
		return total_blocks;
	}

	Blocks_Type get_total_samples() {
		QReadLocker _locker(&m_total_samples);
		return total_samples;
	}

	Blocks_Type get_nof_blocks_per_buffer() {
		QReadLocker _locker(&m_nof_blocks_per_concbuf);
		return nof_blocks_per_concbuf;
	}

	Blocks_Type get_current_consumer_block() {
		QReadLocker _locker(&m_nBufConsumer);
		return nBufConsumer;
	}

	Blocks_Type get_current_producer_block() {
		QReadLocker _locker(&m_nBufProducer);
		return nBufProducer;
	}

private:
	Blocks_Type nof_blocks_per_concbuf, nof_blocks_per_concbuf1,
			nof_blocks_per_concbuf2;
	QReadWriteLock m_nof_blocks_per_concbuf;

	Samples_Type block_size;
	QReadWriteLock m_block_size;

	double block_length;
	QReadWriteLock m_block_length;

	Freq_Type smpl_freq;
	QReadWriteLock m_smpl_freq;

	double signal_length;
	QReadWriteLock m_signal_length;

	std::string bufname;
	QReadWriteLock m_bufname;

	Blocks_Type total_blocks;
	QReadWriteLock m_total_blocks;

	Samples_Type total_samples;
	QReadWriteLock m_total_samples;

	std::vector<itpp::cvec> Buf;

	QReadWriteLock m_nBufProducer, m_nBufConsumer;
	Blocks_Type nBufProducer, nBufConsumer;

	Blocks_Type WFinished;
	QSemaphore freeBuffers, usedBuffers;

	int WaitTimeout;

};

#endif
