/*
 * Karma softsynth
 * 
 * $Id: karma.c,v 1.5 2003/12/30 16:09:09 quarn Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
#include <stdlib.h>
#include <memory.h>


#include "karma.h"
#include "channel.h"
#include "param.h"

static karma_Channel channel[16];

/*-----------------------------------------------------------------------------------------
  karma_init - initialize the karma softsynth
-----------------------------------------------------------------------------------------*/
void karma_init() {
	int i;
	karma_Waveform_initTables();

	for (i = 0; i < 16; i++) {
		karma_Channel_init(&channel[i]);
	}
}

/*-----------------------------------------------------------------------------------------
  karma_process - process samples
-----------------------------------------------------------------------------------------*/
void karma_process(karma_Song *song, int *left, int *right, int samples) {
	int i;
	int pos = 0;

	karma_MidiEvent *currentEvent = song->currentEvent;
	while (currentEvent && (currentEvent->time - song->samplesPlayed) <= samples) {
		/* make sure that all events that are or will be due within the next frame
		   are processed */

		int chn = currentEvent->data[0] & 0xf;
		int cmd = currentEvent->data[0] & 0xf0;
		if (cmd == 0x90) {
			/* karma does not currently support different velocitys for keys
			   so just set it to 64 */
			currentEvent->data[2] = 64;
		}
		if (cmd == 0xb0 && (currentEvent->data[1] == 120 || currentEvent->data[1] >= 123)) {
			for (i = 0; i < 16; i++)
				karma_Channel_allNotesOff(&channel[i]);
		} else {
			karma_MidiEvent *kevent = (karma_MidiEvent*) malloc(sizeof(karma_MidiEvent));
			memset(kevent, 0, sizeof(karma_MidiEvent));
			kevent->data[0] = currentEvent->data[0];
			kevent->data[1] = currentEvent->data[1];
			kevent->data[2] = currentEvent->data[2];
			kevent->time = currentEvent->time - song->samplesPlayed;
			if (kevent->time < 0) kevent->time = 0;
			karma_Channel_addEvent(&channel[chn], kevent);
		}
		currentEvent = currentEvent->next;
	}
	song->currentEvent = currentEvent;

	while (pos < samples) {
		int len = samples - pos;
		if (len > BUFFERSIZE) len = BUFFERSIZE;
		for (i = 0; i < 16; i++) {
			karma_Channel_process(&channel[i], &left[pos], &right[pos], len);
		}
		pos += len;
	}
	song->samplesPlayed += samples;
}

/*-----------------------------------------------------------------------------------------*/

static void addEvent(karma_Song *song, karma_MidiEvent *e) {
	if (!song->event || song->event->time > e->time) {
		/* insert event first in list */
		e->next = song->event;
		song->event = e;
	} else {
		karma_MidiEvent *ev = song->event;
		while (ev->next && ev->next->time <= e->time) {
			ev = ev->next;
		}
		e->next = ev->next;
		ev->next = e;
	}
}

/*-----------------------------------------------------------------------------------------
  karma_loadSong - load a song
-----------------------------------------------------------------------------------------*/
karma_Song *karma_loadSong(unsigned char *buff) {
	union fConvert {
		int ival;
		float fval;
	};

	int pos = 0;
	int i;
	int timeformat;
	int eventNum;
	float samplesPerTick;
	karma_Song *song = (karma_Song*) malloc(sizeof(karma_Song));
	memset(song, 0, sizeof(karma_Song));

	/* read instruments */
	for (i = 0; i < 16; i++) {
		int j;
		for (j = 0; j < 34; j++) {
			union fConvert f;
			f.ival = buff[pos] << 24 |  buff[pos+1] << 16 | buff[pos+2] << 8 | buff[pos+3] << 0; pos += 4;
			karma_Program_setParameter(&channel[i].program, j, f.fval);
		}
	}

	/* read midi-data */
	timeformat = buff[pos] << 8 | buff[pos+1] << 0; pos+=2;
	eventNum = buff[pos] << 8 | buff[pos+1] << 0; pos+=2;

	samplesPerTick = (60.0f / 120.0f * 44100.0f) / (float) timeformat;

	while (eventNum) {
		long lastTime = 0;
		float fraction = 0;

		karma_MidiEvent *root = NULL;
		karma_MidiEvent *e = NULL;

		int var = 1;
		char last = 0;

		/* number of events of this type */
		int num = buff[pos] << 8 | buff[pos+1] << 0; pos+=2;


		/* Read all the delta times for this type of event */
		for (i = 0; i < num; i++) {
			int tmp = 0;
			karma_MidiEvent *ne = (karma_MidiEvent*) malloc(sizeof(karma_MidiEvent));
			ne->time = 0;
			do {
				tmp = buff[pos]; pos++;
				ne->time = (ne->time << 7) | (tmp & 0x7F);
			} while (tmp & 0x80);

			if (ne->time > 0) {
				float samples = ne->time * samplesPerTick + fraction;
				ne->time = (long) samples;
				fraction = samples - ne->time;
			}

			ne->time += lastTime;

			lastTime = ne->time;

			if (!root) {
				e = root = ne;
			}
			e->next = ne;
			e = e->next;
		}

		/* get the type of event */
		root->data[0] = buff[pos]; pos++;
		if ((root->data[0] & 0xf0) == 0xb0) {
			/* this is a midi-controller command so we need to
			   get the midicontroller also */
			root->data[1] = (buff[pos])&0x7f; pos++;
			var++;
		}

		/* read all the events */
		e = root;
		for (i = 0; i < num; i++) {
			char curr;
			karma_MidiEvent *te;

			e->data[0] = root->data[0];
			e->data[1] = root->data[1];

			curr = (buff[pos]) + last; pos++;
			e->data[var] = curr&0x7f;
			last = e->data[var];

			te = e;
			e = e->next;
			addEvent(song, te);
		}
		eventNum -= num;
	}
	song->currentEvent = song->event;

	return song;
}

/*-----------------------------------------------------------------------------------------
  karma_freeSong - free all resources allocated by a song
-----------------------------------------------------------------------------------------*/
void karma_freeSong(karma_Song *song) {
	karma_MidiEvent *event = song->event;
	while (event) {
		karma_MidiEvent *e = event;
		event = event->next;
		free(e);
	}
}

/*-----------------------------------------------------------------------------------------
  karma_free - free all allocated resources
-----------------------------------------------------------------------------------------*/
void karma_free() {
	int i;
	for (i = 0; i < 16; i++) {
		karma_Channel_free(&channel[i]);
	}
}

/*-----------------------------------------------------------------------------------------*/

