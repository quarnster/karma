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
	playing_notes = 0;

	for (int i = 0; i < MAX_NOTES; i++) {
		note[i].currentNote = -1;
		note[i].released = true;
		note[i].active = active = false;
		note[i].samplesPlayed = 0;
		note[i].relSample = -1;
		note[i].high = note[i].band = note[i].low = note[i].notch = 0;
	}
//	high = band = low = notch = 0;
}

//-----------------------------------------------------------------------------------------
inline float getSample(float wave, float phase) {
	float limit = 44100;
	float pl = phase / limit;
	float sample = -1 + (2 * (pl));
	if (wave < 0.2) {				// sin
		return sinf(sample * 3.1415927f);
	}
	else if (wave < 0.4) {			// tri
		if (pl > 0.75) {
			sample = -1 + (pl - 0.75) * 4;
		} else if (pl < 0.25) {
			sample = pl * 4;
		} else {
			sample = 1 - (pl - 0.25) * 4;
		}
		return sample;
	}
	else if (wave < 0.6)				// saw
		return sample;
	else if (wave < 0.8)				// square
		return sample > 0 ? 1.0f : -1.0f;
	else						// noise
		return (float) (2 * (((rand() % 1000) / 1000.0f)- 0.5f));
}

//-----------------------------------------------------------------------------------------
void Channel::process(float *out1, long frames) {
	if (active) {
		for (int i = 0; i < playing_notes; i++) {
			unsigned int pos = 0;
			long sampleFrames = frames;
			float noteFreq = 440.0f * powf(2.0, (note[i].currentNote - 69) / 12.0)/*freqtab[currentNote & 0x7f]*/;
			float lfo1phase = program->lfo1.fPhase;
			float lfo2phase = program->lfo2.fPhase;
			float lfo1 = getSample(program->lfo1.waveform, lfo1phase) * program->lfo1.amount * 80;
			float lfo2 = getSample(program->lfo2.waveform, lfo2phase) * program->lfo2.amount * 1024;

			float freq1 = noteFreq * (1 + program->freq1) + lfo1; //.getValue(note[i].samplesPlayed, note[i].relSample) + lfo1;	// not really linear...
			float freq2 = noteFreq * (1 + program->freq2 + program->modEnvAmount * program->modEnv.getValue(note[i].samplesPlayed, note[i].relSample)) + lfo1;
	//		float gain = (float)(program->gain /** (double)currentVelocity*/ /** midiScaler*/);

			if (note[i].delta >= 0)
			{
				pos += note[i].delta;
				sampleFrames -= note[i].delta;
				note[i].delta = 0;
			}
			long numFrames = sampleFrames;

			float cut = (program->cut + program->adsrAmount * program->filterCut.getValue(note[i].samplesPlayed, note[i].relSample))*8192 + lfo2;
			float res = program->resonance; //program->filterRes.getValue(note[i].samplesPlayed, note[i].relSample);
			float f = (float) (2 * sin(3.1415927f * cut / 44100));
			float q = res;
			float scale = res;
			float dist = 1 + (program->distortion*20);

			// loop
			while (--sampleFrames >= 0)
			{
				float limit = 44100;

				float sample = getSample(program->waveform1, note[i].fPhase1);
				float sample2 = getSample(program->waveform2, note[i].fPhase2);

				sample = sample * (1-program->waveformMix) + sample2 * program->waveformMix;

				if (freq1 < 0) freq1 = 0;
				if (freq2 < 0) freq2 = 0;
				if (program->filter >= 0.2) {
					if (cut < 0) cut = 0;
					if (cut > 8192) cut = 8192;

					note[i].low = note[i].low + f * note[i].band;
					note[i].high = scale * sample - note[i].low - q * note[i].band;
					note[i].band = f * note[i].high + note[i].band;
					note[i].notch = note[i].high + note[i].low;

					if (program->filter < 0.4)
						sample = note[i].low;
					else if (program->filter < 0.6)
						sample = note[i].high;
					else if (program->filter < 0.8)
						sample = note[i].band;
					else
						sample = note[i].notch;
					cut = (program->cut + program->adsrAmount * program->filterCut.getValue(note[i].samplesPlayed, note[i].relSample))*8192 + lfo2;
	//				res = program->filterRes.getValue(note[i].samplesPlayed, note[i].relSample);

					f = (float) (2 * sinf(3.141592f * cut / 44100));
					q = res;
					scale = res;
				}

				if (dist > 1.0) {
					sample *= dist;
					float clamp = 1;
					sample = sample > clamp ? clamp : sample < -clamp ? -clamp : sample;
				}

				float vol = program->amplifier.getValue(note[i].samplesPlayed, note[i].relSample) * program->gain;
				sample *= vol;
				out1[pos++] += sample;

				note[i].fPhase1 += freq1;
				note[i].fPhase2 += freq2;

				// TODO: fix these two
				lfo1phase += program->lfo1.rate*20;
				lfo2phase += program->lfo2.rate*20;

				while (note[i].fPhase1 > limit) note[i].fPhase1 -= limit;
				while (note[i].fPhase2 > limit) note[i].fPhase2 -= limit;
				while (lfo1phase > limit) lfo1phase -= limit;
				while (lfo2phase > limit) lfo2phase -= limit;

				note[i].samplesPlayed++;
				lfo1 = getSample(program->lfo1.waveform, program->lfo1.fPhase) * program->lfo1.amount * 80;
				lfo2 = getSample(program->lfo2.waveform, program->lfo2.fPhase) * program->lfo2.amount * 1024;
				freq1 = noteFreq * (1 + program->freq1) + lfo1; //.getValue(note[i].samplesPlayed, note[i].relSample) + lfo1;
				freq2 = noteFreq * (1 + program->freq2 + program->modEnvAmount * program->modEnv.getValue(note[i].samplesPlayed, note[i].relSample)) + lfo1;


				if (vol == 0 && note[i].released) {
					// this notes volume is too low to hear and the note has been released
					// so remove it from the playing notes
					note[i].active = false;
					playing_notes--;

					if (i != playing_notes) {
						// there are notes after this note that needs to be moved "up"
						// just copy the last note into this space.
						// the note in this place needs to be played again so i--
						memcpy(&note[i], &note[playing_notes], sizeof(karma_Note));
						i--;
					}

					if (playing_notes == 0) // no notes playing in the channel
						active = false;
					break;
				}
			}
		}
		program->lfo1.fPhase += frames * program->lfo1.rate*20;
		program->lfo2.fPhase += frames * program->lfo2.rate*20;

		while (program->lfo1.fPhase > 44100) program->lfo1.fPhase -= 44100;
		while (program->lfo2.fPhase > 44100) program->lfo2.fPhase -= 44100;

	}
}

//-----------------------------------------------------------------------------------------
void Channel::noteOn(long notenum, long velocity, long delta)
{
	if (program == NULL) {
		VstKarma::Debug("program == null\n");
		return;
	}

	if (playing_notes == MAX_NOTES) {
		VstKarma::Debug("Maximum number of notes allready playing\n");
		return;
	}

	note[playing_notes].fPhase1 = note[playing_notes].fPhase2 = 0;
	note[playing_notes].released = false;
	note[playing_notes].active = true;
	note[playing_notes].currentNote = notenum;
	note[playing_notes].samplesPlayed = 0;
	note[playing_notes].relSample = -1;
	note[playing_notes].delta = delta;
	note[playing_notes].high = note[playing_notes].band = note[playing_notes].low = note[playing_notes].notch = 0;

	playing_notes++;

	program->lfo1.fPhase = 0;
	currentVelocity = velocity;
//	currentDelta = delta;
	active = true;
	
}

//-----------------------------------------------------------------------------------------
void Channel::noteOff(long notenum) {
	for (int i = 0; i < playing_notes; i++) {
		if (note[i].currentNote == notenum && !note[i].released) {
			note[i].released = true;
			note[i].relSample = note[i].samplesPlayed;
			break;
		}
	}
}