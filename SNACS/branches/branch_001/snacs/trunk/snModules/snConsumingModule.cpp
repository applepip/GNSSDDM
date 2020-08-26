/*
 * snConsumingModule.cpp
 *
 *  Created on: Aug 17, 2010
 *      Author: fschubert
 */

#include "snConsumingModule.h"

snConsumingModule::snConsumingModule(SNSignal pSNSignal, ConcBuf3* _circbuf) :
	snModule(pSNSignal), circbuf(_circbuf) {

	curr_simtime = 0;

	nof_blocks_per_concbuf = circbuf->get_nof_blocks_per_buffer();
	block_length = circbuf->get_block_length();
	smpl_freq = circbuf->get_smpl_freq();
	signal_length = circbuf->get_signal_length();
	block_size = circbuf->get_block_size();
	total_blocks = circbuf->get_total_blocks();
}

snConsumingModule::~snConsumingModule() {
	// TODO Auto-generated destructor stub
}
