#ifndef __INCLUDED_KARMA_NOTE_H
#define __INCLUDED_KARMA_NOTE_H

typedef struct {
	int phase1;
	int phase2;

	long currentNote;
	int noteFreq;

	bool released;
	bool active;

	long samplesPlayed;
	long relSample;

	long delta;

	float high;
	float low;
	float band;
	float notch;

	int volumeUpdateRate;
	int waveformUpdateRate;
	int lfo1UpdateRate;
	int lfo2UpdateRate;
	int freq2UpdateRate;
} karma_Note;

#endif