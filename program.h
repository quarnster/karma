#ifndef __INCLUDED_KARMA_PROGRAM_H
#define __INCLUDED_KARMA_PROGRAM_H

#include "ADSR.h"

class Program {
public:
	float waveform1;
	float waveform2;
	ADSR freq1;
	ADSR freq2;

	ADSR volume1;
	ADSR volume2;

	float volume;

	float filter;
	ADSR filterRes;
	ADSR filterCut;

	float distortion;
};


#endif