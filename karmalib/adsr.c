#include "adsr.h"

int karma_ADSR_getValue(karma_ADSR *adsr, long samplePos, long relSample) {
	int ret;
	int samplePos2 = samplePos;

	if (relSample)
		samplePos = relSample;

	if (samplePos < adsr->attack) {
		// attack
		ret = (samplePos << 10) / adsr->attack;
	} else if (samplePos < adsr->attack + adsr->decay) {
		// decay
		samplePos -= adsr->attack;
		samplePos <<= 10;


		ret = 1024 - (((samplePos / adsr->decay) * (1024 - adsr->sustain)) >> 10);
	} else {
		// sustain
		ret = adsr->sustain;
	}

	if (relSample)  {
		// release
		samplePos2 -= relSample;
		if (samplePos2 >= adsr->release)
			return 0;

		
		ret = ((1024 - ((samplePos2 << 10) / adsr->release)) * ret) >> 10;
	}

	return ret;
}
