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
		amplifier.sustain = 512;
		gain = 512;
		channel = 1;
		waveform1 = 0.476;
		waveform2 = 0.476;
		waveformMix = 0;
	}
};


#endif