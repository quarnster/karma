#ifndef __INCLUDED_KARMA_ADSR_H
#define __INCLUDED_KARMA_ADSR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int attack;
	int decay;
	int sustain;
	int release;
} karma_ADSR;

int karma_ADSR_getValue(karma_ADSR *adsr, long samplepos, long relSample);

#ifdef __cplusplus
}
#endif
#endif

