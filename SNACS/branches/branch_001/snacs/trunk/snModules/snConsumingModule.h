/*
 * snConsumingModule.h
 *
 *  Created on: Aug 17, 2010
 *      Author: fschubert
 */

#ifndef SNCONSUMINGMODULE_H_
#define SNCONSUMINGMODULE_H_

#include <QReadWriteLock>

#include "snModule.h"

/*
 * This module is a base class for modules that only consume samples, for example the signal output widget.
 */
class snConsumingModule: public snModule {
public:
	snConsumingModule(SNSignal pSNSignal, ConcBuf3* _circbuf);
	virtual ~snConsumingModule();

	ConcBuf3* get_conc_buf() {
		std::cerr
				<< "error: conc_buf cannot be returned: a snConsumingModule does not create a conc_buf.";
		std::exit(1);
	}

	/*
	 * Used by snMainWindow to update the simulation progress bar.
	 */
	double get_current_simulation_time() {
		QReadLocker _locker(&m_curr_simtime);
		const double v = curr_simtime;
		return v;
	}

protected:
	/*
	 * The current simulation time has to be set but sink modules. It is used
	 * by snMainWindow to update the simulation progress bar.
	 */
	void set_current_simulation_time(double _v) {
		QWriteLocker _locker(&m_curr_simtime);
		curr_simtime = _v;
	}

	unsigned int get_nof_xaxis_indices() {
		return Sig.plot_buffer_length * smpl_freq;
	}

	double get_max_xaxis_value() {
		return Sig.plot_buffer_length;
	}

	Freq_Type smpl_freq; ///< sampling frequency of the circbuf with incoming data
	double signal_length; ///< total signal length in s
	double block_length; ///< length of one block in s
	unsigned int nof_blocks_per_concbuf; ///< number of blocks per circbuf
	Samples_Type block_size; ///< length of one block in samples
	Blocks_Type total_blocks; ///< total number of blocks to be processed
	ConcBuf3 *circbuf;
private:
	double curr_simtime; ///< current simulation time
	QReadWriteLock m_curr_simtime;///< mutex to protect current simulation time
};

#endif /* SNCONSUMINGMODULE_H_ */
