/*
 * Karma softsynth
 * 
 * $Id: karma.h,v 1.3 2003/12/30 16:09:09 quarn Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
#ifndef __INCLUDED_KARMA_H
#define __INCLUDED_KARMA_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _karma_MidiEvent {
	struct _karma_MidiEvent *next;
	long time;
	unsigned char data[3];
} karma_MidiEvent;

typedef struct {
	karma_MidiEvent *event;
	karma_MidiEvent *currentEvent;

	long samplesPlayed;
} karma_Song;

void karma_init();
void karma_process(karma_Song *song, int *left, int *right, int samples);

karma_Song *karma_loadSong(unsigned char *buffer);
void karma_freeSong(karma_Song *song);

void karma_free();

#ifdef __cplusplus
}
#endif
#endif

