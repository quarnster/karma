#include <math.h>
#include <stdlib.h>
#include "waveform.h"

#define LIMIT 32767

int sineTable[4096];

void init_sineTable() {
	int i;
	for (i = 0; i < 4096; i++) {
		sineTable[i] = (int) (sin(i / 4096.0f * 2 * 3.1415927f) * LIMIT);
	}
}

int sineSample(int phase, int wavelen) {
	int sample = (int) (phase >> 20);
	int pl = sample&4095;

	return sineTable[pl];
}
int triSample(int phase, int wavelen) {
	int sample = (int) (phase >> 20);
	int pl = sample&4095;

	if (pl > 3072) {
		sample = (pl - 4096) * LIMIT;
	} else if (pl < 1024) {
		sample = pl * LIMIT;
	} else {
		sample = (2048 - pl) * LIMIT;
	}

	return sample >> 10;
}
int sawSample(int phase, int wavelen) {
	return (phase >> 20) << 4;
}
int squareSample(int phase, int wavelen) {
	int sample = (int) (phase >> 20);
	int pl = sample&4095;

	return pl > wavelen ? LIMIT : -LIMIT;
}
int noiseSample(int phase, int wavelen) {
	return (((rand() % 2048) - 1024) * LIMIT) >> 10;
}
