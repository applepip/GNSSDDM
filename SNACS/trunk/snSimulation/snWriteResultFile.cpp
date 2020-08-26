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
 * \file snWriteResultFile.cpp
 *
 * \author Frank Schubert
 */

#include "snWriteResultFile.h"

snWriteResultFile::snWriteResultFile(std::string filename) {
	f = new H5File(filename.c_str(), H5F_ACC_EXCL);
	// fail if file exists: H5F_ACC_EXCL
}

snWriteResultFile::~snWriteResultFile() {
	f->flush(H5F_SCOPE_GLOBAL);
	delete f;
}

void snWriteResultFile::createGroup(std::string name) {
	H5::Group group(f->createGroup(name.c_str()));
}

void snWriteResultFile::write(std::string path, double data) {
	hsize_t dimsf[1]; // dataset dimensions
	dimsf[0] = 1;
	DataSpace dataspace(1, dimsf);
	DataSet dataset = f->createDataSet(path.c_str(), PredType::NATIVE_DOUBLE, dataspace);
	dataset.write(&data, PredType::NATIVE_DOUBLE);
}

void snWriteResultFile::write(std::string path, std::string data) {
	const char *data_out = data.c_str();

	hsize_t dimsf[1]; // dataset dimensions
	dimsf[0] = 1;
	DataSpace dataspace(1, dimsf);
	
    StrType str1(0, data.size());
    DataSet dataset = f->createDataSet(path.c_str(), str1, dataspace);
	dataset.write((void *)data_out, str1);
}

void snWriteResultFile::write(std::string path, double *data, unsigned long num) {
	hsize_t dimsf3[1]; // dataset dimensions
	dimsf3[0] = num;
	DataSpace dspace3(1, dimsf3);
	DataSet dset3 = f->createDataSet(path.c_str(), PredType::NATIVE_DOUBLE, dspace3);
	dset3.write(data, PredType::NATIVE_DOUBLE);
}

void snWriteResultFile::write(std::string path, std::vector<double> &data, unsigned long num) {
	hsize_t dimsf3[1]; // dataset dimensions
	dimsf3[0] = num;
	DataSpace dspace3(1, dimsf3);
	DataSet dset3 = f->createDataSet(path.c_str(), PredType::NATIVE_DOUBLE, dspace3);
	dset3.write(&data[0], PredType::NATIVE_DOUBLE);
}

void snWriteResultFile::write(std::string path, std::vector<std::complex<double> > &data, unsigned long num) {

	struct hdf5_complex {
		double real;
		double imag;
	};

	CompType cp_cmplx(sizeof(hdf5_complex));

	cp_cmplx.insertMember("real", HOFFSET(hdf5_complex, real), PredType::NATIVE_DOUBLE);
	cp_cmplx.insertMember("imag", HOFFSET(hdf5_complex, imag), PredType::NATIVE_DOUBLE);

	hdf5_complex wdata[num];

	hsize_t dimsf3[1]; // dataset dimensions
	dimsf3[0] = num;
	DataSpace dspace3(1, dimsf3);
	DataSet dset3 = f->createDataSet(path.c_str(), cp_cmplx, dspace3);

	for (unsigned long i = 0; i < num; i++) {
		wdata[i].real = data.at(i).real();
		wdata[i].imag = data.at(i).imag();
	}

	//dset3.write(wdata, PredType::NATIVE_DOUBLE);
	dset3.write(wdata, cp_cmplx);

}

void snWriteResultFile::write(std::string path, std::complex<double> *data, unsigned long num) {

	typedef struct hdf5_complex {
		double real;
		double imag;
	} hdf5_complex;

	CompType cp_cmplx(sizeof(hdf5_complex));

	cp_cmplx.insertMember("real", HOFFSET(hdf5_complex, real), PredType::NATIVE_DOUBLE);
	cp_cmplx.insertMember("imag", HOFFSET(hdf5_complex, imag), PredType::NATIVE_DOUBLE);

	hdf5_complex wdata[num];

	hsize_t dimsf3[1]; // dataset dimensions
	dimsf3[0] = num;
	DataSpace dspace3(1, dimsf3);
	DataSet dset3 = f->createDataSet(path.c_str(), cp_cmplx, dspace3);

	for (unsigned long i = 0; i < num; i++) {
		wdata[i].real = data[i].real();
		wdata[i].imag = data[i].imag();
	}

	dset3.write(wdata, cp_cmplx);
}
