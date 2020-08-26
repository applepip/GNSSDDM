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

snSignalOutputWidget::snSignalOutputWidget(libconfig::Setting &Settings, SNSignal pSNSignal, snWidget *psnw,
		ConcBuf* pConcBuf1) :
	snBlock(pSNSignal), snw(psnw), ConcBuf1(pConcBuf1)

{
	nBufs = Settings["buffers_to_display"];

	curve = new snCurveComplex(nBufs * Sig.BufSize, Qt::blue, Qt::red);
	snPlotComplexReal *plot_real = new snPlotComplexReal("real", snPlot::Linear, curve);
	snPlotComplexImag *plot_imag = new snPlotComplexImag("imag", snPlot::Linear, curve);
	snw->addPlot(plot_real);
	snw->addPlot(plot_imag);

	curve_abs = new snCurveReal(nBufs * Sig.BufSize, Qt::blue);
	snPlotReal *plot_abs = new snPlotReal("magnitude", snPlot::Linear, curve_abs);
	plot_abs->setYAxis(-2.5, 2.5);
	snw->addPlot(plot_abs);

	curve_phase = new snCurveReal(nBufs * Sig.BufSize, Qt::blue);
	curve_phase->setCurveSymbol(snCurve::CURVEI, QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::blue), QPen(
			Qt::blue), QSize(3, 3)));
	curve_phase->setCurveStyle(snCurve::CURVEI, QwtPlotCurve::Dots);
	snPlotReal *plot_phase = new snPlotReal("I/Q plot", snPlot::Linear, curve_phase);
	plot_phase->enableGrid();
	plot_phase->setXAxis(-2.5, 2.5);
	plot_phase->setYAxis(-2.5, 2.5);
	snw->addPlot(plot_phase);

	// visualize nBufs buffers
	VisBuf = new std::complex<double>[Sig.BufSize * nBufs];
}

void snSignalOutputWidget::run() {
	snLog("SignalOutput: start.");

	std::complex<double> *Buf;
	Samples_Type nToRead;

	//	std::complex<double> tmp;

	while (isActive()) {
		for (unsigned int o = 0; o < nBufs; o++) {
			Buf = ConcBuf1->RequestReading(&nToRead);
			//				std::cout << " nToRead: " << nToRead;
			//			std::cout.flush();
			for (Samples_Type i = 0; i < nToRead; i++) {
				//		Samples_Type t = i + (o * Sig.BufSize);
				//	std::cout << t << " ";
				/*			if (t > Sig.BufSize * nBufs) {
				 }
				 */
				/*			try {
				 } catch (...) {
				 */
				//				std::cout << " error ";
				//}
				VisBuf[i + (o * Sig.BufSize)] = Buf[i];
			}
			if (ConcBuf1->ReleaseReading()) {
				snLog("SignalOutputWidget: exit.");
				;
				printSummary("SignalOutputWidget");
				return;
			}
			increaseBlks();
			increaseOutputBlks();
		}

		// plot

		snw->pushData();
		for (Samples_Type o = 0; o < Sig.BufSize * nBufs; o++) {
			curve->X.at(o) = o;
			curve->Y.at(o) = VisBuf[o];
			curve_abs->X.at(o) = o;
			curve_abs->Y.at(o) = abs(VisBuf[o]);
			curve_phase->X.at(o) = VisBuf[o].real();
			curve_phase->Y.at(o) = VisBuf[o].imag();
		}
		snw->releasePlotFlag();

	}
}
