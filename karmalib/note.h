#ifndef __INCLUDED_KARMA_NOTE_H
#define __INCLUDED_KARMA_NOTE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef __declspec(align(32)) struct {
	int phase1;
	int phase2;

	long currentNote;
	int noteFreq;

	char released;
	char active;

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

#ifdef __cplusplus
}
#endif
#endif