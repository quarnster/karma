#ifndef __INCLUDED_KARMA_NOTE_H
#define __INCLUDED_KARMA_NOTE_H

typedef struct {
	float fPhase1;
	float fPhase2;

	unsigned char currentNote;
//	int currentVelocity = velocity;
//	currentDelta = delta;
	bool released;
	bool active;

	long samplesPlayed;
	long relSample;

	long delta;

} karma_Note;

#endif