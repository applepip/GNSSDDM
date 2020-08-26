/*
 * snCreatingModule.h
 *
 *  Created on: Aug 17, 2010
 *      Author: fschubert
 */

#ifndef SNCREATINGMODULE_H_
#define SNCREATINGMODULE_H_

#include "snModule.h"

/*
 * This module serves as base class for modules which create samples, i.e. source and processing modules
 */
class snCreatingModule: public snModule {
public:
	/*
	 * constructor that creates a circbuf with the given parameters for this CreatingModule
	 */
	snCreatingModule(SNSignal pSNSignal, std::string _name,
			Freq_Type _smpl_freq, double _signal_length, double _block_length,
			unsigned int _nof_blocks_per_concbuf);

	/*
	 * constructor that creates a circbuf with the parameters from the _circbuf for this CreatingModule
	 */
	snCreatingModule(SNSignal pSNSignal, std::string _name, ConcBuf3* _circbuf);

	virtual ~snCreatingModule();

	ConcBuf3* get_conc_buf() {
		return conc_buf;
	}

	double get_current_simulation_time() {
		std::cerr
				<< "error: get_current_simulation_time: This is a snCreatingModule, function not implemented.\n";
		std::exit(1);
	}

	double get_total_simulation_time() {
		std::cerr
				<< "error: get_total_simulation_time: This is a snCreatingModule, function not implemented.\n";
		std::exit(1);
	}

	unsigned int get_nof_xaxis_indices() {
		return Sig.plot_buffer_length * smpl_freq;
	}

	double get_max_xaxis_value() {
		return Sig.plot_buffer_length;
	}

protected:
	std::string name;
	Freq_Type smpl_freq; ///< sampling frequency of the circbuf with incoming data
	double smpl_ts; ///< length of one sample
	double signal_length; ///< total signal length in s
	double block_length; ///< length of one block in s
	unsigned int nof_blocks_per_concbuf; ///< number of blocks per circbuf
	Samples_Type block_size; ///< length of one block in samples
	ConcBuf3 *conc_buf;
	Samples_Type total_samples; ///< total number of samples
	Blocks_Type total_blocks; ///< total number of blocks
};

#endif /* SNCREATINGMODULE_H_ */
