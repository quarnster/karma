/*
 * An example showing how to use the Karma softsynth
 * 
 * $Id: device.h,v 1.2 2004/01/01 16:06:19 Fredrik Ehnbom Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
#include <karma.h>

#define BUFFERSIZE 8192


extern karma_Song *ksong;

extern int deviceOpen();
extern void deviceClose();
extern void deviceUpdate();

