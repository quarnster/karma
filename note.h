#ifndef __INCLUDED_KARMA_NOTE_H
#define __INCLUDED_KARMA_NOTE_H

typedef struct {
	int phase1;
	int phase2;

	long currentNote;
	bool released;
	bool active;

	long samplesPlayed;
	long relSample;

	long delta;

	float high;
	float low;
	float band;
	float notch;

} karma_Note;

#endif