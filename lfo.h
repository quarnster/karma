#ifndef __INCLUDED_KARMA_LFO_H
#define __INCLUDED_KARMA_LFO_H

class LFO {
public:
	float waveform;
	float rate;
	int amount;

	int phase;

	LFO() {
		waveform = 0;
		rate = 0;
		amount = 0;
		phase = 0;
	}

};


#endif