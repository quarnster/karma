#ifndef __INCLUDED_KARMA_PROGRAM_H
#define __INCLUDED_KARMA_PROGRAM_H

#include "ADSR.h"
#include "LFO.h"

class Program {
public:
	float waveform1;
	float waveform2;

	int wavelen1;
	int wavelen2;
//	ADSR freq1;
//	ADSR freq2;

	float freq1;
	float freq2;
	int waveformMix;

	ADSR modEnv;
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
	ADSR filterCut;

	int distortion;

	ADSR amplifier;
	int gain;

	int echoDelay;
	int echoAmount;
};


#endif