#ifndef __INCLUDED_KARMA_VSTPROGRAM_H
#define __INCLUDED_KARMA_VSTPROGRAM_H

#include "Program.h"

class VstProgram : public Program {
public:
	long channel;
	float fChannel;
	char name[32];

	VstProgram() {
//		name = "bice\0";
		freq1 = 0;
		freq2 = 0;
		amplifier.sustain = 0.5;
		amplifier.release = 0.5;
		amplifier.attack = 0.5;
		amplifier.decay = 0.5;
		gain = 0.5;
		channel = 1;
		waveformMix = 0.5;
	}
};


#endif