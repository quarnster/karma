#ifndef __INCLUDED_KARMA_CHANNEL_H
#define __INCLUDED_KARMA_CHANNEL_H

#include "Program.h"
#include "note.h"

#define MAX_NOTES 10

class Channel {
protected:
	Program *program;
	bool active;
	int playing_notes;

private:
	int panl;
	int panr;

	int *leftEcho;
	int *rightEcho;
	int echoPos;
	int echoSamples;
//	long relSample;

	// ----------------------------
//	float fPhase1, fPhase2;

//	long samplesPlayed;

	long currentVelocity;
//	long currentDelta;


public:
	karma_Note note[MAX_NOTES];
//	bool released;
//	long currentNote;

	Channel();
	~Channel();

	void process(int *left, int *right, long sampleFrames);
	void noteOn(long note, long velocity, long delta);
	void noteOff(long note);

	void setParameter(int cmd, int param);
};


#endif