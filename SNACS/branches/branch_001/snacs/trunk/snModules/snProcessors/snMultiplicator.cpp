/*
 * snProcessorMultiplicator.cpp
 *
 *  Created on: May 11, 2011
 *      Author: frank
 */

#include "snMultiplicator.h"

#include <iostream>
#include <fstream>

snMultiplicator::snMultiplicator(libconfig::Setting &Settings, SNSignal pSNSignal, snWidget *_snw,
		ConcBuf3* _circbuf_in) :
	snCreatingModule(pSNSignal, "snChannel", _circbuf_in), snw(_snw), circbuf_in(_circbuf_in) {

	snLog("snMultiplicator: channel cir interpolation setup.");

	Sig.results_file->createGroup("snMultiplicator");
	Sig.results_file->createGroup("snMultiplicator/parameters");

	std::string filename = static_cast<const char *> (Settings["file"]);
	Sig.results_file->write("snMultiplicator/parameters/filename", filename);

	start_sec = Settings["start_sec"];
	Sig.results_file->write("snMultiplicator/parameters/start_sec", filename);

	std::ifstream f(filename.c_str(), std::ios::in);
	if (!f.is_open()) {
		throw snSimulationException("snMultiplicator: cannot open file " + filename + ".");
	}

	// first number in file is frequency:
	f >> mult_data_freq;

	if ((1.0 / block_length) != mult_data_freq) {
		throw snSimulationException(
				"snMultiplicator: block_length must be same length as (multiplication data frequency)^(-1).");
	}
	Sig.results_file->write("snMultiplicator/parameters/multiplication_data_frequency", mult_data_freq);

	mult_data = new itpp::cvec(total_blocks);
	Blocks_Type value_index = 0;
	while (!f.eof()) {
		std::complex<double> value;
		f >> value;
		//std::cout << "read: " << value << "\n";
		mult_data->set(value_index, value);
		value_index++;
		if (value_index == total_blocks)
			break;
	}

	if (value_index < total_blocks) {
		throw snSimulationException("snMultiplicator: too few data values in file.");
	}

	f.close();
	//std::vector<double> a = *mult_data;
	Sig.results_file->write_cvec("snMultiplicator/parameters/multiplication_data", *mult_data);

	//std::cout << "\ntotal blocks: " << total_blocks << "\n";

	// setup widgets:
	out_curve = new snCurveComplex(get_nof_xaxis_indices(), snCurve::blue, snCurve::red);
	snPlot *out_plot = new snPlot("Signal out, real / imag", snPlot::Linear,
			out_curve);
	snPlotMagn *out_plot_magn = new snPlotMagn("magnitude", snPlot::Log20, out_curve, smpl_freq);
	out_plot->setXAxis(0.0, get_max_xaxis_value());
	out_plot->setYAxis(-2, 2);
	out_plot->setXAxisLabel("t [s]");
	out_plot->setYAxisLabel("A");

	snw->addPlot(out_plot);
	snw->addPlot(out_plot_magn);

	std::cout << "block_length: " << block_length << "\n";
	mult_curve = new snCurveComplex(total_blocks, snCurve::blue, snCurve::red);
	mult_curve_dB = new snCurveReal(total_blocks, snCurve::blue);
	mult_curve_angl = new snCurveReal(total_blocks, snCurve::blue);
	for (size_t k = 0; k < total_blocks; k++) {
		mult_curve->update_value_X(k, k * block_length);
		mult_curve->update_value_Y(k, 0);
		mult_curve_dB->update_value_X(k, k * block_length);
		mult_curve_dB->update_value_Y(k, 0);
		mult_curve_angl->update_value_X(k, k * block_length);
		mult_curve_angl->update_value_Y(k, 0);
	}
	snPlot *mult_plot = new snPlot("Mult. data, real/imag", snPlot::Linear,
			mult_curve);
	snPlot *mult_plot_magn = new snPlot("Mult. data", snPlot::Linear, mult_curve_dB);
	snPlot *mult_plot_angl = new snPlot("Mult. data angle", snPlot::Linear, mult_curve_angl);

	mult_plot->enableGrid();
	mult_plot->setXAxis(0.0, signal_length);
	mult_plot->setXAxisLabel("t [s]");
	mult_plot->setYAxisLabel("A");
	mult_plot_magn->enableGrid();
	mult_plot_magn->setXAxis(0.0, signal_length);
	mult_plot_magn->setXAxisLabel("t [s]");
	mult_plot_magn->setYAxisLabel("A [dB]");
	mult_plot_angl->enableGrid();
	mult_plot_angl->setXAxis(0.0, signal_length);
	mult_plot_angl->setXAxisLabel("t [s]");
	mult_plot_angl->setYAxisLabel("Angle");
	snw->addPlot(mult_plot);
	snw->addPlot(mult_plot_magn);
	snw->addPlot(mult_plot_angl);

	snLog("snMultiplicator: channel cir interpolation setup complete.");
}

snMultiplicator::~snMultiplicator() {
	delete mult_data;
}

void snMultiplicator::run() {

	Blocks_Type value_index = 0;
	while (true) {
		itpp::cvec & BufIn = circbuf_in->RequestReading();
		itpp::cvec & BufOut = conc_buf->RequestWriting();

		if (circbuf_in->get_current_consumer_block() * block_length < start_sec) {
			mult_data->set(value_index, 1.0);
		}

		BufOut = mult_data->get(value_index) * BufIn;

		if (circbuf_in->ReleaseReading()) {
			conc_buf->ReleaseWriting(1);
			snLog("snMultiplicator: exit.");
			printSummary("snMultiplicator");
			return;
		} else {
			// should we plot?
			if (snw->requestPlotFlag()) {
				for (Samples_Type o = 0; o < get_nof_xaxis_indices(); o++) {
					out_curve->update_value_X(o, static_cast<double> (o) / smpl_freq); // in s
					out_curve->update_value_Y(o, BufOut[o]);
				}
				snw->releasePlotFlag();
			}
			conc_buf->ReleaseWriting(0);
		}

		snw->pushData();
		mult_curve->update_value_X(value_index, value_index * block_length);
		mult_curve->update_value_Y(value_index, mult_data->get(value_index));
		mult_curve->setPointsToPlot(value_index);

		mult_curve_dB->update_value_X(value_index, value_index * block_length);
		mult_curve_dB->update_value_Y(value_index, 20 * log10(abs(mult_data->get(value_index))));
		mult_curve_dB->setPointsToPlot(value_index);

		mult_curve_angl->update_value_X(value_index, value_index * block_length);
		mult_curve_angl->update_value_Y(value_index, arg(mult_data->get(value_index)));
		mult_curve_angl->setPointsToPlot(value_index);

		snw->releasePlotFlag();

		value_index++;
	}
}
