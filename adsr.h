#ifndef __INCLUDED_KARMA_ADSR_H
#define __INCLUDED_KARMA_ADSR_H

class ADSR {
private:
//	bool released;
//	long relSample;

public:
	float attack;
	float decay;
	float sustain;
	float release;

	ADSR();
//	void triggerKey();
//	void releaseKey();

	float getValue(long samplepos, long relSample);
};


#endif