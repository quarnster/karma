#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "waveform.h"

#ifdef __GNUC__
int   sineTable[WAVETABLESIZE]	__attribute__((aligned(32)));
int    triTable[WAVETABLESIZE]	__attribute__((aligned(32)));
int    sawTable[WAVETABLESIZE]	__attribute__((aligned(32)));
int squareTable[WAVETABLESIZE]	__attribute__((aligned(32)));
int  noiseTable[WAVETABLESIZE]	__attribute__((aligned(32)));
#else
__declspec(align(32)) int sineTable[WAVETABLESIZE];
__declspec(align(32)) int triTable[WAVETABLESIZE];
__declspec(align(32)) int sawTable[WAVETABLESIZE];
__declspec(align(32)) int squareTable[WAVETABLESIZE];
__declspec(align(32)) int noiseTable[WAVETABLESIZE];
#endif

void karma_Waveform_initTables() {
	int i;
	
	for (i = 0; i < WAVETABLESIZE; i++) {
		float prog = (float) (i / ((float) ((WAVETABLESIZE-1))));
		sineTable[i] = (int) (sin(prog * 2 * 3.1415927f) * LIMIT);
		{
			if (prog < 0.25) {
				triTable[i] = (prog * 4) * LIMIT;
			} else if (prog > 0.75) {
				triTable[i] = (-1 + (prog-0.75) * 4) * LIMIT;
			} else {
				triTable[i] = (1 - (prog-0.25) * 4) * LIMIT;
			}
		}
		sawTable[i] = (-1 + 2 * prog) * LIMIT;
		noiseTable[i] = (((rand() % 2048) - 1024) * LIMIT) >> 10;
	}
	squareTable[0] = LIMIT;
	squareTable[1] = -LIMIT;
}
