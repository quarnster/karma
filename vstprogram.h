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
		volume1.sustain = 0.5;
		volume1.release = 0.001;
		volume2.release = 0.001;
		volume = 1.0;
		channel = 1;
	}
};


#endif