#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vstkarma.h"

// *very* basic monophonic 'synth' example

enum
{
	kWaveSize = 44100	// samples (must be power of 2 here)
};

//const double midiScaler = (1. / 127.);
//static float fScaler = kWaveSize / 44100.0f;

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void VstKarma::setSampleRate (float sampleRate)
{
	// TODO: check
	AudioEffectX::setSampleRate(sampleRate);
//	fScaler = (float)((double)kWaveSize / (double)sampleRate);
}

//-----------------------------------------------------------------------------------------
void VstKarma::setBlockSize (long blockSize)
{
	AudioEffectX::setBlockSize (blockSize);
	// you may need to have to do something here...
}

//-----------------------------------------------------------------------------------------
void VstKarma::resume ()
{
	wantEvents ();
}

//-----------------------------------------------------------------------------------------
void VstKarma::initProcess ()
{
//	fScaler = (float)((double)kWaveSize / 44100.);	// we don't know the sample rate yet
}

int buffer[44100];
//-----------------------------------------------------------------------------------------
void VstKarma::process (float **inputs, float **outputs, long sampleFrames)
{
	if (sampleFrames <= 44100) {
		memset(&buffer, 0, 44100 * sizeof(int));
		for (int i = 0; i < 16; i++) {
			channel[i].process(buffer, sampleFrames);
		}
		for (int i = 0; i < sampleFrames; i++) {
			outputs[0][i] += buffer[i] / 32767.0f;
		}

	}
}


//-----------------------------------------------------------------------------------------
void VstKarma::processReplacing (float **inputs, float **outputs, long sampleFrames)
{

	if (sampleFrames <= 44100) {
		memset(&buffer, 0, 44100 * sizeof(int));
		for (int i = 0; i < 16; i++) {
			channel[i].process(buffer, sampleFrames);
		}
		for (int i = 0; i < sampleFrames; i++) {
			outputs[0][i] = buffer[i] / 32767.0f;
		}
	}
}

//-----------------------------------------------------------------------------------------
long VstKarma::processEvents (VstEvents* ev)
{
	for (long i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type != kVstMidiType)
			continue;
		VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
		char* midiData = event->midiData;
		long cmd = midiData[0] & 0xf0;		// extract command
		long chn = midiData[0] & 0x0f;		// extract channel

		if (cmd == 0x90 || cmd == 0x80)	{	// note on / note off
			long note = midiData[1] & 0x7f;
			long velocity = midiData[2] & 0x7f;
			if (cmd == 0x80)
				velocity = 0;
			if (!velocity/* && (note == channel[chn].currentNote)*/)
				channel[chn].noteOff(note);	// note off by velocity 0
			else
				channel[chn].noteOn(note, velocity, event->deltaFrames);
//		} else if (cmd == 0xc0) { // Program change
//			long program = midiData[1] & 0x7f;
//			channel[chn].setProgram(&programs[program]);
		} else if (cmd == 0xb0)	// Channel Mode Messages
			if (midiData[1] == 120 || midiData[1] >= 123) {
				// 120 -> All sounds off
				// 123->127 different all notes off (TODO: implement better)
				for (int i = 0; i < 16; i++)
					channel[i].AllNotesOff();

				if (midiData[1] > 123) {
					char buf[256];
					sprintf(buf, "warning: %d, %d\n", cmd, midiData[1]);
					Debug(buf);
				}
			}
		else {
			char buf[256];
			sprintf(buf, "other command: %d, %d\n", cmd, midiData[1]);
			Debug(buf);
		}
		event++;
	}
	return 1;	// want more
}
/*
//-----------------------------------------------------------------------------------------
void VstKarma::noteOn(long note, long velocity, long delta)
{
	channel[i].fPhase1 = channel[i].fPhase2 = 0;
	channel[i].currentNote = note;
	channel[i].currentVelocity = velocity;
	channel[i].currentDelta = delta;
	channel[i].noteIsOn = true;
	channel[i].samplesPlayed = 0;
	high = band = low = notch = 0;
	channel[i].freq1.triggerKey();
	channel[i].volume1.triggerKey();
	channel[i].freq2.triggerKey();
	channel[i].volume2.triggerKey();
	channel[i].active = true;
}

//-----------------------------------------------------------------------------------------
void VstKarma::noteOff() {
	channel[i].noteIsOn = false;
	channel[i].freq1.releaseKey();
	channel[i].volume1.releaseKey();
	channel[i].freq2.releaseKey();
	channel[i].volume2.releaseKey();
}

*/