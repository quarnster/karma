#ifndef __INCLUDED_KARMA_CHANNEL_H
#define __INCLUDED_KARMA_CHANNEL_H

#include "Program.h"

class Channel {
protected:
	Program *program;
	float high;
	float low;
	float band;
	float notch;

	bool active;
	long relSample;

	// ----------------------------
	float fPhase1, fPhase2;
//	float fScaler;

	long samplesPlayed;

	long currentVelocity;
	long currentDelta;

	float getSample(float wave, float phase);

public:
	bool released;
	long currentNote;

	Channel();
	void process(float *out, long sampleFrames);
	void noteOn(long note, long velocity, long delta);
	void noteOff();
};


#endif