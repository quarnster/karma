#include <stdlib.h>

#include "karma.h"
#include "karmainternal.h"

static karma_Channel channel[16];

void karma_init() {
	int i;
	karma_Waveform_initTables();

	for (i = 0; i < 16; i++) {
		karma_Channel_init(&channel[i]);
	}
}

void karma_process(karma_Song *song, int *left, int *right, int samples) {
	int i;

	karma_MidiEvent *currentEvent = song->currentEvent;
	while (currentEvent && (currentEvent->time - song->samplesPlayed) <= samples) {
		int chn = currentEvent->data[0] & 0xf;
		int cmd = currentEvent->data[0] & 0xf0;
		if (cmd == 0x90) {
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

	for (i = 0; i < 16; i++) {
		karma_Channel_process(&channel[i], left, right, samples);
	}
	song->samplesPlayed += samples;
}

static void addEvent(karma_Song *song, karma_MidiEvent *e) {
	if (!song->event || song->event->time > e->time) {
		// first element
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
union fConvert {
	int ival;
	float fval;
};


karma_Song *karma_loadSong(unsigned char *buff) {
	int pos = 0;
	int i;
	int timeformat;
	int eventNum;
	float samplesPerTick;
	karma_Song *song = (karma_Song*) malloc(sizeof(karma_Song));
	memset(song, 0, sizeof(karma_Song));

	// read instruments
	for (i = 0; i < 16; i++) {
		int j;
		for (j = 0; j < 34; j++) {
			union fConvert f;
			f.ival = buff[pos] << 24 |  buff[pos+1] << 16 | buff[pos+2] << 8 | buff[pos+3] << 0; pos += 4;
			karma_Program_setParameter(&channel[i].program, j, f.fval);
		}
	}

	// read midi-data
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

		int num = buff[pos] << 8 | buff[pos+1] << 0; pos+=2;


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

		root->data[0] = buff[pos]; pos++;
		if ((root->data[0] & 0xf0) == 0xb0) {
			root->data[1] = (buff[pos])&0x7f; pos++;
			var++;
		}


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

void karma_freeSong(karma_Song *song) {
	karma_MidiEvent *event = song->event;
	while (event) {
		karma_MidiEvent *e = event;
		event = event->next;
		free(e);
	}
}

void karma_free() {
	int i;
	for (i = 0; i < 16; i++) {
		karma_Channel_free(&channel[i]);
	}
}

