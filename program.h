#ifndef __INCLUDED_KARMA_PROGRAM_H
#define __INCLUDED_KARMA_PROGRAM_H

#include "ADSR.h"
#include "LFO.h"

class Program {
public:
	int waveform1;
	int waveform2;

	int wavelen1;
	int wavelen2;
//	ADSR freq1;
//	ADSR freq2;

	float freq1;
	float freq2;
	int waveformMix;

	karma_ADSR modEnv;
	float modEnvAmount;

	LFO lfo1;
	LFO lfo2;
//	ADSR volume1;
//	ADSR volume2;


	float filter;
	float resonance;
	float cut;
	float adsrAmount;
//	ADSR filterRes;
	karma_ADSR filterCut;

	int distortion;

	karma_ADSR amplifier;
	int gain;

	int echoDelay;
	int echoAmount;
};


#endif