/*
 * snCreatingModule.cpp
 *
 *  Created on: Aug 17, 2010
 *      Author: fschubert
 */

#include "snCreatingModule.h"

snCreatingModule::snCreatingModule(SNSignal pSNSignal, std::string _name,
		Freq_Type _smpl_freq, double _signal_length, double _block_length,
		unsigned int _nof_blocks_per_concbuf) :
	snModule(pSNSignal), name(_name), smpl_freq(_smpl_freq), signal_length(
			_signal_length), block_length(_block_length),
			nof_blocks_per_concbuf(_nof_blocks_per_concbuf) {

	conc_buf = new ConcBuf3(nof_blocks_per_concbuf, block_length, smpl_freq,
			signal_length, name);
	block_size = conc_buf->get_block_size();
	total_blocks = conc_buf->get_total_blocks();

}

snCreatingModule::snCreatingModule(SNSignal pSNSignal, std::string _name,
		ConcBuf3* _circbuf) :
	snModule(pSNSignal), name(_name) {

	nof_blocks_per_concbuf = _circbuf->get_nof_blocks_per_buffer();
	block_length = _circbuf->get_block_length();
	smpl_freq = _circbuf->get_smpl_freq();
	smpl_ts = 1.0 / smpl_freq;
	signal_length = _circbuf->get_signal_length();
	block_size = _circbuf->get_block_size();
	total_blocks = _circbuf->get_total_blocks();

	conc_buf = new ConcBuf3(nof_blocks_per_concbuf, block_length, smpl_freq,
			signal_length, name);
}

snCreatingModule::~snCreatingModule() {
	delete conc_buf;
}
