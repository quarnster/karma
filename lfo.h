#ifndef __INCLUDED_KARMA_LFO_H
#define __INCLUDED_KARMA_LFO_H

class LFO {
public:
	float waveform;
	float rate;
	float amount;

	float fPhase;

	LFO() {
		waveform = 0;
		rate = 0;
		amount = 0;
		fPhase = 0;
	}

};


#endif