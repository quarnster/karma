#ifndef __INCLUDED_KARMA_WAVEFORM_H
#define __INCLUDED_KARMA_WAVEFORM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*WAVEFORM_CALLBACK) (int phase,int wavelen);

void init_sineTable();

int sineSample(int phase, int wavelen);
int triSample(int phase, int wavelen);
int sawSample(int phase, int wavelen);
int squareSample(int phase, int wavelen);
int noiseSample(int phase, int wavelen);

#ifdef __cplusplus
}
#endif

#endif 