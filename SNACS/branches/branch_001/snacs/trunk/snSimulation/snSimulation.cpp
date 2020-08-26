/*
 *  SNACS - The Satellite Navigation Radio Channel Simulator
 *
 *  Copyright (C) 2012  F. M. Schubert
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
 * \file snSimulation.cpp
 *
 * \author Frank Schubert
 */

#include "snSimulation.h"

#include <QDateTime>
#include <QDir>
#include <QTextStream>

// source modules
#include "../snModules/snSources/snSignalGenerate.h"
//#include "../snModules/snSources/snSignalInputBinary.h"

// processor modules
#include "../snModules/snProcessors/snProcessorADC.h"
#include "../snModules/snProcessors/snProcessorLPF.h"
#include "../snModules/snProcessors/snProcessorChannel.h"
//#include "../snModules/snProcessors/snMultiplicator.h"
#include "../snModules/snProcessors/snProcessorNoise.h"
//
//// sink modules
#include "../snModules/snSinks/snSDRStandard.h"
//#include "../snModules/snSinks/snSignalOutputBinary.h"
#include "../snModules/snSinks/snSignalOutputWidget.h"

snSimulationException::snSimulationException(const char *s) {
	exceptionDescription = s;
}

snSimulationException::snSimulationException(std::string s) {
	exceptionDescription = s;
}

bool doubles_equal(double a, double b) {
	return std::abs(a - b) <= DBL_EPSILON;
}

bool isInt(double a) {
	return (a == floor(a));
}

snSimulation::snSimulation() {
	log_level = snModule::info;

}

snSimulation::~snSimulation() {

}

void snSimulation::set_log_level(const snModule::LogType &t) {
	log_level = t;
}

/*
 * log messages to txt file.
 */
void snSimulation::log(const QString &s, snModule::LogType t) { // TODO
//	std::cout << s.toStdString() << "\n";

	if (t > log_level)
		return;

	QString msg;
	switch (t) {
	case snModule::critical:
		msg = "error: ";
		break;
	case snModule::warning:
		msg = "warning: ";
		break;
	case snModule::info:
		msg = "info: ";
		break;
	case snModule::debug:
		msg = "debug: ";
		break;
	case snModule::debugv:
		msg = "debug: ";
		break;
	}

	QTime Dd;
	QString Ds;
	Ds = QString("[") + Dd.currentTime().toString("hh:mm:ss.zzz") + QString("] ");

	QTextStream logfile(&logfile_handle);

	//		logfile << Ds.toStdString() << s.toStdString() << "\n";
	QString logstr = Ds + msg + s;
	logfile << logstr << "\n";
	logfile.flush();

	emit LogSignal(logstr, t);
}

std::vector<snModulePtr> snSimulation::setup_simulation(libconfig::Setting &set,
		widget_factory_base *widget_factory) {
	try {
		SNSignal Sig;
		ProcessingParameters SigProc;
		SigProc.ProcessingDelay = 0;

		// check if output directory exists:
		std::string output_directory_str = static_cast<const char *>(set["result_directory"]);

		QString out_dir_str = output_directory_str.c_str();
		if (static_cast<bool>(set["append_date_to_outdir"]))
			out_dir_str += QDateTime::currentDateTime().toString("_yyyy-MM-dd_HH-mm-ss");

		QDir out_dir;

// TODO: remove trailing slash or backslash here

		if (!out_dir.exists(out_dir_str)) {
			log("result_directory " + out_dir_str + " does not exist. creating it now.");
			out_dir.mkpath(out_dir_str);
		} else {
			throw snSimulationException(
					"Error: Result directory '" + out_dir_str.toStdString()
							+ "' exists already. Try adding parameter 'Simulation.append_date_to_outdir = true;' This will create a new output directory for every simulation run.");
		}

//		std::cout << "open file " << QString(out_dir_str + "/SNACS-log.txt").toStdString().c_str() << std::endl;

		logfile_handle.setFileName(out_dir_str + "/SNACS-log.txt");
		if (!logfile_handle.open(QIODevice::WriteOnly | QIODevice::Text)) {
			throw snSimulationException("snMainWindow: cannot open log file. exit.");
		}

		// read c0
		double val = set["c0"];
		if (val < 0) {
			throw snSimulationException("Simulation.c0 must be greater than 0.");
		}
		Sig.c0 = val;
		log("c0: " + QString::number(Sig.c0) + " m/s");

// open results file
		Sig.results_directory = out_dir_str.toStdString();
		Sig.results_file = new snWriteResultFile(QString(out_dir_str + "/result.h5").toStdString());

// should we store signals in txt files?
		Sig.write_txt_data = false;

		Sig.txt_data_block_interval = 0;
		Sig.nof_txt_samples = 0;
		Sig.txt_data_path = "";

		if (set.exists("Debug")) {
			Sig.write_txt_data = set["Debug"]["write_txt_data"];
			if (Sig.write_txt_data == true) {
				std::cout << "storing txt signals\n";
				Sig.txt_data_block_interval = set["Debug"]["txt_data_block_interval"];
				Sig.nof_txt_samples = set["Debug"]["nof_txt_samples"];

				// create all necessary directories:
				const double signal_length = set["Modules"][0]["signal_length"];
				const double block_length = set["Modules"][0]["block_length"];
				const Blocks_Type nof_conc_bufs = floor(signal_length / block_length);
				Sig.txt_data_path = out_dir_str.toStdString() + "/txt_signals/";
				std::cout << "MainWindow: Sig.txt_data_path: " << Sig.txt_data_path << "\n";

				for (size_t k = 0; k <= nof_conc_bufs; k += Sig.txt_data_block_interval) {
					const int blk_num = k / Sig.txt_data_block_interval;
					std::cout << "create directory " << blk_num << "\n";
					QDir txt_dir;
					txt_dir.mkpath(Sig.txt_data_path.c_str() + QString::number(blk_num));
				}
			}
		}

		Sig.plot_buffer_length = set["plot_buffer_length"]; // in s

		/* TODO
		 if (Sig.plot_buffer_length > static_cast<double> (Sig.BufSize) / Sig.SmplFreq) {
		 log("abort.");
		 throw snSimulationException(
		 "Plotting.plot_buffer_length must be smaller or equal than the length of a concbuf.");
		 }
		 */

// setup modules:
		size_t nof_modules = set["Modules"].getLength();
		log("setting up " + QString::number(nof_modules) + " modules");

		std::vector<snModulePtr> snModules; ///< vector of base class pointers for all snBlocks

		for (size_t i = 0; i < nof_modules; i++) {

			const std::string current_module_str = static_cast<const char *>(set["Modules"][i]["Type"]);

			// TODO: parameter checks missing, e.g. check for exactly 1 source block, exactly 1 sink block, etc.
			if (current_module_str == "snSignalGenerate") {
				snModulePtr new_module(new snSignalGenerate(set["Modules"][i], Sig, widget_factory));
				snModules.push_back(new_module);

				log("setup of module SignalGenerate complete.");
//			} else if (current_module_str == "snSignalInputBinary") {
//				snWidget *new_widget = new snWidgetQwt(0, "Signal Input Binary", snWidgetQwt::INLINE));
//
//				snModules.push_back(
//						new snSignalInputBinary(cfg.lookup(QString(BlockPath).toStdString()), Sig,
//								snWs.back()));
//
//				log("setup of snBlock snSignalInputBinary complete.");
			} else if (current_module_str == "snLowPassFilter") {
				snModulePtr new_module(
						new snProcessorLPF(set["Modules"][i], Sig, SigProc, widget_factory,
								snModules.back()->get_conc_buf()));
				snModules.push_back(new_module);

				log("setup of snBlock snLowPassFilter complete.");
			} else if (current_module_str == "snADC") {
				snModulePtr new_module(
						new snProcessorADC(set["Modules"][i], Sig, widget_factory,
								snModules.back()->get_conc_buf()));
				snModules.push_back(new_module);

				log("setup of snBlock snADC complete.");
			} else if (current_module_str == "snNoise") {
				snModulePtr new_module(
						new snProcessorNoise(set["Modules"][i], Sig, widget_factory,
								snModules.back()->get_conc_buf()));
				snModules.push_back(new_module);

				log("setup of module snNoise complete.");
//			} else if (current_module_str == "snMultiplicator") {
//				snWidget *new_widget = new snWidgetQwt(this, "Multiplicator", snWidgetQwt::INLINE));
//
//				snModules.push_back(
//						new snMultiplicator(cfg.lookup(QString(BlockPath).toStdString()), Sig, snWs.back(),
//								snModules.back()->get_conc_buf()));
//
//				log("setup of snBlock snMultiplicator complete.");
			} else if (current_module_str == "snChannel") {
				snModulePtr new_module(
						new snProcessorChannel(set["Modules"][i], Sig, SigProc, widget_factory,
								snModules.back()->get_conc_buf()));
				snModules.push_back(new_module);

				log("setup of snBlock snChannel complete.");
			} else if (current_module_str == "snSDRStandard") {
				snModulePtr new_module(
						new snSDRStandard(set["Modules"][i], Sig, SigProc, widget_factory,
								snModules.back()->get_conc_buf()));
				snModules.push_back(new_module);

				log("setup of module snSDRStandard complete.");
//			} else if (current_module_str == "snSignalOutputBinary") {
//
//				snWidget *new_widget = new snWidgetQwt(this, "Signal Output Binary", snWidgetQwt::INLINE));
//				snModules.push_back(
//						new snSignalOutputBinary(cfg.lookup(QString(BlockPath).toStdString()), Sig, SigProc,
//								snWs.back(), snModules.back()->get_conc_buf()));
//
//				log("setup of snBlock SignalOutputBinary complete.");
			} else if (current_module_str == "snSignalOutputWidget") {
				snModulePtr new_module(
						new snSignalOutputWidget(set["Modules"][i], Sig, widget_factory,
								snModules.back()->get_conc_buf()));
				snModules.push_back(new_module);

				log("setup of module SignalOutputWidget complete.");
			} else {
				throw snSimulationException("Unknown module type " + current_module_str);
			}

			// connect log signal of snModule to log slot of snSimulation (this)
			connect(snModules.back().get(), SIGNAL(LogSignal(const QString &, const snModule::LogType &)),
					this, SLOT(log(const QString &, const snModule::LogType &)));
		}
		return snModules;
	} catch (snSimulationException e) { // write error to log file and throw
		log(e.what(), snModule::critical);
		throw;
		// catch failure caused by the H5File operations
	} catch (H5::Exception error) {
		std::string msg = "HDF5 error in function " + std::string(error.getCFuncName()) + ": "
				+ std::string(error.getCDetailMsg());
		error.printError();
		log(msg.c_str(), snModule::critical);
		throw snSimulationException(msg);
	}
//	// catch failure caused by the DataSet operations
//	catch (DataSetIException error) {
//		error.printError();
//		std::abort();
//	}
//	// catch failure caused by the DataSpace operations
//	catch (DataSpaceIException error) {
//		error.printError();
//		std::abort();
//	}
//	// catch failure caused by the DataSpace operations
//	catch (DataTypeIException error) {
//		error.printError();
//		std::abort();
//	}

// save a copy of the configuration file to the output directory
//	cfg.writeFile(QString(out_dir_str + "/config.cfg").toStdString().c_str());
}
