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
 * \file snSimulation.h
 *
 * \brief Declarations common to all SNACS source files.
 *
 * \author Frank Schubert
 */

#ifndef SNSIMULATION_H_
#define SNSIMULATION_H_

#include <tr1/memory>

#include "snGlobal.h"
#include "snWriteResultFile.h"
#include "../snModules/snModule.h"
#include "../snWidget/snWidget.h"

#include <QString>
#include <QFile>

typedef std::tr1::shared_ptr<snModule> snModulePtr;

class snSimulation: public QObject {
Q_OBJECT

public:
	snSimulation();
	~snSimulation();

	std::vector<snModulePtr> setup_simulation(libconfig::Setting &set, widget_factory_base *widget_factory);

public slots:
	void log(const QString &s, snModule::LogType t = snModule::info);
	void set_log_level(const snModule::LogType &t);

signals:
	/**
	 * \brief is connected in snMainWindow to the log text widget.
	 *
	 */
	void LogSignal(const QString &mess, const snModule::LogType &d);

private:
	snModule::LogType log_level;
	QFile logfile_handle;
};

#endif /* SNSIMULATION_H_ */
