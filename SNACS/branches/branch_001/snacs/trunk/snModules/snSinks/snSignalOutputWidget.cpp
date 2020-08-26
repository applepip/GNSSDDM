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

#include "snSignalOutputWidget.h"

snSignalOutputWidget::snSignalOutputWidget(libconfig::Setting &Settings, SNSignal pSNSignal,
		widget_factory_base *widget_factory, ConcBuf3* _circbuf) :
		snConsumingModule(pSNSignal, _circbuf)

{
	nBufs = Settings["buffers_to_display"];

	snw = widget_factory->get_new_widget("Signal Output");

	curve = new snCurveComplex(nBufs * block_size, snCurve::blue, snCurve::red);
	snPlot *plot_complex = snw->get_new_plot_complex("real / imag", snPlot::Linear, curve);
	plot_complex->setXAxis(0.0, nBufs * block_size / smpl_freq);
	plot_complex->setXAxisLabel("t [s]");
	plot_complex->setYAxisLabel("A");
	snw->addPlot(plot_complex);

//	snPlot *plot_fft_mag = snw->get_new_plot_complex("FFT, Magnitude", snPlot::FFTMagnitudeDB, curve);
//	snw->addPlot(plot_fft_mag);

//	snPlot *plot_real = snw->get_new_plot_real("real", snPlot::Linear);
//	snPlot *plot_imag = snw->get_new_plot_real("imag", snPlot::Linear);
//	plot_real->addCurve(curve);
//	// TODO add curves 2x
//	snw->addPlot(plot_real);
//	snw->addPlot(plot_imag);

//	curve_abs = snw->get_new_curve_real(nBufs * block_size, snCurve::black);
//	snPlot *plot_abs = snw->get_new_plot_real("absolute", snPlot::Linear);
//	plot_abs->addCurve(curve_abs);
////	plot_abs->setYAxis(-2.5, 2.5);//				curve_magn->update_value_X(o, time_val);
//	//				curve_magn->update_value_Y(o, VisBuf[o]);
//
//	plot_abs->setXAxis(0.0, get_max_xaxis_value());
//	plot_abs->setXAxisLabel("t [s]");
//	plot_abs->setYAxisLabel("A");
//	snw->addPlot(plot_abs);

//	curve_magn = snw->get_new_curve_complex(nBufs * block_size, snCurve::blue, snCurve::red);
	snPlot *plot_magn = snw->get_new_plot_complex("magnitude", snPlot::Linear, curve);
//	plot_magn->addCurve(curve);
////	plot_abs->setYAxis(-2.5, 2.5);
	plot_magn->setXAxis(0.0, get_max_xaxis_value());
	plot_magn->setXAxisLabel("t [s]");
	plot_magn->setYAxisLabel("A");
	snw->addPlot(plot_magn);

	//
//	curve_phase = snw->get_new_curve_real(nBufs * block_size, snCurve::blue);
//	curve_phase->setCurveSymbol(snCurve::CIRCLE, snCurve::blue, snCurve::static_cast<double>(o) / smpl_freq)SMALL);
//	curve_phase->setCurveStyle(snCurve::DOTS);
//	snPlot *plot_phase = snw->get_new_plot_real("I/Q plot", snPlot::Linear); //, curve_phase
//	plot_phase->enableGrid();
//	plot_phase->setXAxis(-2.5, 2.5);
//	plot_phase->setYAxis(-2.5, 2.5);
//	snw->addPlot(plot_phase);

// visualize nBufs buffers
	VisBuf = new std::complex<double>[block_size * nBufs];
}

void snSignalOutputWidget::run() {
	snLog("SignalOutput: start.");

	Blocks_Type act_block = 0;
	while (isActive()) {
		for (unsigned int o = 0; o < nBufs; o++) {
			itpp::cvec& Buf = circbuf->RequestReading();
			for (Samples_Type i = 0; i < block_size; i++) {
				VisBuf[i + (o * block_size)] = Buf[i];
			}
			if (circbuf->ReleaseReading()) {
				snLog("SignalOutputWidget: exit.");
				printSummary("SignalOutputWidget");
				return;
			}
			act_block++;
		}

		// plot
		if (snw->requestPlotFlag()) {
			for (Samples_Type o = 0; o < block_size * nBufs; o++) {
				const double time_val = static_cast<double>(o) / smpl_freq;
				curve->update_value_X(o, time_val);
				curve->update_value_Y(o, VisBuf[o]);
//				curve_magn->update_value_X(o, time_val);
//				curve_magn->update_value_Y(o, VisBuf[o]);
//				curve_abs->update_value_X(o, time_val);
//				curve_abs->update_value_Y(o, abs(VisBuf[o]));
//			curve_phase->update_value_X(o, VisBuf[o].real());
//			curve_phase->update_value_Y(o, VisBuf[o].imag());
			}
			snw->releasePlotFlag();
		}

		// set the current simulation time
		set_current_simulation_time(
				act_block / static_cast<double>(circbuf->get_total_blocks()) * signal_length);
	}
}
