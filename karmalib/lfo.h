/*
 * Karma softsynth
 * 
 * $Id: lfo.h,v 1.3 2003/12/30 16:09:09 quarn Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
#ifndef __INCLUDED_KARMA_LFO_H
#define __INCLUDED_KARMA_LFO_H

#include "waveform.h"

typedef struct {
	int waveform;
	int* waveformTable;

	float rate;
	int amount;

	int phase;
} karma_LFO;


#endif

