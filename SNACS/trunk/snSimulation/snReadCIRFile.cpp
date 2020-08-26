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

/*!
 * \file	snReadCIRFile.cpp
 * \brief	Implementation of reading channel impulse responses from a HDF5 file.
 *
 * \author	Frank Schubert
 */

#include "snReadCIRFile.h"

#include <sstream>

snReadCIRFile::snReadCIRFile(std::string filename) {
	// Try block to detect exceptions raised by any of the calls inside it
	try {
		/*
		 * Turn off the auto-printing when failure occurs so that we can
		 * handle the errors appropriately
		 */
		//		Exception::dontPrint();

		/*
		 Open the file
		 */
		file = new H5File(filename.c_str(), H5F_ACC_RDONLY);

		// read simulation parameters ///////////////////////////////////////

//		CompType cp_parameters_simulation(sizeof(parameters_simulation_struct));
//		cp_parameters_simulation.insertMember("SampFreq", HOFFSET(
//				parameters_simulation_struct, cir_rate),
//				H5::PredType::NATIVE_DOUBLE);
//		cp_parameters_simulation.insertMember("MaximumSpeed", HOFFSET(
//				parameters_simulation_struct, MaximumSpeed),
//				H5::PredType::NATIVE_DOUBLE);
//		cp_parameters_simulation.insertMember("SatElevation", HOFFSET(
//				parameters_simulation_struct, SatElevation),
//				H5::PredType::NATIVE_DOUBLE);
//		cp_parameters_simulation.insertMember("SatAzimut", HOFFSET(
//				parameters_simulation_struct, SatAzimut),
//				H5::PredType::NATIVE_DOUBLE);
//		cp_parameters_simulation.insertMember("NumberOfSteps", HOFFSET(
//				parameters_simulation_struct, cir_amount),
//				H5::PredType::NATIVE_DOUBLE);
//		cp_parameters_simulation.insertMember("c0", HOFFSET(
//				parameters_simulation_struct, c0), H5::PredType::NATIVE_DOUBLE);

		DataSet(file->openDataSet("/parameters/simulation/c0")).read(
				&my_params.c0, H5::PredType::NATIVE_DOUBLE);

		DataSet(file->openDataSet("/parameters/simulation/cir_rate")).read(
				&my_params.cir_rate, H5::PredType::NATIVE_DOUBLE);

		double cir_amount_double;
		DataSet(file->openDataSet("/parameters/simulation/cir_amount")).read(
				&cir_amount_double, H5::PredType::NATIVE_DOUBLE);

		my_params.cir_amount = static_cast<Blocks_Type> (cir_amount_double);

		// read cirs amount ////////////////////////////////////////////////////////////////////////////////////
		group_cirs = new Group(file->openGroup("/cirs"));

		hsize_t cirs_amount;
		H5Gget_num_objs(group_cirs->getId(), &cirs_amount);
		if (cirs_amount != getCIRAmount()) {
			throw snSimulationException(
					"snReadCIRFile: cir_amount in file != cirs amount!");
		}

		// read reference_range ////////////////////////////////////////////////////////////////////////////////////
		ref_ranges = new double[getCIRAmount()];
		hsize_t dimsf3[] = { getCIRAmount() }; // dataset dimensions
		DataSpace dspace3(1, dimsf3);
//		DataSet(file->openDataSet("/reference_range/range_increments")).read(ref_ranges, H5::PredType::NATIVE_DOUBLE, dspace3);
		DataSet(file->openDataSet("/reference_range/range_absolut")).read(ref_ranges, H5::PredType::NATIVE_DOUBLE, dspace3);

	} // end of try block
	// catch failure caused by the H5File operations
	catch (FileIException error) {
		error.printError();
		std::stringstream err_msg;
		err_msg <<  "HDF5 error: ";
		err_msg << error.getCDetailMsg() << " in function " << error.getCFuncName();
		throw snSimulationException(err_msg.str());
	}

	// catch failure caused by the DataSet operations
	catch (DataSetIException error) {
		error.printError();
		std::stringstream err_msg;
		err_msg <<  "HDF5 error: ";
		err_msg << error.getCDetailMsg() << " in function " << error.getCFuncName();
		throw snSimulationException(err_msg.str());
	}

	// catch failure caused by the DataSpace operations
	catch (DataSpaceIException error) {
		error.printError();
		std::stringstream err_msg;
		err_msg <<  "HDF5 error: ";
		err_msg << error.getCDetailMsg() << " in function " << error.getCFuncName();
		throw snSimulationException(err_msg.str());
	}

	// catch failure caused by the DataSpace operations
	catch (DataTypeIException error) {
		error.printError();
		std::stringstream err_msg;
		err_msg <<  "HDF5 error: ";
		err_msg << error.getCDetailMsg() << " in function " << error.getCFuncName();
		throw snSimulationException(err_msg.str());
	}

}

cir_struct snReadCIRFile::getCIR(Blocks_Type cir_num) {
	cir_struct result_cir;

	try {
		std::ostringstream cir_number_str;
		cir_number_str << cir_num;
		char *cir_cstr = new char[cir_number_str.str().size() + 1];
		strcpy(cir_cstr, cir_number_str.str().c_str());

		DataSet dataset = DataSet(group_cirs->openDataSet(cir_cstr));

		DataSpace dataspace = DataSpace(dataset.getSpace());

		CompType cp_echo(sizeof(echo_struct_hdf5));

		cp_echo.insertMember("delay", HOFFSET(echo_struct_hdf5, delay),
				PredType::NATIVE_DOUBLE);
		cp_echo.insertMember("real", HOFFSET(echo_struct_hdf5, real),
				PredType::NATIVE_DOUBLE);
		cp_echo.insertMember("imag", HOFFSET(echo_struct_hdf5, imag),
				PredType::NATIVE_DOUBLE);

		int echoes_amount = dataspace.getSimpleExtentNpoints();

		echo_struct_hdf5 echoes[echoes_amount];
		dataset.read(echoes, cp_echo);

		result_cir.echoes.resize(echoes_amount);
		for (int i = 0; i < echoes_amount; i++) {
			result_cir.echoes.at(i).delay = echoes[i].delay;
			result_cir.echoes.at(i).weight.real() = echoes[i].real;
			result_cir.echoes.at(i).weight.imag() = echoes[i].imag;
		}
		result_cir.ref_range = ref_ranges[cir_num];
	}
	// catch failure caused by the H5File operations
	catch (FileIException error) {
		//		throw snSimulationException("hdf5 prob");
		error.printError();
	}

	// catch failure caused by the DataSet operations
	catch (DataSetIException error) {
		//		throw snSimulationException("hdf5 prob");
		error.printError();
	}

	// catch failure caused by the DataSpace operations
	catch (DataSpaceIException error) {
		//		throw snSimulationException("hdf5 prob");
		error.printError();
	}

	// catch failure caused by the DataSpace operations
	catch (DataTypeIException error) {
		//		throw snSimulationException("hdf5 prob");
		error.printError();
	}
	return result_cir;
}

snReadCIRFile::~snReadCIRFile() {
	delete group_cirs;
	delete file;
}
