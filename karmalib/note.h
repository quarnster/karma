/*
 * Karma softsynth
 * 
 * $Id: note.h,v 1.3 2003/12/30 16:09:09 quarn Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
#ifndef __INCLUDED_KARMA_NOTE_H
#define __INCLUDED_KARMA_NOTE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
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
