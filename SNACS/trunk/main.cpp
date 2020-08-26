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
 * \file main.cpp
 *
 * \brief SNACS main program file.
 *
 * \author Frank Schubert
 */

#include <QApplication>

#include <tclap/CmdLine.h>

#include "snSimulation/snSimulation.h"
#include "snSimulation/snMainWindow.h"

#ifdef __unix__
#include <X11/Xlib.h>
#endif

int main(int argc, char **argv) {

	std::cout << "\n==== " << WelcomeMessage << " " << " ====\n\n";

	std::cout << "SNACS 0.1 Copyright (C) 2009  F. M. Schubert\n"
    	 << "This program comes with ABSOLUTELY NO WARRANTY.\n"
    	 << "This is free software, and you are welcome to redistribute it\n"
    	 << "under certain conditions. See the licenses/gpl-3.0.txt file for more information.\n";

	QString ConfigFile;
	bool StartSimulation = false;
	int DebugLevel = 0;

	// parse command line
	try {

		TCLAP::CmdLine cmd("", ' ', SNACSVersion);

		TCLAP::ValueArg<int> debugArg("d", "debug", "debug level (0...9)",
				false, 0, "int", cmd);

		TCLAP::ValueArg<std::string> nameArg("f", "file",
				"load configuration file", false, "", "string", cmd);

		TCLAP::SwitchArg StartSimulationSwitch("s", "start",
				"start simulation right after loading configuration file",
				false);
		cmd.add(StartSimulationSwitch);

		TCLAP::SwitchArg ConsoleSwitchArg("c", "console",
				"do not use the GUI, print all messages to the console", false);
		cmd.add(ConsoleSwitchArg);

		// Parse the argv array.
		cmd.parse(argc, argv);

		// Get the value parsed by each arg.
		std::string ConfigFileArg = nameArg.getValue();
		ConfigFile = ConfigFileArg.c_str();
		StartSimulation = StartSimulationSwitch.getValue();
		DebugLevel = debugArg.getValue();

	} catch (TCLAP::ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId()
				<< std::endl;
	}

#ifdef __unix__
	XInitThreads(); // make X11 output thread-safe
#endif

	QApplication App(argc, argv);

	// set up main window
	try {
		snMainWindow MainWindow(StartSimulation);
		MainWindow.setDebugLevel(DebugLevel);

		//MainWindow.resize(QApplication::desktop()->screenGeometry().width()-10, .25*QApplication::desktop()->screenGeometry().height());
		MainWindow.showMaximized();

		MainWindow.log(WelcomeMessage.c_str());

#ifdef USE_SSE
		MainWindow.log("Using SSE instructions.");
#else
		MainWindow.log("Not using SSE instructions.");
#endif
		if (ConfigFile != "") {
			MainWindow.loadFile(ConfigFile);
			if (StartSimulation && MainWindow.getSimStatus()
					== snMainWindow::Ready)
				MainWindow.startSimu();

		} else if (StartSimulation) {
			MainWindow.logErr("Cannot start simulation: no config file given");
		}

		App.exec();

	} catch (snSimulationException e) {
		std::cout << "error: " << e.what().c_str() << "\n";
		std::exit(1);
	} catch (std::exception &e) {
		std::cout << "error: " << e.what() << "\n";
		// catch failure caused by the H5File operations
	} catch (FileIException error) {
		error.printError();
		std::exit(1);
	}
	// catch failure caused by the DataSet operations
	catch (DataSetIException error) {
		error.printError();
		std::exit(1);
	}
	// catch failure caused by the DataSpace operations
	catch (DataSpaceIException error) {
		error.printError();
		std::exit(1);
	}
	// catch failure caused by the DataSpace operations
	catch (DataTypeIException error) {
		error.printError();
		std::exit(1);
	} catch (...) {
		std::cout << "error: unknown exception thrown.\n";
		std::exit(1);
	}
	return 0;
}

