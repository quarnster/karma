#ifndef __INCLUDED_KARMA_CHANNEL_H
#define __INCLUDED_KARMA_CHANNEL_H

#include "Program.h"
#include "note.h"

#define MAX_NOTES 5

class Channel {
protected:
	Program *program;
	float high;
	float low;
	float band;
	float notch;

	bool active;

//	long relSample;

	// ----------------------------
//	float fPhase1, fPhase2;

//	long samplesPlayed;
	int playing_notes;

	long currentVelocity;
//	long currentDelta;

	float getSample(float wave, float phase);

public:
	karma_Note note[MAX_NOTES];
//	bool released;
//	long currentNote;

	Channel();
	void process(float *out, long sampleFrames);
	void noteOn(long note, long velocity, long delta);
	void noteOff(long note);
};


#endif