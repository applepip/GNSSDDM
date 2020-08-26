/*
 * snCodeE1B.h
 *
 *  Created on: Mar 10, 2010
 *      Author: frank
 */

#ifndef SNCODEE1B_H_
#define SNCODEE1B_H_

#include <cstdlib>

#include "snGNSSCode.h"

class snCodeGalileo: public snGNSSCode {
public:
	enum code_type {
		E1B, E1C, E5aI, E5aQ, E5bI, E5bQ
	};

	snCodeGalileo(code_type ct, int _prn);
	virtual ~snCodeGalileo();
	void initialize_E1_codes();
	void initialize_E5_codes();
	std::vector<double> generate_code(int p);

private:

	std::vector<std::string> strCodesE1B, strCodesE1C;
	std::vector<std::string> strCodesE5aI, strCodesE5aQ, strCodesE5bI, strCodesE5bQ;

	/**
	 * \brief generates one E1B pseudorandom code sequence
	 */
	std::vector<double> generate_code(code_type ct, size_t prn);

};

#endif /* SNCODEE1B_H_ */
