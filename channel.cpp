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
__inline int getSample(int wave, int phase, int wavelen) {
	int sample = (int) (phase >> 20);
	int pl = sample;
	pl += 2048;

	if (wave == 0) { // sin
		return sinf(sample / 2048.0f * 3.1415927f) * LIMIT;
	} else if (wave == 1) { // tri
		if (pl > 3072) {
			sample = (pl - 4096) * LIMIT;
		} else if (pl < 1024) {
			sample = pl * LIMIT;
		} else {
			sample = (2048 - pl) * LIMIT;
		}

		return sample >> 10;
	} else if (wave == 2) { // saw
		sample <<= 4;
		return sample;
	} else if (wave == 3) { // square
		return pl > wavelen ? LIMIT : -LIMIT;
	} else {
		return ((2 * ((rand() % 1024) - 512)) * LIMIT) >> 10;
	}
}

//-----------------------------------------------------------------------------------------
void Channel::process(int *left, int *right, long frames) {

	if (active || echoSamples > 0) {
		memset(&channelBufferL, 0, BUFFERSIZE * sizeof(int));
		memset(&channelBufferR, 0, BUFFERSIZE * sizeof(int));
	} else return;

	int *lBuf;
	int *rBuf;
	if (active) {
		if (program->echoDelay > 1024 && program->echoAmount > 256)
			echoSamples = program->echoDelay;

		for (int i = 0; i < playing_notes; i++) {
			lBuf = channelBufferL;
			rBuf = channelBufferR;

			int sampleFrames = frames;
			int lfo1phase = program->lfo1.phase;
			int lfo2phase = program->lfo2.phase;
			int lfo1 = (getSample(program->lfo1.waveform, lfo1phase, 2048) * program->lfo1.amount) >> 10;
			int lfo2 = (getSample(program->lfo2.waveform, lfo2phase, 2048) * program->lfo2.amount) >> 10;

			int lfo1rate = (0xffffffff/44100) * (program->lfo1.rate * 20);
			int lfo2rate = (0xffffffff/44100) * (program->lfo2.rate * 20);
			int freq1base = note[i].noteFreq * (1 + program->freq1);
			int freq1 = freq1base + lfo1;
			int freq2 = note[i].noteFreq * ((1 + program->freq2 + program->modEnvAmount * karma_ADSR_getValue(&program->modEnv, note[i].samplesPlayed, note[i].relSample) / 1024.0f)) + lfo1;

			if (note[i].delta > 0)
			{
				int sub = note[i].delta < frames ? note[i].delta : frames;
				
				sampleFrames -= sub;
				note[i].delta -= sub;
				lBuf += sub;
				rBuf += sub;
			}

			float cut = (program->cut + program->adsrAmount * karma_ADSR_getValue(&program->filterCut, note[i].samplesPlayed, note[i].relSample)/1024.0f)*8192 + lfo2 / 1024.0f;
			float res = program->resonance;
			float f = (float) (2 * sin(3.1415927f * cut / 44100));
			float q = res;
			float scale = res;
			int dist = 1024 + (program->distortion*20);
			int vol = 0;
			short realSample = 0;
			int sample;

			// loop
			while (--sampleFrames >= 0)
			{

				if (program->waveformMix < 50)
					sample = getSample(program->waveform1, note[i].phase1, program->wavelen1);
				else if (program->waveformMix > 1000)
					sample = getSample(program->waveform2, note[i].phase2, program->wavelen2);
				else {
					int tmp = 1024;
					tmp -= program->waveformMix;
					sample = getSample(program->waveform1, note[i].phase1, program->wavelen1);
					sample *= tmp;
					sample >>= 10;
					
					tmp = getSample(program->waveform2, note[i].phase2, program->wavelen2);
					tmp *= program->waveformMix;
					tmp >>= 10;
					sample += tmp;
				}

				if (freq1 < 0) freq1 = 0;
				if (freq2 < 0) freq2 = 0;


				if (program->filter >= 0.2) {
					float fsample = (float) (sample / 32767.0f);
//					if (cut < 0) cut = 0;
//					if (cut > 8192) cut = 8192;

					// note[i].low = note[i].low + f * note[i].band;
					note[i].low += f * note[i].band;
					note[i].high = scale * fsample - note[i].low - q * note[i].band;
					// note[i].band = f * note[i].high + note[i].band;
					note[i].band += f * note[i].high;

					note[i].notch = note[i].high;
					note[i].notch += note[i].low;

					if (program->filter < 0.4)
						fsample = note[i].low;
					else if (program->filter < 0.6)
						fsample = note[i].high;
					else if (program->filter < 0.8)
						fsample = note[i].band;
					else
						fsample = note[i].notch;

					if (program->adsrAmount > 50)
						cut = (program->cut + program->adsrAmount * karma_ADSR_getValue(&program->filterCut, note[i].samplesPlayed, note[i].relSample)/1024.0f)*8192 + lfo2 / 1024.0f;

					f = (float) (2 * sinf(3.141592f * cut / 44100));
//					q = res;
//					scale = res;

					fsample *= 32767;
					sample = (int) fsample;
				}


				if (dist > 1024) {
					sample *= dist;
					sample >>= 10;
					sample = sample > 32767 ? 32767 : sample < -32767 ? -32767 : sample;
				}

				vol = karma_ADSR_getValue(&program->amplifier, note[i].samplesPlayed, note[i].relSample);
				vol *= program->gain;
				vol >>= 10;

				sample *= vol;
				sample >>= 10;
				sample &= 0xffff;
				realSample = (short) (sample);

				sample = realSample;
				sample *= panl;
				sample >>= 10;
				*lBuf++ += sample;

				sample = realSample;
				sample *= panr;
				sample >>= 10;
				*rBuf++ += sample;

				note[i].phase1 += freq1;
				note[i].phase2 += freq2;

				note[i].samplesPlayed++;

				if (program->lfo1.amount > 50) {
					lfo1phase += lfo1rate;
					lfo1 = (getSample(program->lfo1.waveform, lfo1phase, 2048) * program->lfo1.amount) >> 10;
					freq1 = freq1base + lfo1;
				}
				if (program->lfo2.amount > 50) {
					lfo2phase += lfo2rate;
					lfo2 = (getSample(program->lfo2.waveform, lfo2phase, 2048) * program->lfo2.amount) >> 10;
				}

				if (program->waveformMix > 50)
					freq2 = note[i].noteFreq * (1 + program->freq2 + program->modEnvAmount * karma_ADSR_getValue(&program->modEnv, note[i].samplesPlayed, note[i].relSample) / 1024.0f) + lfo1;


				if (note[i].released && vol <= 0) {
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

	lBuf = channelBufferL;
	rBuf = channelBufferR;

	if (echoSamples > 0) {
		--echoSamples;
		if (leftEcho && rightEcho) {
			int tmp;
			for (int i = 0; i < frames; i++) {
				if (echoPos >= MAX_ECHO * 2)
					echoPos = 0;
				int j = echoPos - (program->echoDelay);

				if( j < 0 )
					j += MAX_ECHO * 2;

				leftEcho[ echoPos ] = *lBuf++;
				tmp = leftEcho[j];
				tmp *= program->echoAmount;
				tmp >>= 10;
				leftEcho[ echoPos ] += tmp;
				*left++ += leftEcho[ echoPos ];

				rightEcho[ echoPos ] = *rBuf++;
				tmp = rightEcho[j];
				tmp *= program->echoAmount;
				tmp >>= 10;
				rightEcho[ echoPos ] += tmp;
				*right++ += rightEcho[ echoPos ];
				++echoPos;
			}
		}
	} else {
		for (int i = 0; i < frames; i++) {
			*left++ += *lBuf++;
			*right++ += *rBuf++;
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

	int idx = playing_notes;

	if (playing_notes == MAX_NOTES) {
		int index[4];
		for (int i = 0; i < 4; i++) index[i] = -1;

		int kolen = 0;
		int snlen = 0;
		int len = 0;
		for (int i = 0; i < playing_notes; i++) {
			if (note[i].currentNote == notenum && note[i].released && (index[0] != -1)) {
				index[0] = i;
			} else if (note[i].released && note[i].samplesPlayed > kolen ) {
				index[1] = i;
				kolen = note[i].samplesPlayed;
			} else if (note[i].currentNote == notenum && note[i].samplesPlayed > snlen) {
				index[2] = i;
				snlen = note[i].samplesPlayed;
			} else if (note[i].samplesPlayed >= len) {
				index[3] = i;
				len = note[i].samplesPlayed;
			}
		}
		for (int i = 0; i < 4; i++) {
			if (index[i] != -1) {
				idx = index[i];
				break;
			}
		}
		playing_notes--;
	}

	if (idx == MAX_NOTES) {
		VstKarma::Debug("ERROR! Maximum notes!\n");
		return;
	}
/*
	char buf[256];
	sprintf(buf, "note: %d, %d, %d\n", notenum, delta, volume);
	VstKarma::Debug(buf);
*/

	note[idx].phase1 = note[idx].phase2 = 0;
	note[idx].released = false;
	note[idx].active = true;
	note[idx].currentNote = notenum;
	note[idx].noteFreq = (0xffffffff/44100) * (440.0f * powf(2.0, (note[idx].currentNote - 69) / 12.0));
	note[idx].samplesPlayed = 0;
	note[idx].relSample = 0;
	note[idx].delta = delta;
	note[idx].high = note[idx].band = note[idx].low = note[idx].notch = 0;

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
		case kWaveform1:	program->waveform1		= value*4;		break;
		case kFreq1:		program->freq1			= (value*2)-1;		break;
		case kFreq2:		program->freq2			= (value*2)-1;		break;
		case kWaveform2:	program->waveform2		= value*4;		break;
		case kModEnvA:		program->modEnv.attack		= (int)(value*44100*2);	break;
		case kModEnvD:		program->modEnv.decay		= (int)(value*44100*2);	break;
		case kModEnvAmount:	program->modEnvAmount		= (value*2)-1;		break;
		case kWaveformMix:	program->waveformMix		= (int) (value * 1024);	break;
		case kLFO1:		program->lfo1.waveform		= value*4;		break;
		case kLFO1amount:	program->lfo1.amount		= (int) (value * (1024.0f * 80.0f));	break;
		case kLFO1rate:		program->lfo1.rate		= value;		break;
		case kLFO2:		program->lfo2.waveform		= value*4;		break;
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
