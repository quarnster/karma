#ifndef __INCLUDED_KARMA_PROGRAM_H
#define __INCLUDED_KARMA_PROGRAM_H

#include "ADSR.h"
#include "LFO.h"

//typedef ((int) 
typedef struct {
	int waveform1;
	int waveform2;

	WAVEFORM_CALLBACK waveform1Function;
	WAVEFORM_CALLBACK waveform2Function;

	int wavelen1;
	int wavelen2;
//	ADSR freq1;
//	ADSR freq2;

	float freq1;
	float freq2;
	int waveformMix;

	karma_ADSR modEnv;
	float modEnvAmount;

	karma_LFO lfo1;
	karma_LFO lfo2;
//	ADSR volume1;
//	ADSR volume2;


	int filter;
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
} karma_Program;

#ifdef __cplusplus
extern "C" {
#endif

void karma_Program_init(karma_Program *program);

#ifdef __cplusplus
}
#endif

#endif