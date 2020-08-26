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
 * \file snWriteResultFile.h
 *
 * \author Frank Schubert
 */

#ifndef SNWRITERESULTFILE_H_
#define SNWRITERESULTFILE_H_

#include <string>
#include <vector>
#include <complex>

#include <itpp/itbase.h>

#include "H5Cpp.h"

#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

/**
 * \brief opens a HDF5 file for output. Every snBlock in the simulation chain can write
 * simulation result data to that file with the write functions.
 */
class snWriteResultFile {
public:
	/**
	 * \brief constructor
	 *
	 * \param <std::string filename> file name
	 */
	snWriteResultFile(std::string filename);
	virtual ~snWriteResultFile();

	/**
	 * \brief creates a group in the result HDF5 file
	 */
	void createGroup(std::string name);

	/**
	 * \brief writes a single double value
	 *
	 * \param <std::string path> the path inside the HDF5 file to write to
	 * \param <double data> the value to write
	 */
	void write(std::string path, double data);

	/**
	 * \brief writes a double array
	 *
	 * \param <std::string path> the path inside the HDF5 file to write to
	 * \param <double *data> pointer to double array
	 * \param <unsigned long num> number of elements to write
	 */
	void write(std::string path, double *data, unsigned long num);

	/**
	 * \brief writes a std string
	 *
	 * \param <std::string path> the path inside the HDF5 file to write to
	 * \param <std::string *data> string
	 */
	void write(std::string path, std::string data);

	/**
	 * \brief writes a std::vector &.
	 *
	 * \param <std::string path> the path inside the HDF5 file to write to
	 * \param <std::vector<double> &data> reference to the vector
	 */
	void write(std::string path, std::vector<double> &data, unsigned long num);

	/**
	 * \brief writes a std::complex vector.
	 *
	 * \param <std::string path> the path inside the HDF5 file to write to
	 * \param <std::vector<std::complex<double> > &data> reference to the complex vector
	 */
	void write(std::string path, std::vector<std::complex<double> > &data,
			unsigned long num);

	/**
	 * \brief writes an array of complex doubles
	 *
	 * \param <std::string path> the path inside the HDF5 file to write to
	 * \param <std::complex<double> *data> pointer to the array
	 * \param <unsigned long num> amoun of elements to write
	 */
	void write(std::string path, std::complex<double> *data, unsigned long num);

	/**
	 * \brief writes an itpp vector
	 *
	 * \param <std::string path> the path inside the HDF5 file to write to
	 * \param itpp vector
	 */
	void write_vec(std::string path, itpp::vec data);

	/**
	 * \brief writes an itpp vector
	 *
	 * \param <std::string path> the path inside the HDF5 file to write to
	 * \param itpp vector
	 */
	void write_cvec(std::string path, itpp::cvec data);

	DataSet* initialize_dataset(std::string path, unsigned long len);
	void write_to_dataset(DataSet* dataset, unsigned long index, double data);

private:
	H5::H5File *f;
};

#endif /* SNWRITERESULTFILE_H_ */
