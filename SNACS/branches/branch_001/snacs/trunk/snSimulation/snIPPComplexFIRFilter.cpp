/*
 * snIPPComplexFIRFilter.cpp
 *
 *  Created on: Aug 20, 2010
 *      Author: fschubert
 */

#ifdef HAVE_IPP

#include "snIPPComplexFIRFilter.h"

snIPPRealFIRFilter::snIPPRealFIRFilter(int _datalen, int _numtaps):datalen(_datalen), numtaps(_numtaps) {

	/* determine state size
	 IppStatus ippsFIRGetStateSize_64fc(int tapsLen, int* pBufferSize);

	 Return Values
	 ippStsNoErr
	 Indicates no error.
	 ippStsNullPtrErr
	 Indicates an error if the pBufferSize pointer is NULL.
	 ippStsFIRLenErr
	 Indicates an error if nxTapsLen is less than or equal to 0.
	 ippStsFIRMRFactorErr
	 Indicates an error if upFactor (downFactor) is less than or equal to 0.
	 */

	x = ippsMalloc_64f(datalen);
	y = ippsMalloc_64f(datalen);

	taps = ippsMalloc_64f(numtaps);

	/* initialize IPP FIR function
	 IppStatus ippsFIRInitAlloc_64fc(IppsFIRState_64fc** ppState, const Ipp64fc* pTaps, int tapsLen, const Ipp64fc* pDlyLine);
	 Parameters

	 pTaps
	 Pointer to the array containing the tap values. The number of elements in the array is tapsLen.

	 tapsLen
	 Number of elements in the array containing the tap values.

	 tapsFactor
	 Scale factor for the taps of Ipp32s data type (for integer versions only).

	 pDlyLine
	 Pointer to the array containing the delay line values. The number of elements in the array is tapsLen.

	 ppState
	 Pointer to the pointer to the FIR state structure to be created.
	 */
	status = ippsFIRInitAlloc_64f(&state,taps,numtaps,NULL);
}

void snIPPRealFIRFilter::set_coeffs(itpp::vec& H) {

	if (H.size() != numtaps) {
		std::cerr<<"error: snIPPRealFIRFilter::set_state: H.size() != numtaps\n";
		std::exit(1);
	}

	Ipp64f* taps = reinterpret_cast<Ipp64f*>(&H[0]);

	// IppStatus ippsFIRSetTaps_64fc(const Ipp64fc* pTaps, IppsFIRState_64fc* pState);
	ippsFIRSetTaps_64f(taps, state);
}

void snIPPRealFIRFilter::filter(itpp::vec &X, itpp::vec &Y) {

	if (X.size() != datalen) {
		std::cerr<<"error: snIPPRealFIRFilter::set_state: X.size() != datalen\n";
		std::exit(1);
	}
	if (Y.size() != datalen) {
		std::cerr<<"error: snIPPRealFIRFilter::set_state: Y.size() != datalen\n";
		std::exit(1);
	}

	x = reinterpret_cast<Ipp64f*>(&X[0]);
	y = reinterpret_cast<Ipp64f*>(&Y[0]);

	/*
	 IppStatus ippsFIR_64fc(const Ipp64fc* pSrc, Ipp64fc* pDst, int numIters, IppsFIRState_64fc* pState);
	 */
	status = ippsFIR_64f( x, y, datalen, state );
}

snIPPRealFIRFilter::~snIPPRealFIRFilter() {
	ippsFIRFree_64f(state);

	ippsFree(x);
	ippsFree(y);
}

snIPPComplexFIRFilter::snIPPComplexFIRFilter(int _datalen, int _numtaps):datalen(_datalen), numtaps(_numtaps) {

	/* determine state size
	 IppStatus ippsFIRGetStateSize_64fc(int tapsLen, int* pBufferSize);

	 Return Values
	 ippStsNoErr
	 Indicates no error.
	 ippStsNullPtrErr
	 Indicates an error if the pBufferSize pointer is NULL.
	 ippStsFIRLenErr
	 Indicates an error if nxTapsLen is less than or equal to 0.
	 ippStsFIRMRFactorErr
	 Indicates an error if upFactor (downFactor) is less than or equal to 0.
	 */

	x = ippsMalloc_64fc(datalen);
	y = ippsMalloc_64fc(datalen);

	taps = ippsMalloc_64fc(numtaps);

	/* initialize IPP FIR function
	 IppStatus ippsFIRInitAlloc_64fc(IppsFIRState_64fc** ppState, const Ipp64fc* pTaps, int tapsLen, const Ipp64fc* pDlyLine);
	 Parameters

	 pTaps
	 Pointer to the array containing the tap values. The number of elements in the array is tapsLen.

	 tapsLen
	 Number of elements in the array containing the tap values.

	 tapsFactor
	 Scale factor for the taps of Ipp32s data type (for integer versions only).

	 pDlyLine
	 Pointer to the array containing the delay line values. The number of elements in the array is tapsLen.

	 ppState
	 Pointer to the pointer to the FIR state structure to be created.
	 */
	status = ippsFIRInitAlloc_64fc(&state,taps,numtaps,NULL);
}

void snIPPComplexFIRFilter::set_coeffs(itpp::cvec& H) {

	if (H.size() != numtaps) {
		std::cerr<<"error: snIPPComplexFIRFilter::set_state: H.size() != numtaps\n";
		std::exit(1);
	}

	Ipp64fc* taps = reinterpret_cast<Ipp64fc*>(&H[0]);

	// IppStatus ippsFIRSetTaps_64fc(const Ipp64fc* pTaps, IppsFIRState_64fc* pState);
	ippsFIRSetTaps_64fc(taps, state);
}

void snIPPComplexFIRFilter::filter(itpp::cvec &X, itpp::cvec &Y) {

	if (X.size() != datalen) {
		std::cerr<<"error: snIPPComplexFIRFilter::set_state: X.size() != datalen\n";
		std::exit(1);
	}
	if (Y.size() != datalen) {
		std::cerr<<"error: snIPPComplexFIRFilter::set_state: Y.size() != datalen\n";
		std::exit(1);
	}

	x = reinterpret_cast<Ipp64fc*>(&X[0]);
	y = reinterpret_cast<Ipp64fc*>(&Y[0]);

	/*
	 IppStatus ippsFIR_64fc(const Ipp64fc* pSrc, Ipp64fc* pDst, int numIters, IppsFIRState_64fc* pState);
	 */
	status = ippsFIR_64fc( x, y, datalen, state );
}

snIPPComplexFIRFilter::~snIPPComplexFIRFilter() {
	ippsFIRFree_64fc(state);

	ippsFree(x);
	ippsFree(y);
}

#endif
