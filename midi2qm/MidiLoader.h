/*
 * Converts midi-files and fxb-banks into the format used by the Karma synth
 * 
 * $Id: MidiLoader.h,v 1.2 2004/01/01 16:06:19 Fredrik Ehnbom Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
#include <stdio.h>

struct MidiEvent {
	MidiEvent *next;
	long time;
	char data[3];
};
class MidiLoader {
	
public:
	MidiEvent *event;
	int eventNum;
	int timeformat;

	MidiLoader(FILE *in);
	~MidiLoader();

	void readTrack(FILE *fp);
	void addEvent(MidiEvent *e);
};