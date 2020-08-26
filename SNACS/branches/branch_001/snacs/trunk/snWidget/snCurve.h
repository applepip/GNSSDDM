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

/*!
 * \file	snCurve.h
 * \brief	Contains implementations for snCurves to be used with snPlots.
 *
 * \author	Frank Schubert
 */

#ifndef SNCURVE_H_
#define SNCURVE_H_

#include <itpp/itbase.h>

#include <iostream>
#include <string>

/*!
 * \brief Base class for all curves that can be used with snPlots
 */
class snCurve {
public:
	enum CurveColor {
		black, blue, red, green, darkBlue, darkRed, darkGreen, darkMagenta
	};

	snCurve(int pn, CurveColor pcolor, std::string _name = "");
	virtual ~snCurve();

	double get_X(size_t i) {
		return X(i);
	}
	size_t get_max_points() const {
		return max_points;
	}
	size_t get_points_to_plot() const {
		return points_to_plot;
	}

	CurveColor get_color() const {
		return color;
	}
	std::string get_name() const {
		return name;
	}

	enum CurveStyle {
		LINES, STICKS, DOTS
	};
	virtual void setCurveStyle(CurveStyle _style) {
		style = _style;
	}
	CurveStyle get_style() const {
		return style;
	}

	enum CurveSymbol {
		NONE, CIRCLE
	};
	enum SymbolSize {
		SMALL, MEDIUM, LARGE
	};
	virtual void setCurveSymbol(CurveSymbol _symbol, CurveColor _color, SymbolSize _symbol_size) {
		symbol = _symbol;
		color = _color;
		symbol_size = _symbol_size;
	}
	CurveSymbol get_symbol() const {
		return symbol;
	}
	SymbolSize get_symbol_size() const {
		return symbol_size;
	}

	void setPointsToPlot(size_t p);
	virtual void update_value_X(size_t i, double v) {
		X(i) = v;
	}

	itpp::vec get_frequency_vector();
	itpp::cvec fft(const itpp::cvec &fft_in);

protected:
	size_t max_points, points_to_plot;
	CurveColor color;
	std::string name;
	CurveStyle style;
	CurveSymbol symbol;
	SymbolSize symbol_size;
	itpp::vec X; ///< holds x axis component
};

/*!
 * \brief	Implements a curve for real data
 */
class snCurveReal: public snCurve {
public:
	snCurveReal(int pn, CurveColor pcolor, std::string _name = "");
	~snCurveReal();

	void update_value_Y(size_t i, double v);

	void set_color(CurveColor _color) {
		color = _color;
	}

	double get_Y(size_t i) {
		return Y(i);
	}

	itpp::vec fft_magnitude();
	itpp::vec fft_phase();

protected:
	itpp::vec Y;
};

/*!
 * \brief	Implements a curve for complex data
 */
class snCurveComplex: public snCurve {
public:
	snCurveComplex(int pn, CurveColor pcolorI, CurveColor pcolorQ, std::string _name = "");
	~snCurveComplex();

	void update_value_X(size_t i, double v);
	void update_value_Y(size_t i, std::complex<double> v);

	void set_color(CurveColor cI, CurveColor cQ) {
		color = cI;
		colorQ = cQ;
	}
	CurveColor get_colorQ() const {
		return colorQ;
	}

	std::complex<double> get_Y(size_t i) {
		return Y(i);
	}

	itpp::vec fft_magnitude();
	itpp::vec fft_phase();

protected:
	CurveColor colorQ;
	itpp::cvec Y; ///< holds signal inphase and quadrature component
};

/*!
 * \brief	Implements a curve for 3d data
 */
class snCurve3D: public snCurve {
public:
	snCurve3D(int pnx, int pny);
	~snCurve3D();

	int ny;

	virtual void update_XY(size_t ix, size_t iy, double v);

	virtual double **get_SXY() {
		return SXY;
	}

protected:
	double **SXY; ///< holds 3D data
};

#endif /* SNCURVE_H_ */
