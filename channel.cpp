#include "Channel.h"
#include "vstkarma.h"
#include <math.h>
#include <stdlib.h>

#ifndef NULL
#define NULL 0
#endif

//-----------------------------------------------------------------------------------------
Channel::Channel() {
	program = NULL;
	currentNote = -1;
	released = true;
	active = false;
	samplesPlayed = 0;
	relSample = -1;
	high = band = low = notch = 0;
}

//-----------------------------------------------------------------------------------------
float Channel::getSample(float wave, float phase) {
	float limit = 44100;
	float sample = -1 + (2 * (phase/limit));
	if (wave < 0.2)					// off
		return 0;
	else if (wave < 0.4)				// saw
		return sample;
	else if (wave < 0.6)				// square
		return sample > 0 ? 1.0f : -1.0f;
	else if (wave < 0.8)				// sine
		return sinf(sample * 3.1415927f * 2);
	else						// noise
		return (float) (2 * (((rand() % 1000) / 1000.0f)- 0.5f));
}

//-----------------------------------------------------------------------------------------
void Channel::process(float *out1, long sampleFrames) {
	if (active) {
		float noteFreq = 440.0f * powf(2.0, (currentNote - 69) / 12.0)/*freqtab[currentNote & 0x7f]*/;

		float freq1 = noteFreq * program->freq1.getValue(samplesPlayed, relSample);	// not really linear...
		float freq2 = noteFreq * program->freq2.getValue(samplesPlayed, relSample);
		float vol = (float)(program->volume /** (double)currentVelocity*/ /** midiScaler*/);

		if (currentDelta >= 0)
		{
			out1 += currentDelta;
			sampleFrames -= currentDelta;
			currentDelta = 0;
		}

		float cut = program->filterCut.getValue(samplesPlayed, relSample)*8192;
		float res = program->filterRes.getValue(samplesPlayed, relSample);
		float f = (float) (2 * sin(3.1415927f * cut / 44100));
		float q = res;
		float scale = res;
		float dist = 1 + (program->distortion*100);

		// loop
		while (--sampleFrames >= 0)
		{
			float limit = 44100;

			float sample = getSample(program->waveform1, fPhase1);
			float vol1 = program->volume1.getValue(samplesPlayed, relSample) * vol;
			float sample2 = getSample(program->waveform2, fPhase2);
			float vol2 = program->volume2.getValue(samplesPlayed, relSample)* vol;

			sample = sample * vol1 + sample2 * vol2;

//			if (sample < 0.01 && released)
//				active = false;

			low = low + f * band;
			high = scale * sample - low - q * band;
			band = f * high + band;
			notch = high + low;

			if (program->filter < .20)
				sample = sample; // TODO: fix...
			else if (program->filter < 0.4)
				sample = low;
			else if (program->filter < 0.6)
				sample = high;
			else if (program->filter < 0.8)
				sample = band;
			else
				sample = notch;
			cut = program->filterCut.getValue(samplesPlayed, relSample)*8192;
			res = program->filterRes.getValue(samplesPlayed, relSample);

			f = (float) (2 * sinf(3.141592f * cut / 44100));
			q = res;
			scale = res;

			if (dist > 0.0) {
				sample *= dist;
				float clamp = vol1 > vol2 ? vol1 : vol2;
				sample = sample > clamp ? clamp : sample < -clamp ? -clamp : sample;
			}
			(*out1++) += sample;//wave1[(long)fPhase1 & mask] * fVolume1 * vol;
			fPhase1 += freq1;
			fPhase2 += freq2;

//			fPhase1 %= 44100;
//			fPhase2 %= 44100;
			while (fPhase1 > limit) fPhase1 -= limit;
			while (fPhase2 > limit) fPhase2 -= limit;

			samplesPlayed++;
			freq1 = noteFreq * program->freq1.getValue(samplesPlayed, relSample);
			freq2 = noteFreq * program->freq2.getValue(samplesPlayed, relSample);
		}
	}
}

//-----------------------------------------------------------------------------------------
void Channel::noteOn(long note, long velocity, long delta)
{
	if (program == NULL) {
		VstKarma::Debug("program == null");
		return;
	}
	fPhase1 = fPhase2 = 0;
	currentNote = note;
	currentVelocity = velocity;
	currentDelta = delta;
	released = false;
	active = true;
	samplesPlayed = 0;
	relSample = -1;
	high = band = low = notch = 0;
}

//-----------------------------------------------------------------------------------------
void Channel::noteOff() {
	released = true;
	relSample = samplesPlayed;
}