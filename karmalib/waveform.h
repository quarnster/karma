/*
 * Karma softsynth
 * 
 * $Id: waveform.h,v 1.3 2003/12/30 16:09:09 quarn Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
#ifndef __INCLUDED_KARMA_WAVEFORM_H
#define __INCLUDED_KARMA_WAVEFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#define LIMIT 32767

#define WAVETABLESIZE 8192	/* Note that you have to change the shift down below also if you change this
				   32 - 19 = 13 -> 2^13 = 8192 */

#define PHASE2TABLE_NORMAL(phase) (((int) (phase >> 19)) & (WAVETABLESIZE-1))
#define PHASE2TABLE_SQUARE(phase, wavelen) (PHASE2TABLE_NORMAL(phase) > wavelen ? 0 : 1)
#define PHASE2TABLE(waveform, phase, wavelen) (waveform == 3 ? PHASE2TABLE_SQUARE(phase, wavelen) : PHASE2TABLE_NORMAL(phase))

extern int sineTable[WAVETABLESIZE];
extern int triTable[WAVETABLESIZE];
extern int sawTable[WAVETABLESIZE];
extern int squareTable[WAVETABLESIZE];
extern int noiseTable[WAVETABLESIZE];


void karma_Waveform_initTables();

#ifdef __cplusplus
}
#endif

#endif 

