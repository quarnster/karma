#include "ADSR.h"

ADSR::ADSR() {
	attack = decay = sustain = release = 0;
}

int ADSR::getValue(long samplePos, long relSample) {
/*
	long alength = (long) (attack * 44100*2);
	long dlength = (long) (decay * 44100*2);
	long rlength = (long) (release * 44100*2);
*/
	if (relSample == -1) {
		if (samplePos < attack) {
			// attack
			return (samplePos << 10) / attack;
		} else if (samplePos < attack + decay) {
			// decay
			samplePos -= attack;
			samplePos <<= 10;


			return 1024 - (((samplePos / decay) * (1024 - sustain)) >> 10);
		} else {
			// sustain
			return sustain;
		}
	} else {
		// release
		samplePos -= relSample;
		if (samplePos >= release)
			return 0;

		samplePos <<= 10;
		return ((1024 - (samplePos / release)) * sustain) >> 10;
	}


	return sustain;
}
