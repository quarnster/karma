#ifndef __INCLUDED_KARMA_ADSR_H
#define __INCLUDED_KARMA_ADSR_H

class ADSR {
public:
	int attack;
	int decay;
	int sustain;
	int release;

	ADSR();

	int getValue(long samplepos, long relSample);
};


#endif