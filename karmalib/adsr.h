/*
 * Karma softsynth
 * 
 * $Id: adsr.h,v 1.3 2003/12/30 16:09:09 quarn Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
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

