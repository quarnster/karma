#ifndef __INCLUDED_KARMA_LFO_H
#define __INCLUDED_KARMA_LFO_H

typedef struct {
	int waveform;
	float rate;
	int amount;

	int phase;
} karma_LFO;


#endif