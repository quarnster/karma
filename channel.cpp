#include "Channel.h"
#include "vstkarma.h"
#include "param.h"
#include <math.h>
#include <stdlib.h>

#ifndef NULL
#define NULL 0
#endif


int channelBufferL[BUFFERSIZE];
int channelBufferR[BUFFERSIZE];

//-----------------------------------------------------------------------------------------
Channel::Channel() {
	volume = 1024;
	panl = panr = sqrt(0.5) * 1024;
	program = NULL;
	playing_notes = 0;
	leftEcho = rightEcho = NULL;

	for (int i = 0; i < MAX_NOTES; i++) {
		note[i].currentNote = -1;
		note[i].released = true;
		note[i].active = active = false;
		note[i].samplesPlayed = 0;
		note[i].relSample = 0;
		note[i].high = note[i].band = note[i].low = note[i].notch = 0;
	}
}
//-----------------------------------------------------------------------------------------
Channel::~Channel() {
	if (leftEcho)
		delete[] leftEcho;
	if (rightEcho)
		delete[] rightEcho;
}

#define LIMIT 32767
//-----------------------------------------------------------------------------------------
inline int getSample(float wave, int phase) {
	int sample = (int) (phase >> 20);
	int pl = sample + 2048;

	if (wave < 0.2) {				// sin
		return sinf(sample / 2048.0f * 3.1415927f) * LIMIT;
	}
	else if (wave < 0.4) {				// tri
		if (pl > 3072) {
			sample = (pl - 4096) * LIMIT;
		} else if (pl < 1024) {
			sample = pl * LIMIT;
		} else {
			sample = (2048 - pl) * LIMIT;
		}

		return sample >> 10;
	}
	else if (wave < 0.6)				// saw
		return sample << 4;
	else if (wave < 0.8)				// square
		return sample > 0 ? LIMIT : -LIMIT;
	else						// noise
		return ((2 * ((rand() % 1024) - 512)) * LIMIT) >> 10;
}

//-----------------------------------------------------------------------------------------
void Channel::process(int *left, int *right, long frames) {

	memset(&channelBufferL, 0, BUFFERSIZE * sizeof(int));
	memset(&channelBufferR, 0, BUFFERSIZE * sizeof(int));

	if (active) {
		if (program->echoDelay > 1024 && program->echoAmount > 256)
			echoSamples = program->echoDelay;

		for (int i = 0; i < playing_notes; i++) {
			unsigned int pos = 0;
			long sampleFrames = frames;
			int noteFreq = (0xffffffff/44100) * (440.0f * powf(2.0, (note[i].currentNote - 69) / 12.0));
			int lfo1phase = program->lfo1.phase;
			int lfo2phase = program->lfo2.phase;
			int lfo1 = (getSample(program->lfo1.waveform, lfo1phase) * program->lfo1.amount) >> 10;
			int lfo2 = (getSample(program->lfo2.waveform, lfo2phase) * program->lfo2.amount) >> 10;

			int lfo1rate = (0xffffffff/44100) * (program->lfo1.rate * 20);
			int lfo2rate = (0xffffffff/44100) * (program->lfo2.rate * 20);
			int freq1base = noteFreq * (1 + program->freq1);
			int freq1 = freq1base + lfo1;
			int freq2 = noteFreq * ((1 + program->freq2 + program->modEnvAmount * program->modEnv.getValue(note[i].samplesPlayed, note[i].relSample) / 1024.0f)) + lfo1;

			if (note[i].delta > 0)
			{
				int sub = note[i].delta < frames ? note[i].delta : frames;
				
				sampleFrames -= sub;
				note[i].delta -= sub;
				pos += sub;
			}
			long numFrames = sampleFrames;

			float cut = (program->cut + program->adsrAmount * program->filterCut.getValue(note[i].samplesPlayed, note[i].relSample)/1024.0f)*8192 + lfo2 / 1024.0f;
			float res = program->resonance;
			float f = (float) (2 * sin(3.1415927f * cut / 44100));
			float q = res;
			float scale = res;
			int dist = 1024 + (program->distortion*20);

			// loop
			while (--sampleFrames >= 0)
			{
				int sample;
				if (program->waveformMix > 1000)
					sample = getSample(program->waveform2, note[i].phase2);
				else if (program->waveformMix < 50)
					sample = getSample(program->waveform1, note[i].phase1);
				else
					sample = ((getSample(program->waveform1, note[i].phase1) * (1024 - program->waveformMix)) >> 10) + ((getSample(program->waveform2, note[i].phase2) * program->waveformMix) >> 10);

				if (freq1 < 0) freq1 = 0;
				if (freq2 < 0) freq2 = 0;

				if (program->filter >= 0.2) {
					float fsample = (float) (sample / 32767.0f);
//					if (cut < 0) cut = 0;
//					if (cut > 8192) cut = 8192;

					note[i].low = note[i].low + f * note[i].band;
					note[i].high = scale * fsample - note[i].low - q * note[i].band;
					note[i].band = f * note[i].high + note[i].band;
					note[i].notch = note[i].high + note[i].low;

					if (program->filter < 0.4)
						fsample = note[i].low;
					else if (program->filter < 0.6)
						fsample = note[i].high;
					else if (program->filter < 0.8)
						fsample = note[i].band;
					else
						fsample = note[i].notch;
					cut = (program->cut + program->adsrAmount * program->filterCut.getValue(note[i].samplesPlayed, note[i].relSample)/1024.0f)*8192 + lfo2 / 1024.0f;

					f = (float) (2 * sinf(3.141592f * cut / 44100));
					q = res;
					scale = res;
					sample = fsample * 32767.0f;
				}

				if (dist > 1024) {
					sample = (sample * dist) >> 10;
					sample = sample > 32767 ? 32767 : sample < -32767 ? -32767 : sample;
				}

				int vol = (((program->amplifier.getValue(note[i].samplesPlayed, note[i].relSample) * program->gain) >> 10) * volume) >> 10;
				short realSample = (short) (((sample * vol) >> 10)&0xffff);

				channelBufferL[pos] += (realSample * panl) >> 10;
				channelBufferR[pos] += (realSample * panr) >> 10;
				pos++;

				// left = sample*sqrt(1.0-panning);
				// right= sample*sqrt(panning);


				note[i].phase1 += freq1;
				note[i].phase2 += freq2;

				note[i].samplesPlayed++;

				if (program->lfo1.amount > 50) {
					lfo1phase += lfo1rate;
					lfo1 = (getSample(program->lfo1.waveform, lfo1phase) * program->lfo1.amount) >> 10;
					freq1 = freq1base + lfo1;
				}
				if (program->lfo2.amount > 50) {
					lfo2phase += lfo2rate;
					lfo2 = (getSample(program->lfo2.waveform, lfo2phase) * program->lfo2.amount) >> 10;
				}

				if (program->waveformMix > 50)
					freq2 = noteFreq * (1 + program->freq2 + program->modEnvAmount * program->modEnv.getValue(note[i].samplesPlayed, note[i].relSample) / 1024.0f) + lfo1;


				if (note[i].released && vol == 0) {
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

					if (playing_notes == 0) {// no notes playing in the channel
						active = false;
					}
					break;
				}
			}

		}

		program->lfo1.phase += frames * (0xffffffff/44100) * program->lfo1.rate*20;
		program->lfo2.phase += frames * (0xffffffff/44100) * program->lfo2.rate*20;
	}

	if (echoSamples > 0) {
		echoSamples--;
		if (leftEcho && rightEcho) {
			for (int i = 0; i < frames; i++) {
				if (echoPos >= MAX_ECHO * 2)
					echoPos = 0;
				int j = echoPos - (program->echoDelay);

				if( j < 0 )
					j += MAX_ECHO * 2;

				left[i] += leftEcho[ echoPos ] = channelBufferL[i] + ((leftEcho[j] * program->echoAmount) >> 10);
				right[i] += rightEcho[ echoPos ] = channelBufferR[i] + ((rightEcho[j] * program->echoAmount) >> 10);
				echoPos++;
			}
		}
	} else if (active) {
		for (int i = 0; i < frames; i++) {
			left[i] += channelBufferL[i];
			right[i] += channelBufferR[i];
		}
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
/*
	for (int i = 0; i < playing_notes; i++) {
		if (note[i].currentNote == notenum && note[i].volume == volume) {
			note[i].delta = delta;
			note[i].phase1 = note[i].phase2 = 0;
			note[i].released = false;
			note[i].active = true;
			note[i].currentNote = notenum;
			note[i].samplesPlayed = 0;
			note[i].relSample = -1;
			note[i].delta = delta;
			note[i].high = note[i].band = note[i].low = note[i].notch = 0;
			note[i].volume = volume;
			return;
		}
	}
*/
/*
	char buf[256];
	sprintf(buf, "note: %d, %d, %d\n", notenum, delta, volume);
	VstKarma::Debug(buf);
*/

	note[playing_notes].phase1 = note[playing_notes].phase2 = 0;
	note[playing_notes].released = false;
	note[playing_notes].active = true;
	note[playing_notes].currentNote = notenum;
	note[playing_notes].samplesPlayed = 0;
	note[playing_notes].relSample = 0;
	note[playing_notes].delta = delta;
	note[playing_notes].high = note[playing_notes].band = note[playing_notes].low = note[playing_notes].notch = 0;

	playing_notes++;

	if (program->echoDelay > 1024 && !leftEcho) {
		leftEcho = new int[MAX_ECHO*2];
		rightEcho = new int[MAX_ECHO*2];
	}
//	program->lfo1.phase = 0;
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
//-----------------------------------------------------------------------------------------
void Channel::setParameter(int index, int param) {
	if (program == NULL) {
		VstKarma::Debug("program == null\n");
		return;
	}

	float value = param / 127.0f;
	switch (index) {
		case kWaveform1:	program->waveform1		= value;		break;
		case kFreq1:		program->freq1			= (value*2)-1;		break;
		case kFreq2:		program->freq2			= (value*2)-1;		break;
		case kWaveform2:	program->waveform2		= value;		break;
		case kModEnvA:		program->modEnv.attack		= (int)(value*44100*2);	break;
		case kModEnvD:		program->modEnv.decay		= (int)(value*44100*2);	break;
		case kModEnvAmount:	program->modEnvAmount		= (value*2)-1;		break;
		case kWaveformMix:	program->waveformMix		= (int) (value * 1024);	break;
		case kLFO1:		program->lfo1.waveform		= value;		break;
		case kLFO1amount:	program->lfo1.amount		= (int) (value * (1024.0f * 80.0f));	break;
		case kLFO1rate:		program->lfo1.rate		= value;		break;
		case kLFO2:		program->lfo2.waveform		= value;		break;
		case kLFO2amount:	program->lfo2.amount		= (int) (value * (1024.0f * 1024.0f));	break;
		case kLFO2rate:		program->lfo2.rate		= value;		break;
		case kFilterType:	program->filter			= value;		break;
		case kFilterRes:	program->resonance		= value;		break;
		case kFilterCut:	program->cut			= value;		break;
		case kFilterADSRAmount:	program->adsrAmount		= value;		break;
		case kFilterCutA:	program->filterCut.attack	= (int)(value*44100*2);	break;
		case kFilterCutD:	program->filterCut.decay	= (int)(value*44100*2);	break;
		case kFilterCutS:	program->filterCut.sustain	= (int)(value*1024);	break;
		case kFilterCutR:	program->filterCut.release	= (int)(value*44100*2);	break;
		case kDistortion:	program->distortion		= (int) (value*1024);	break;
		case kAmplifierA:	program->amplifier.attack	= (int)(value*44100*2);	break;
		case kAmplifierD:	program->amplifier.decay	= (int)(value*44100*2);	break;
		case kAmplifierS:	program->amplifier.sustain	= (int)(value*1024);	break;
		case kAmplifierR:	program->amplifier.release	= (int)(value*44100*2);	break;
		case kGain:		program->gain			= (int)(value*1024);	break;
		case kEchoDelay:	program->echoDelay		= (int)(value*MAX_ECHO);	break;
		case kEchoAmount:	program->echoAmount		= (int)(value*1024);	break;

		case kPan:
			panl = sqrt(1.0-value) * 1024;
			panr = sqrt(value) * 1024;
			break;
	}
}
