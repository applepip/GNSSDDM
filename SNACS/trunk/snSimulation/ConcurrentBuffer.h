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
 * \file ConcurrentBuffer.h
 *
 * \author Frank Schubert
 */

#ifndef DEF_CONCBUF
#define DEF_CONCBUF

#include <cstdlib>
#include <iostream>
#include <complex>
#include <QSemaphore>

#include "snSimulation.h"

/**
 * \brief ConcBuf is a class that implements a circular buffer for concurrent access from two distinct threads.
 *
 * The constructor sets up a concbuf consisting of an amount of arrays, called concbuf blocks. The concbuffer is
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
class ConcBuf {

public:
	/**
	 * \brief Concurrent buffer (concbuf) constructor
	 *
	 * \param <long aBuf> amount of concbuf blocks (arrays) per concbuf
	 * \param <Samples_Type sBuf> length of one concbuf block
	 * \param <std::string pbufname> name of the concbuffer
	 */
	ConcBuf(Blocks_Type aBuf, Samples_Type sBuf, std::string pbufname);
	~ConcBuf();

	/**
	 * \brief snBlocks call RequestWriting to block the next concbuf block for write access
	 * \return pointer to the first element of a concbuf block which is being locked for writing.
	 */
	std::complex<double> *RequestWriting();

	/**
	 * \brief releases write lock to a concbuf block
	 * \param <long aDataWritten> amount of data written
	 * \param <long WFinished> 0 for continued operation. 1 for indication that this
	 * concbuf block was the last one to write to.
	 */
	void ReleaseWriting(Samples_Type aDataWritten, bool WFinished);

	/**
	 * \brief request read lock to next concbuf block
	 * \param <Samples_Type *nToRead> this variable will contain the amount of elements
	 * that can be read from the concbuf block
	 * \return pointer to the first element of the concbuf block
	 */
	std::complex<double> *RequestReading(Samples_Type *nToRead);

	/**
	 * \brief releases read lock
	 *
	 */
	bool ReleaseReading();

private:
	std::string bufname;
	std::complex<double> *Buf;
	Samples_Type BufSize;
	Blocks_Type maxBuf;

	Samples_Type *nToBeRead;
	Blocks_Type nBufProducer, nBufConsumer;

	Blocks_Type WFinished;
	QSemaphore freeBuffers, usedBuffers;

	int WaitTimeout;
};

#endif
