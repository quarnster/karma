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
		freq1.sustain = 0.5;
		amplifier.sustain = 0.5;
		amplifier.release = 0.001;
		gain = 0.5;
		channel = 1;
		waveformMix = 0;
	}
};


#endif