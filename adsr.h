#ifndef __INCLUDED_KARMA_ADSR_H
#define __INCLUDED_KARMA_ADSR_H

class ADSR {
public:
	float attack;
	float decay;
	float sustain;
	float release;

	ADSR();

	float getValue(long samplepos, long relSample);
};


#endif