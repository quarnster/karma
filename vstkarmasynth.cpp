#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vstkarma.h"
#include "param.h"

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

#ifdef __GNUC__
static int bufferL[BUFFERSIZE] __attribute__((aligned(32)));
static int bufferR[BUFFERSIZE] __attribute__((aligned(32)));
#else
static __declspec(align(32)) int bufferL[BUFFERSIZE];
static __declspec(align(32)) int bufferR[BUFFERSIZE];
#endif

//-----------------------------------------------------------------------------------------
void VstKarma::process (float **inputs, float **outputs, long sampleFrames)
{
	if (sampleFrames <= BUFFERSIZE) {
		memset(&bufferL, 0, BUFFERSIZE * sizeof(int));
		memset(&bufferR, 0, BUFFERSIZE * sizeof(int));
		for (int i = 0; i < 16; i++) {
			karma_Channel_process(&channel[i], bufferL, bufferR, sampleFrames);
		}
		for (int i = 0; i < sampleFrames; i++) {
			float samplel = bufferL[i] / 32767.0f;
			float sampler = bufferR[i] / 32767.0f;
			samplel = samplel > 1.0f ? 1.0f : samplel < -1.0f ? -1.0f : samplel;
			sampler = sampler > 1.0f ? 1.0f : sampler < -1.0f ? -1.0f : sampler;
			outputs[0][i] += samplel;
			outputs[1][i] += sampler;
		}
	}
}


//-----------------------------------------------------------------------------------------
void VstKarma::processReplacing (float **inputs, float **outputs, long sampleFrames)
{

	if (sampleFrames <= BUFFERSIZE) {
		memset(&bufferL, 0, BUFFERSIZE * sizeof(int));
		memset(&bufferR, 0, BUFFERSIZE * sizeof(int));
		for (int i = 0; i < 16; i++) {
			karma_Channel_process(&channel[i], bufferL, bufferR, sampleFrames);
		}

		for (int i = 0; i < sampleFrames; i++) {
			float samplel = bufferL[i] / 32767.0f;
			float sampler = bufferR[i] / 32767.0f;
			samplel = samplel > 1.0f ? 1.0f : samplel < -1.0f ? -1.0f : samplel;
			sampler = sampler > 1.0f ? 1.0f : sampler < -1.0f ? -1.0f : sampler;
			outputs[0][i] = samplel;
			outputs[1][i] = sampler;
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
				karma_Channel_noteOff(&channel[chn], note);	// note off by velocity 0
			else
				karma_Channel_noteOn(&channel[chn], note, velocity, event->deltaFrames);
		} else if (cmd == 0xb0)	{// Channel Mode Messages
			if (event->deltaFrames) {
				char buf[256];
				sprintf(buf, "warning: %d, %d\n", midiData[1], event->deltaFrames);
				Debug(buf);
			}

			if (midiData[1] == 120 || midiData[1] >= 123) {
				// 120 -> All sounds off
				// 123->127 different all notes off (TODO: implement better)
				for (int i = 0; i < 16; i++)
					karma_Channel_allNotesOff(&channel[i]);

				if (midiData[1] > 123) {
					char buf[256];
					sprintf(buf, "warning: %d, %d\n", cmd, midiData[1]);
					Debug(buf);
				}
			} else if (midiData[1] == 7) { // volume change
				karma_Program_setParameter(&channel[chn].program, kGain, (midiData[2]&0x7f) / 127.0f);
			} else if (midiData[1] == 10) { // pan change
				float value = (midiData[2]&0x7f) / 127.0f;
				channel[chn].panl = sqrt(1.0f-value) * 1024;
				channel[chn].panl = sqrt(value) * 1024;
//				karma_Program_setParameter(&channel[chn].program, kPan, (midiData[2]&0x7f) / 127.0f);
			} else if (midiData[1] == 74) { // cut off
				karma_Program_setParameter(&channel[chn].program, kFilterCut, (midiData[2]&0x7f) / 127.0f);
			} else if (midiData[1] >= 12) {
				karma_Program_setParameter(&channel[chn].program, midiData[1] - 12, (midiData[2]&0x7f) / 127.0f);
			} else {
				char buf[256];
				sprintf(buf, "unimplemented control: %d, %d\n", cmd, midiData[1]);
				Debug(buf);
			}
		} else {
			char buf[256];
			sprintf(buf, "other command: %d, %d\n", cmd, midiData[1]);
			Debug(buf);
		}
		event++;
	}
	return 1;	// want more
}
