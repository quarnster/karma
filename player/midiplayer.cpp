#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include "MidiPlayer.h"

int read4bytes(FILE *fp) {
	int tmp1 = fgetc(fp);
	int tmp2 = fgetc(fp);
	int tmp3 = fgetc(fp);
	int tmp4 = fgetc(fp);

//	printf("read: %d, %d, %d, %d\n", tmp1, tmp2, tmp3, tmp4);
	return  tmp1 << 24 | tmp2  << 16 | tmp3  << 8 | tmp4;
}
int read2bytes(FILE *fp) {
	return fgetc(fp) << 8 | fgetc(fp);
}
int readVarInt(FILE *fp, int *read) {
	int ret = 0;
	int tmp = 0;
	do {
		(*read)--;
		tmp = fgetc(fp);
		ret = (ret << 7) | (tmp & 0x7F);
	} while (tmp & 0x80);
	return ret;
}
float samplesPerTick = 60.0 / 120.0 * 44100.0f;

MidiPlayer::MidiPlayer(char *file) {
	event = NULL;
	FILE *in = NULL;
	if ((in = fopen(file, "rb")) == NULL) {
		printf("Error opening file!\n");
		return;
	}
	char buf[256];
	fread(buf, 4, 1, in);
	buf[4] = '\0';
	printf("track? %s\n", buf);
	if (strncmp(buf, "MThd", 4)) {
		printf("Not a midifile!\n");
		printf("%s != MThd\n", buf);
		fclose(in);
		return;
	}

	int length = read4bytes(in);
	int filetype = read2bytes(in);
	int tracknum = read2bytes(in);
	int timeformat = read2bytes(in);
	samplesPerTick /= (float) timeformat;

	printf("filetype: %d, tracknum: %d, format: %d\n", filetype, tracknum, timeformat);

	for (int i = 0; i < tracknum; i++) {
		readTrack(in);
	}
/*
	MidiEvent *ev = event;
	float samplesPerTick = 60.0 / 120.0 * 44100.0f / timeformat;
	float fraction = 0;
	long last = 0;
	while (ev != NULL) {
		if (ev->time > 0) {
			float samples = ev->time * samplesPerTick + fraction;
			ev->time = (long) samples;
			fraction = samples - ev->time;
		}
		ev->time += last;
		last = ev->time;

		ev = ev->next;
	}
*/
	fclose(in);
}

static const char eventLength[] = {
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void MidiPlayer::addEvent(MidiEvent *e) {
	if (!event || event->time > e->time) {
		// first element
		e->next = event;
		event = e;
	} else {
		MidiEvent *ev = event;
		while (ev->next && ev->next->time <= e->time) {
			ev = ev->next;
		}
		e->next = ev->next;
		ev->next = e;
	}
}

void MidiPlayer::readTrack(FILE *fp) {
	char buf[256];
	fread(buf, 4, 1, fp);
	buf[4] = '\0';
	printf("track? %s\n", buf);

	if (strncmp(buf, "MTrk", 4)) {
		printf("Not a miditrack!\n");
		return;
	}

	int eventNum = 0;
	int bytesLeft = read4bytes(fp);

	bool lastOk = false;
//	MidiEvent *lastEvent = NULL;
	int last = 0;

	float fraction = 0;
	long lastTime = 0;

	while (bytesLeft > 0/* && !feof(fp)*/) {
		MidiEvent *e = NULL;
		
		long delay = readVarInt(fp, &bytesLeft);
		int byte = fgetc(fp); bytesLeft--;
		int cmd = byte & 0xF0;
		int pos = 0;

		if (byte == 0xFF) {
			lastOk = false;
			last = byte;
			fgetc(fp); // type
			bytesLeft--;
			int len = readVarInt(fp, &bytesLeft);
			bytesLeft -= len;
			fseek(fp, len, SEEK_CUR);
			continue;
		} else if (byte == 0xF0 || byte == 0xF7) {
			lastOk = false;
			last = byte;
			int len = readVarInt(fp, &bytesLeft);
			bytesLeft -= len;
			fseek(fp, len, SEEK_CUR);
			continue;
		}

		if (byte >= 0x80) {
			// new command
			if (cmd == 0x90 || cmd == 0x80 || cmd == 0xb0) {
				e = (MidiEvent*) malloc(sizeof(MidiEvent));
				if (e) {
					memset(e, 0, sizeof(MidiEvent));
					e->time = delay;
					e->data[pos++] = byte;
				} else printf("out of memory????!??!?!\n");

//				memset(e, 0, sizeof(MidiEvent));
//				e->time = delay;
//				e->data[pos++] = byte;
				lastOk = true;
			} else {
				lastOk = false;
			}
		} else if (lastOk) {
			// same as previous command
			e = (MidiEvent*) malloc(sizeof(MidiEvent));
			if (e) {
				memset(e, 0, sizeof(MidiEvent));
				e->time = delay;
				e->data[pos++] = last;
				e->data[pos++] = byte;
				byte = cmd = e->data[0]&0xff;
			} else printf("out of memory????!??!?!\n");
		} else if (last == 0xFF) {
			fgetc(fp); // type
			bytesLeft--;
			int len = readVarInt(fp, &bytesLeft);
			bytesLeft -= len;
			fseek(fp, len, SEEK_CUR);
			continue;
		} else if (last == 0xF0 || last == 0xF7) {
			int len = readVarInt(fp, &bytesLeft);
			bytesLeft -= len;
			fseek(fp, len, SEEK_CUR);
			continue;
		}

		while (pos-1 < eventLength[cmd-0x80]) {
			int read = fgetc(fp); bytesLeft--;
			if (e != NULL)
				e->data[pos] = read;
			pos++;
		};

		if (e) {
			if (e->time > 0) {
				float samples = e->time * samplesPerTick + fraction;
				e->time = (long) samples;
				fraction = samples - e->time;
			}
			e->time += lastTime;
			lastTime = e->time;

			addEvent(e);
			eventNum++;
		}
/*
		if (event == NULL) {
			event = e;
		}
		if (root == NULL) {
			lastEvent = root = e;
		}
		if (e != NULL) {
			lastEvent->next = e;
			lastEvent = e;
			eventNum++;
		}
*/
		last = byte;
	}
	printf("A total of %d events was read\n", eventNum);
}

MidiPlayer::~MidiPlayer() {
	int i = 0;
	if (event) {
		while (event != NULL) {
			MidiEvent *e = event;
			event = event->next;
			free(e);
			i++;
		}
	}
	printf("%d events freed\n", i);
}