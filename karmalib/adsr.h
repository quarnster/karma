#ifndef __INCLUDED_KARMA_ADSR_H
#define __INCLUDED_KARMA_ADSR_H

typedef struct {
	int attack;
	int decay;
	int sustain;
	int release;
} karma_ADSR;

#ifdef __cplusplus
extern "C" {
#endif

int karma_ADSR_getValue(karma_ADSR *adsr, long samplepos, long relSample);

#ifdef __cplusplus
}
#endif
#endif