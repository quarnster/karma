#ifndef __INCLUDED_KARMA_PROGRAM_H
#define __INCLUDED_KARMA_PROGRAM_H

#include "ADSR.h"
#include "LFO.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef __declspec(align(32)) struct {
	int waveform1;
	int waveform2;

	int *waveform1Table;
	int *waveform2Table;

	int wavelen1;
	int wavelen2;

	float freq1;
	float freq2;
	int waveformMix;

	karma_ADSR modEnv;
	float modEnvAmount;

	karma_LFO lfo1;
	karma_LFO lfo2;


	int filter;
	float resonance;
	float cut;
	float adsrAmount;
	karma_ADSR filterCut;

	int distortion;

	karma_ADSR amplifier;
	int gain;

	int echoDelay;
	int echoAmount;
} karma_Program;

void karma_Program_init(karma_Program *program);
void karma_Program_setParameter(karma_Program *program, long index, float value);
float karma_Program_getParameter(karma_Program *program, long index);


#ifdef __cplusplus
}
#endif

#endif