#ifndef __INCLUDED_KARMA_PROGRAM_H
#define __INCLUDED_KARMA_PROGRAM_H

#include "ADSR.h"
#include "LFO.h"

class Program {
public:
	float waveform1;
	float waveform2;
	ADSR freq1;
	ADSR freq2;

	float waveformMix;

	LFO lfo1;
	LFO lfo2;
//	ADSR volume1;
//	ADSR volume2;


	float filter;
	float resonance;
	float cut;
	float adsrAmount;
//	ADSR filterRes;
	ADSR filterCut;

	float distortion;

	ADSR amplifier;
	float gain;
};


#endif