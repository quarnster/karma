#include "ADSR.h"

ADSR::ADSR() {
	attack = decay = sustain = release = 0;
}

int ADSR::getValue(long samplePos, long relSample) {
	long samplePos2 = samplePos;
	int ret;

	if (relSample) // ADSR should not advance when released
		samplePos = relSample;

	if (samplePos < attack) {
		// attack
		ret =  (samplePos << 10) / attack;
	} else if (samplePos < attack + decay) {
		// decay
		samplePos -= attack;
		samplePos <<= 10;


		ret = 1024 - (((samplePos / decay) * (1024 - sustain)) >> 10);
	} else {
		// sustain
		ret = sustain;
	}
	if (relSample) {
		// release
		samplePos2 -= relSample;
		if (samplePos2 >= release)
			return 0;

		samplePos2 <<= 10;
		return ((1024 - (samplePos2 / release)) * ret) >> 10;
	}

	return ret;
}
