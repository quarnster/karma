#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vstkarma.h"

// *very* basic monophonic 'synth' example

enum
{
	kNumFrequencies = 128,	// 128 midi notes
	kWaveSize = 44100	// samples (must be power of 2 here)
};

const double midiScaler = (1. / 127.);
static float fScaler = kWaveSize / 44100.0f;
//static float sawtooth[kWaveSize];
//static float pulse[kWaveSize];
static float freqtab[kNumFrequencies];

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void VstKarma::setSampleRate (float sampleRate)
{
	// TODO: check
	AudioEffectX::setSampleRate(sampleRate);
	fScaler = (float)((double)kWaveSize / (double)sampleRate);
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
//	channel[i].fPhase1 = channel[i].fPhase2 = 0.f;
	fScaler = (float)((double)kWaveSize / 44100.);	// we don't know the sample rate yet
//	channel[i].noteIsOn = false;
//	noteIsOn = false;
//	currentDelta = 0;
/*
	long i;

	// make waveforms
	long wh = kWaveSize / 4;	// 1:3 pulse
	for (i = 0; i < kWaveSize; i++)
	{
		sawtooth[i] = (float)(-1. + (2. * ((double)i / (double)kWaveSize)));
		pulse[i] = (i < wh) ? -1.f : 1.f;
	}
*/
	// make frequency (Hz) table
	double k = 1.059463094359;	// 12th root of 2
	double a = 6.875;	// a
	a *= k;	// b
	a *= k;	// bb
	a *= k;	// c, frequency of midi note 0
	for (long i = 0; i < kNumFrequencies; i++)	// 128 midi notes
	{
		freqtab[i] = (float)a;
		a *= k;
	}
	char buf[256];
	sprintf(buf, "max freq: %f\n", freqtab[127]);
	Debug(buf);

	Debug("done init process\n");
}

//-----------------------------------------------------------------------------------------
void VstKarma::process (float **inputs, float **outputs, long sampleFrames)
{
	for (int i = 0; i < 16; i++) {
		channel[i].process(outputs[0], sampleFrames);
	}						
}


//-----------------------------------------------------------------------------------------
void VstKarma::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
	float *out1 = outputs[0];
	for (int i = 0; i < sampleFrames; i++) {
		out1[i] = 0;
	}

	for (int i = 0; i < 16; i++) {
		channel[i].process(outputs[0], sampleFrames);
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
		long chn = midiData[0] & 0x0f;	// extract channel
		if (cmd == 0x90 || cmd == 0x80)		// we only look at notes
		{
			long note = midiData[1] & 0x7f;
			long velocity = midiData[2] & 0x7f;
			if (cmd == 0x80)
				velocity = 0;
			if (!velocity && (note == channel[chn].currentNote))
				channel[chn].noteOff();	// note off by velocity 0
			else
				channel[chn].noteOn(note, velocity, event->deltaFrames);
		} else if (cmd == 0xc0) { // Program change
			long program = midiData[1] & 0x7f;
			channel[chn].setProgram(&programs[program]); //setProgram(program);
		} else if (cmd == 0xb0 && midiData[1] == 0x7e)	// all notes off
			for (int i = 0; i < 16; i++)
				channel[i].noteOff();
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