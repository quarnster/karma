#include "ADSR.h"

ADSR::ADSR() {
	attack = decay = sustain = release = 0;
}

float ADSR::getValue(long samplePos, long relSample) {
	long alength = (long) (attack * 44100*2);
	long dlength = (long) (decay * 44100*2);
	long rlength = (long) (release * 44100*2);

	if (relSample == -1) {
		if (samplePos < alength) {
			return (float) (samplePos / (float) alength);
		} else if (samplePos < alength + dlength) {
			samplePos -= alength;

			float percent = (float) (samplePos / (float) dlength);

			return 1 - (percent * (1 - sustain));
		} else {
			return sustain;
		}
	} else {
		samplePos -= relSample;
		if (samplePos > rlength)
			return 0;

		return (float) ((1 - (samplePos / (float) rlength)) * sustain);
	}


	return sustain;
}
