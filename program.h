#ifndef __INCLUDED_KARMA_PROGRAM_H
#define __INCLUDED_KARMA_PROGRAM_H

#include "ADSR.h"
#include "LFO.h"

//typedef ((int)
#ifdef __GNUC__
#define __declspec(x) __attribute__((x))
#endif
typedef __declspec(align(32)) struct {
	int waveform1;
	int waveform2;

	int *waveform1Table;
	int *waveform2Table;

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