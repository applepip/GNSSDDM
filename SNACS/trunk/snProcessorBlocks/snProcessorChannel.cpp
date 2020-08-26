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
 * \file snProcessorChannel.cpp
 *
 * \author Frank Schubert
 */

#include "snProcessorChannel.h"

snProcessorChannel::snProcessorChannel(libconfig::Setting &Settings, SNSignal pSNSignal,
		ProcessingParameters &ProcSig, snWidget *psnwCIR, snWidget *psnwFIR, snWidget *psnw,
		ConcBuf* pConcBuf1, ConcBuf* pConcBuf2) :
	snBlock(pSNSignal), cir_snw(psnwCIR), fir_snw(psnwFIR), out_snw(psnw), ConcBuf1(pConcBuf1),
			ConcBuf2(pConcBuf2) {

	snLog("snChannel: channel cir interpolation setup.");

	std::string interpolation_type_str = static_cast<const char *> (Settings["interpolation_type"]);

	if (interpolation_type_str == "SINC") {
		interpolation_type = SINC;
	} else if (interpolation_type_str == "RRC") {
		interpolation_type = RRC;
		rrc_rolloff_factor = Settings["rrc_rolloff_factor"];
	} else {
		throw snSimulationException("Unknown interpolation type.");
	}

	std::string filename = static_cast<const char *> (Settings["file"]);
	interpolation_bandwidth = Settings["interpolation_bandwidth"];

	file = new snReadCIRFile(filename);

	if (file->getC0() != Sig.c0) {
		throw snSimulationException("snChannel: c0 of simulation and c0 of CIR file differ!");
	}

	CIRrate = file->getCIRRate();
	aCIR = static_cast<Blocks_Type> (file->getCIRAmount());

	snLogDeb("snChannel: CIR amount in file: " + QString::number(aCIR) + ", CIR rate: "
			+ QString::number(CIRrate), 4);

	// are there enough CIRs for signal length?
	if ((static_cast<double> (aCIR) / CIRrate) < Sig.Len) {
		throw snSimulationException(
				"snChannel: Not enought CIRs in file for simulation signal length!");
	}

	// if there are more CIRs in file than simulation length, set CIR amount to simulation length
	if ((static_cast<double> (aCIR) / CIRrate) > Sig.Len) {
		aCIR = static_cast<Blocks_Type> (floor(Sig.Len * CIRrate));
	}

	snLogDeb("snChannel: will use " + QString::number(aCIR) + " CIRs from file.", 4);

	// scan through CIR file to find max delay.
	// but only for length of simulation
	MaxDelay = 0.0;

	// TODO MinDelay has to be added here!

	//MaxPR = 0.0;
	unsigned int MaxEchoes = 0;
	for (Blocks_Type i = 0; i < aCIR; i++) {
		cir_struct act_cir = file->getCIR(i); // CIRs in file start with number 1
		/*
		 if (act_cir.ref_range > MaxPR) {
		 MaxPR = act_cir.ref_range;
		 }
		 */
		// read Echos
		for (size_t o = 0; o < act_cir.echoes.size(); o++) {
			if (act_cir.echoes.at(o).delay > MaxDelay) {
				MaxDelay = act_cir.echoes.at(o).delay;
			}
		}

		if (act_cir.echoes.size() > MaxEchoes) {
			MaxEchoes = act_cir.echoes.size();
		}
	}

	snLogDeb("snProcessorChannel: CIR file: max delay: " + QString::number(MaxDelay), 4);

	// find the sample where the sinc sidelobes decayed down to 1e-6:
	const double Om = 2.0 * M_PI * (interpolation_bandwidth);
	double fac = 0.0;
	add_delay = 0.0;
	double sinc_result = 0.0;
	int n = 0;
	do {
		add_delay = .5 * 1.0 / Om + n * 1.0 / Om;
		n++;

		fac = Om * add_delay;
		if (doubles_equal(fac, 0.0))
			sinc_result = 1.0;
		else
			sinc_result = fabs(sin(fac) / fac);

		std::cout << "fac: " << fac << ", abs(sin(fac) / fac): " << fabs(sin(fac) / fac)
				<< ", sinc_result: " << sinc_result << "\n";

	} while (sinc_result > 1e-4);

	std::cout << "add_delay: " << add_delay << " = " << add_delay * Sig.SmplFreq << " samples.\n";

	// add .2 at beginning and end to let sinc lobes decay sufficiently
	// TODO this should be dependent on max magnitude and interpolation frequency
	//aFIR = static_cast<int> (ceil(MaxDelay * 1.4 * Sig.SmplFreq));

	aFIR = static_cast<int> (ceil((MaxDelay + 2 * add_delay) * Sig.SmplFreq));

	std::cout << "afir: " << aFIR << "\n";
	snLogDeb("snProcessorChannel: amount FIR coeffs " + QString::number(aFIR) + " = "
			+ QString::number(aFIR * Sig.Ts) + " s", 4);

	//ProcSig.ProcessingDelay += .2 * static_cast<double> (aFIR) / Sig.SmplFreq; // processing delay caused by convolution, 0.2 * is added, see below DelayOffset
	ProcSig.ProcessingDelay += add_delay;
	//ProcSig.ProcessingDelay += static_cast<double> (aFIR) / 2.0 / Sig.SmplFreq; // processing delay caused by filter

	// check if CIRrate is equal to Sig.BufSize:
	if (static_cast<Samples_Type> ((1.0 / CIRrate) * Sig.SmplFreq) != Sig.BufSize) {
		throw snSimulationException(QString("snChannel: CIR rate (" + QString::number(CIRrate)
				+ "Hz = " + QString::number(((1.0 / CIRrate) * Sig.SmplFreq))
				+ " samples) does not match ConcBuf size (" + QString::number(Sig.BufSize)
				+ ")! Adjust ConcBuf size to be same length as 1 / cir rate.").toStdString());
	}

	// 1st snWidget for model CIR input:
	cir_curve = new snCurveComplex(MaxEchoes, Qt::blue, Qt::red);
	cir_curve->setCurveSymbol(snCurve::CURVEI, QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::blue),
			QPen(Qt::blue), QSize(5, 5)));
	cir_curve->setCurveStyle(snCurve::CURVEI, QwtPlotCurve::Sticks);

	cir_curve->setCurveSymbol(snCurve::CURVEQ, QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::red), QPen(
			Qt::red), QSize(5, 5)));
	cir_curve->setCurveStyle(snCurve::CURVEQ, QwtPlotCurve::Sticks);

	snPlotComplexMagn *cir_plot_magn = new snPlotComplexMagn("magnitude", snPlot::Log20, cir_curve);
	cir_plot_magn->setYAxis(-50, 5);
	cir_plot_magn->setXAxis(0, aFIR * (Sig.Ts));
	snPlotComplexAngle *cir_plot_angl = new snPlotComplexAngle("angle", snPlot::Linear, cir_curve);
	cir_plot_angl->setYAxis(-3.5, 3.5);
	cir_plot_angl->setXAxis(0, aFIR * (Sig.Ts));

	cir_snw->addPlot(cir_plot_magn);
	cir_snw->addPlot(cir_plot_angl);

	// 2nd snWidget for interpolated FIR coefficients:
	fir_curve = new snCurveComplex(aFIR, Qt::blue, Qt::red);
	fir_curve->setCurveSymbol(snCurve::CURVEI, QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::blue),
			QPen(Qt::blue), QSize(4, 4)));
	fir_curve->setCurveStyle(snCurve::CURVEI, QwtPlotCurve::Lines);

	fir_curve->setCurveSymbol(snCurve::CURVEQ, QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::red), QPen(
			Qt::red), QSize(4, 4)));
	fir_curve->setCurveStyle(snCurve::CURVEQ, QwtPlotCurve::Lines);

	snPlotComplexMagn *fir_plot_magn = new snPlotComplexMagn("magnitude", snPlot::Log20, fir_curve);
	fir_plot_magn->setXAxis(0, aFIR * Sig.Ts);
	fir_plot_magn->setYAxis(-50, 5);
	snPlotComplexAngle *fir_plot_angl = new snPlotComplexAngle("angle", snPlot::Linear, fir_curve);
	fir_plot_angl->setXAxis(0, aFIR * Sig.Ts);
	fir_plot_angl->setYAxis(-3.5, 3.5);
	snPlotMagn *plot_magn = new snPlotMagn("magnitude", snPlot::Log20, fir_curve, Sig.SmplFreq);
	plot_magn->setYAxis(-50, 5);
	snPlotAngl *plot_angl = new snPlotAngl("angle", snPlot::Linear, fir_curve, Sig.SmplFreq);
	plot_angl->setYAxis(-3.5, 3.5);

	fir_snw->addPlot(fir_plot_magn);
	fir_snw->addPlot(fir_plot_angl);
	fir_snw->addPlot(plot_magn);
	fir_snw->addPlot(plot_angl);

	// 3rd snWidget for model output of convolution:
	out_curve = new snCurveComplex(plot_x_max, Qt::blue, Qt::red);
	snPlotComplexRealImag *out_plot = new snPlotComplexRealImag("real / imag", snPlot::Linear,
			out_curve);
	snPlotMagn *out_plot_magn = new snPlotMagn("magnitude", snPlot::Log20, out_curve, Sig.SmplFreq);
	snPlotAngl *out_plot_angl = new snPlotAngl("angle", snPlot::Linear, out_curve, Sig.SmplFreq);
	out_plot->setXAxis(0.0, Sig.plot_buffer_length);
	out_plot->setYAxis(-8, 8);
	out_plot->setXAxisLabel("t [s]");
	out_plot->setYAxisLabel("A");

	out_snw->addPlot(out_plot);
	out_snw->addPlot(out_plot_magn);
	out_snw->addPlot(out_plot_angl);

	snLog("snChannel: channel cir interpolation setup complete.");
}

snProcessorChannel::~snProcessorChannel() {
	delete file;
}

void snProcessorChannel::run() {
	Samples_Type nToRead;
	std::complex<double> *BufIn, *BufOut;

	//double PRtotal = 0;
	//double DelayOffset = 0.2 * static_cast<double> (aFIR) / Sig.SmplFreq;
	double DelayOffset = add_delay;

	// setup FIR filter:
	snCmplxFIRFilter CmplxFilter1(Sig.BufSize, aFIR);

	std::vector<std::complex<double> > Coeffs(aFIR);

	// read data from buf:
	Blocks_Type curNumber = 0;
	while (isActive()) {
		// get next CIR:
		cir_struct act_cir = file->getCIR(curNumber);

		// adjust delays of echoes according to actual pseudorange:
		//PRtotal += act_cir.ref_range; // += for PRchange
		snLogDeb("snInterpolator: actual PR: " + QString::number(act_cir.ref_range)
				+ ", actPRchange/c0: " + QString::number(act_cir.ref_range / Sig.c0), 4);

		for (size_t o = 0; o < act_cir.echoes.size(); o++) // for all echos
		{
			//			act_cir.echoes.at(o).delay += DelayOffset + act_cir.ref_range
			//					/ Sig.c0;
			act_cir.echoes.at(o).delay += DelayOffset;
		}

		// interpolate the read CIR coefficients to FIR coefficients
		for (int o = 0; o < aFIR; o++) {
			Coeffs.at(o) = std::complex<double>(0, 0);
		}

		if (interpolation_type == SINC) {
			// interpolate FIR coefficients with sinc interpolation. result to BufFIR:
			const double Om = 2.0 * M_PI * (interpolation_bandwidth);
			for (size_t o = 0; o < act_cir.echoes.size(); o++) // for all echos
			{
				for (int k = 0; k < aFIR; k++) // for all time steps
				{
					const double fac = Om * (k * Sig.Ts - act_cir.echoes.at(o).delay);
					if (fac == 0) // sin(0)/0 = 1;
					{
						Coeffs.at(k) += act_cir.echoes.at(o).weight * 1.0;
					} else {
						Coeffs.at(k) += act_cir.echoes.at(o).weight * (sin(fac) / fac);
					}
				}
			}
		} else if (interpolation_type == RRC) {
			for (size_t o = 0; o < act_cir.echoes.size(); o++) // for all echos
			{
				//				std::cout << "act_cir.echoes.at(o).weight: "
				//						<< abs(act_cir.echoes.at(o).weight) << "\n";
				for (int k = 0; k < aFIR; k++) // for all time steps
				{
					const double t = k * Sig.Ts - act_cir.echoes.at(o).delay;
					Coeffs.at(k) += act_cir.echoes.at(o).weight * (sin(M_PI * t
							* interpolation_bandwidth) / (M_PI * t * interpolation_bandwidth))
							* cos(M_PI * rrc_rolloff_factor * t * interpolation_bandwidth) / (1
							- (4 * pow(rrc_rolloff_factor, 2) * t * t * pow(
									interpolation_bandwidth, 2)));
				}
			}
		}
		// interpolation of FIR coefficients complete
		/*
		 for (int o = 0; o < aFIR; o++) {
		 Coeffs.at(o) = std::complex<double>(0, 0);
		 }
		 Coeffs.at(7) = 1.0;
		 */
		BufIn = ConcBuf1->RequestReading(&nToRead);
		BufOut = ConcBuf2->RequestWriting();
		if (nToRead == Sig.BufSize) {
			// do the convolution:
			CmplxFilter1.filter(BufIn, BufOut, Coeffs);
		} else if (nToRead == 0) {
			//do nothing anymore
		} else { // we cannot handle this until now
			std::cout << "\n snProcessorChannel: nToRead != Sig.BufSize";
			std::cout << "\n snProcessorChannel: nToRead: " << nToRead << ", Sig.BufSize: "
					<< Sig.BufSize;
			std::exit(1);
		}

		if (ConcBuf1->ReleaseReading()) {
			ConcBuf2->ReleaseWriting(nToRead, 1);
			snLog("snChannel: exit.");
			printSummary("snProcessorChannel");
			return;
		} else {
			// should we plot?
			if (out_snw->requestPlotFlag()) {
				for (Samples_Type o = 0; o < plot_x_max; o++) {
					out_curve->X[o] = static_cast<double> (o) / Sig.SmplFreq; // in s
					out_curve->Y[o] = BufOut[o];
				}
				out_snw->releasePlotFlag();
			}

			ConcBuf2->ReleaseWriting(nToRead, 0);

			// should we plot?
			if (cir_snw->requestPlotFlag()) {
				for (size_t o = 0; o < act_cir.echoes.size(); o++) {
					cir_curve->X[o] = act_cir.echoes.at(o).delay;
					cir_curve->Y[o] = act_cir.echoes.at(o).weight;
				}
				cir_curve->setPointsToPlot(snCurve::CURVEI, act_cir.echoes.size());
				cir_curve->setPointsToPlot(snCurve::CURVEQ, act_cir.echoes.size());
				cir_snw->releasePlotFlag();
			}
			if (fir_snw->requestPlotFlag()) {
				for (int o = 0; o < aFIR; o++) {
					fir_curve->X[o] = o * Sig.Ts;
					fir_curve->Y[o] = Coeffs.at(o);
				}
				fir_snw->releasePlotFlag();
			}

		}
		curNumber++;
		increaseBlks();
	}
}
