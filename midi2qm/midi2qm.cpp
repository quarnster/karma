/*
 * Converts midi-files and fxb-banks into the format used by the Karma synth
 * 
 * $Id: midi2qm.cpp,v 1.3 2004/01/01 16:06:19 Fredrik Ehnbom Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "MidiLoader.h"

#define cMagic 		'CcnK'
#define fMagic		'FxCk'
#define bankMagic	'FxBk'


extern int read4bytes(FILE *fp);
void write4bytes(FILE *fp, int blah) {
	fputc((blah >> 24) & 0xff, fp);
	fputc((blah >> 16) & 0xff, fp);
	fputc((blah >>  8) & 0xff, fp);
	fputc((blah >>  0) & 0xff, fp);
}

void write2bytes(FILE *fp, int blah) {
	fputc((blah >>  8) & 0xff, fp);
	fputc((blah >>  0) & 0xff, fp);
}

void convertFxp(FILE *in, FILE *out) {
	int header = read4bytes(in);
	int length = read4bytes(in);

	if (header !=  cMagic) {
		printf("ERROR!!! header != cMagic!!\n");
		printf("%d, %d\n", header, cMagic);
		return;
	}

	int header2 = read4bytes(in);
	switch (header2) {
		case bankMagic:
			{
				fseek(in, 3*4, SEEK_CUR);
				int programs = read4bytes(in);

				fseek(in, 128, SEEK_CUR);

				for (int i = 0; i < programs; i++) {
					convertFxp(in, out);
				}
			}
			break;
		case fMagic:
			{
				fseek(in, 3*4, SEEK_CUR);
				int params = read4bytes(in);
				printf("params: %d\n", params);
				fseek(in, 28, SEEK_CUR);

				for (int i = 0; i < params; i++) {
					write4bytes(out, read4bytes(in));
				}
			}
			break;
		default:
			printf("ERROR!!! Unknown chunk: %d\n", header);
			printf("len: %d\n", length);
			

			fseek(in, length - 4, SEEK_CUR);
			break;

	}
}

void sort(MidiEvent *events, int num) {

	// Make noteons with 0-velocity into noteoffs
	for (int i = 0; i < num; i++) {
		if ((events[i].data[0] & 0xf0) == 0x90 && !events[i].data[2]) {
			events[i].data[0] = 0x80 | (events[i].data[0] & 0xf);
		}
	}

	// sort by commands
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			if (events[i].data[0] > events[j].data[0]) {
				MidiEvent e;
				memcpy(&e, &events[j], sizeof(MidiEvent));
				memcpy(&events[j], &events[i], sizeof(MidiEvent));
				memcpy(&events[i], &e, sizeof(MidiEvent));
			}
		}
	}

	// sort 0xb0-commands by argument 1
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			if ((events[i].data[0] &0xf0) == 0xb0 && (events[i].data[0] == events[j].data[0])) {
				if (events[i].data[1] > events[j].data[1]) {
					MidiEvent e;
					memcpy(&e, &events[j], sizeof(MidiEvent));
					memcpy(&events[j], &events[i], sizeof(MidiEvent));
					memcpy(&events[i], &e, sizeof(MidiEvent));
				}
			}
		}
	}

	// sort all non 0xb0-commands by time
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			if ((events[i].data[0] &0xf0) != 0xb0 && (events[i].data[0] == events[j].data[0]) &&  events[i].time < events[j].time) {
				MidiEvent e;
				memcpy(&e, &events[j], sizeof(MidiEvent));
				memcpy(&events[j], &events[i], sizeof(MidiEvent));
				memcpy(&events[i], &e, sizeof(MidiEvent));
			}
		}
	}

	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			if ((events[i].data[0] &0xf0) == 0xb0 && (events[i].data[0] == events[j].data[0]) && (events[i].data[1] == events[j].data[1])) {
				if (events[i].time < events[j].time) {
					MidiEvent e;
					memcpy(&e, &events[j], sizeof(MidiEvent));
					memcpy(&events[j], &events[i], sizeof(MidiEvent));
					memcpy(&events[i], &e, sizeof(MidiEvent));
				}
			}
		}
	}
}
void writeVarInt(FILE *fp, int val) {
	val &= 0x7fffffff;
	if (val < 128)
		fputc(val&0x7f, fp);
	else if (val < 16383) {
		fputc(0x80 | (val >> 7) &0x7f, fp);
		fputc(val&0x7f, fp);
	} else if (val < 2097152) {
		fputc(0x80 | (val >> 14) &0x7f, fp);
		fputc(0x80 | (val >> 7) &0x7f, fp);
		fputc(val&0x7f, fp);
	} else {
		fputc(0x80 | (val >> 21) &0x7f, fp);
		fputc(0x80 | (val >> 14) &0x7f, fp);
		fputc(0x80 | (val >> 7) &0x7f, fp);
		fputc(val&0x7f, fp);
	}
}
void writeTrack(FILE *fp, MidiEvent *events, int num) {
	write2bytes(fp, num);

	int lastTime = 0;
	for (int i = 0; i < num; i++) {
		// write deltas
		int curr = events[i].time - lastTime;
		writeVarInt(fp, curr);
		lastTime += curr;
	}

	putc(events[0].data[0]&0xff, fp);
	int pos = 1;
	if ((events[0].data[0] & 0xf0) == 0xb0) {
		putc(events[0].data[1]&0xff, fp);
		pos++;
	}

	char last = 0;
	for (int i = 0; i < num; i++) {
		// write data
		char curr = events[i].data[pos] - last;
		putc(curr&0xff, fp);
		last += curr;
	}

}

int getTrackLen(MidiEvent *events, int len) {
	int i;
	char data[2];
	data[0] = events[0].data[0];
	data[1] = events[0].data[1];
	for (i = 1; i < len; i++) {
		if (events[i].data[0] != data[0]) {
			break;
		} else if ((data[0] & 0xf0) == 0xb0 && data[1] != events[i].data[1]) {
			break;
		}
	}
	return i;
}
int main(int argc, char **argv)
{
	if (argc != 4) {
		printf("usage: %s file.fxp file.mid file.qm\n", argv[0]);
		return 0;
	}

	FILE *fxp;
	FILE *mid;
	FILE *qm;

	if ((fxp = fopen(argv[1], "rb")) == NULL) {
		printf("File %s could not be opened\n", argv[1]);
		return -1;
	}

	if ((mid = fopen(argv[2], "rb")) == NULL) {
		printf("File %s could not be opened\n", argv[2]);
		fclose(fxp);
		return -1;
	}
	if ((qm = fopen(argv[3], "wb")) == NULL) {
		printf("File %s could not be opened\n", argv[3]);
		fclose(fxp);
		fclose(mid);
		return -1;
	}

	convertFxp(fxp, qm);
	MidiLoader midi(mid);

	MidiEvent *events = new MidiEvent[midi.eventNum];
	MidiEvent *ev = midi.event;
	for (int i = 0; i < midi.eventNum; i++) {
		memcpy(&events[i], ev, sizeof(MidiEvent));
		ev = ev->next;
	}

	sort(events, midi.eventNum);
	write2bytes(qm, midi.timeformat);
	write2bytes(qm, midi.eventNum);

	int index = 0;
	while (index != midi.eventNum) {
		int len = getTrackLen(&events[index], midi.eventNum - index);
		writeTrack(qm, &events[index], len);
		index += len;
	}

	delete[] events;

	fclose(fxp);
	fclose(mid);
	fclose(qm);
	return 0;
}

