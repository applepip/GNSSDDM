/*
 *  SNACS - The Satellite Navigation Radio Channel Simulator
 *  console version
 *
 *  Copyright (C) 2012. M. Schubert
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

#include "../snSimulation/snGlobal.h"
#include "snConsole.h"

//#include <QApplication>
#include <QtCore/QCoreApplication>

#include <tclap/CmdLine.h>

#ifdef __unix__
#include <X11/Xlib.h>
#endif

int main(int argc, char **argv) {

//#ifdef __unix__
//	XInitThreads(); // make X11 output thread-safe
//#endif

	QCoreApplication App(argc, argv);

	// parse command line
	QString filename;

	TCLAP::CmdLine cmd("", ' ', SNACSVersion);

	TCLAP::ValueArg<int> debugArg("d", "debug", "debug level (1 or 2)", false, 0, "int", cmd);

	TCLAP::ValueArg<std::string> nameArg("f", "file", "load configuration file", false, "", "string", cmd);

	// Parse the argv array.
	cmd.parse(argc, argv);

	// Get the value parsed by each arg.
	std::string ConfigFileArg = nameArg.getValue();
	filename = ConfigFileArg.c_str();
	const int DebugLevel = debugArg.getValue();

	snModule::LogType log_level = snModule::info;
	if (DebugLevel == 1)
		log_level = snModule::debug;
	else if (DebugLevel == 2)
		log_level = snModule::debugv;

	snConsole prg(log_level);
	prg.load_config_file(filename);

	prg.start_simulation();

//	prg.stop_simulation();

	App.exec();

	return 0;
}

