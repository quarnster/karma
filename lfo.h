#ifndef __INCLUDED_KARMA_LFO_H
#define __INCLUDED_KARMA_LFO_H

#include "waveform.h"

typedef struct {
	int waveform;
	WAVEFORM_CALLBACK waveformFunction;

	float rate;
	int amount;

	int phase;
} karma_LFO;


#endif