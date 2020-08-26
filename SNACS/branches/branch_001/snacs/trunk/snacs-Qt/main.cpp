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

#include "snMainWindow.h"

#ifdef __unix__
#include <X11/Xlib.h>
#endif

int main(int argc, char **argv) {

#ifdef __unix__
	XInitThreads(); // make X11 output thread-safe
#endif

	QApplication App(argc, argv);

	// set up main window
	snMainWindow MainWindow(argc, argv);

	App.exec();

	return 0;
}

