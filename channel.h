#ifndef __INCLUDED_KARMA_CHANNEL_H
#define __INCLUDED_KARMA_CHANNEL_H

#include "Program.h"
#include "note.h"

#define MAX_NOTES 10

class Channel {
protected:
	Program *program;

	bool active;

//	long relSample;

	// ----------------------------
//	float fPhase1, fPhase2;

//	long samplesPlayed;
	int playing_notes;

	long currentVelocity;
//	long currentDelta;


public:
	karma_Note note[MAX_NOTES];
//	bool released;
//	long currentNote;

	Channel();
	void process(int *out, long sampleFrames);
	void noteOn(long note, long velocity, long delta);
	void noteOff(long note);
};


#endif