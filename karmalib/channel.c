#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "channel.h"
// #include "vstkarma.h"
#include "param.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifdef __GNUC__
static int channelBufferL[BUFFERSIZE] __attribute__((aligned(32)));
static int channelBufferR[BUFFERSIZE] __attribute__((aligned(32)));
#else
static __declspec(align(32)) int channelBufferL[BUFFERSIZE];
static __declspec(align(32)) int channelBufferR[BUFFERSIZE];
#endif

static float freqtab[128];
//-----------------------------------------------------------------------------------------
void karma_Channel_init(karma_Channel *channel) {
	int i;
	double k = 1.059463094359;	// 12th root of 2
	double a = 6.875;	// a

	memset(channel, 0, sizeof(karma_Channel));
	channel->panl = channel->panr = (int) (sqrt(0.5) * 1024);
	channel->playing_notes = 0;
	channel->leftEcho = NULL;
	channel->rightEcho = NULL;
	channel->active = FALSE;
	channel->volume = 1024;

	for (i = 0; i < MAX_NOTES; i++) {
		channel->note[i].currentNote = -1;
		channel->note[i].released = TRUE;
		channel->note[i].active = FALSE;
		channel->note[i].samplesPlayed = 0;
		channel->note[i].relSample = 0;
		channel->note[i].high = channel->note[i].band = channel->note[i].low = channel->note[i].notch = 0;
	}
	karma_Program_init(&channel->program);

	a *= k;	// b
	a *= k;	// bb
	a *= k;	// c, frequency of midi note 0
	for (i = 0; i < 128; i++)	// 128 midi notes
	{
		freqtab[i] = (float)a;
		a *= k;
	}
}
//-----------------------------------------------------------------------------------------
void karma_Channel_free(karma_Channel *channel) {
	karma_MidiEvent *event = channel->event;

	while (event) {
		karma_MidiEvent *e = event;
		event = event->next;
		free(e);
	}

	if (channel->leftEcho)
		free(channel->leftEcho);
	if (channel->rightEcho)
		free(channel->rightEcho);
}

//-----------------------------------------------------------------------------------------
#define MIN_UPDATE_RATE 65536
void calculateVolumeUpdateRate(karma_Program *program, karma_Note * note) {
	int vol;
	int rate = MIN_UPDATE_RATE;

	if (note->released) {
		vol = program->amplifier.release;
		vol >>= 7;
	} else {
		vol = program->amplifier.attack > program->amplifier.decay ? program->amplifier.attack : program->amplifier.decay;
		vol >>= 7;
	}
	while (rate > vol) rate >>= 1;
	if (rate == 0) rate += 1;
	note->volumeUpdateRate = rate -1;
}
void calculateFreq2UpdateRate(karma_Program *program, karma_Note * note) {
//	note->freq2UpdateRate = 0;
//	return;

	int freq;
	int rate = MIN_UPDATE_RATE;

	if (program->waveformMix < 50 || fabs(program->modEnvAmount) < 0.2) {
		note->freq2UpdateRate = MIN_UPDATE_RATE-1;
		return;
	}

	if (note->released) {
		freq = program->modEnv.release;
		freq >>= 8;

	} else {

		freq = program->modEnv.attack > program->modEnv.decay ? program->modEnv.attack : program->modEnv.decay;
		freq >>= 8;
	}
	while (rate > freq) rate >>= 1;
	if (rate == 0) rate += 1;
	note->freq2UpdateRate = rate -1;
}
void calculateWaveformUpdateRate(karma_Program *program, karma_Note * note) {
/*
	int wf;
	if (program->waveformMix < 50) {
		// only waveform1
		2 * (440.0f * powf(2.0, (note[idx].currentNote - 69) / 12.0))
	} else if (program->waveformMix > 1000) {
		// only waveform2
	} else {
		// both waveforms;
	}
	while (rate > wf) rate >>= 1;
	if (rate == 0) rate += 1;
	note->waveformUpdateRate = rate -1;

	if (note->waveformUpdateRate > note->freq2UpdateRate)
		note->waveformUpdateRate = note->freq2UpdateRate;
*/
	note->waveformUpdateRate = 0;
}
void calculateLfo1UpdateRate(karma_Program *program, karma_Note *note) {
	if (program->lfo1.amount < 50) {
		note->lfo1UpdateRate = MIN_UPDATE_RATE-1;
		return;
	}

	// TODO: something better than this...
	note->lfo1UpdateRate = 0;
}
void calculateLfo2UpdateRate(karma_Program *program, karma_Note *note) {
	if (!program->filter || program->lfo2.amount < 50) {
		note->lfo2UpdateRate = MIN_UPDATE_RATE-1;
		return;
	}

	// TODO: something better than this...
	note->lfo2UpdateRate = 0;
}
void calculateUpdateRates(karma_Program *program, karma_Note *note) {
	calculateVolumeUpdateRate(program,note);
	calculateFreq2UpdateRate(program, note);
	calculateWaveformUpdateRate(program, note);
	calculateLfo1UpdateRate(program, note);
	calculateLfo2UpdateRate(program, note);
}

//-----------------------------------------------------------------------------------------
void karma_Channel_process(karma_Channel *channel, int *left, int *right, long length) {
	int *lBuf;
	int *rBuf;
	int i;
	int samplePos = 0;
	karma_Program *program = &channel->program;

	if (channel->active || channel->echoSamples > 0) {
		memset(&channelBufferL, 0, BUFFERSIZE * sizeof(int));
		memset(&channelBufferR, 0, BUFFERSIZE * sizeof(int));
	} else return;

	while (channel->active && samplePos < length) {
		int frames = length - samplePos;

		if (program->echoDelay > 1024 && program->echoAmount > 256)
			channel->echoSamples = program->echoDelay;

		if (channel->event && frames > channel->event->time)
			frames = channel->event->time;

		for (i = 0; i < channel->playing_notes; i++) {
			karma_Note *note = &channel->note[i];

			int sampleFrames = frames;
			int lfo1phase = program->lfo1.phase;
			int lfo2phase = program->lfo2.phase;
			int lfo1 = (program->lfo1.waveformTable[PHASE2TABLE(program->lfo1.waveform, lfo1phase, WAVETABLESIZE/2)] * program->lfo1.amount) >> 10;
			int lfo2 = (program->lfo2.waveformTable[PHASE2TABLE(program->lfo2.waveform, lfo2phase, WAVETABLESIZE/2)] * program->lfo2.amount) >> 10;

			int lfo1rate = (int) ((0xffffffff/44100) * (program->lfo1.rate * 20));
			int lfo2rate = (int) ((0xffffffff/44100) * (program->lfo2.rate * 20));
			int freq1base = note->noteFreq * (1 + program->freq1);
			int freq1 = freq1base + lfo1;
			int freq2base = (int) (note->noteFreq * ((1 + program->freq2 + program->modEnvAmount * karma_ADSR_getValue(&program->modEnv, note->samplesPlayed, note->relSample) / 1024.0f)));
			int freq2 = freq2base + lfo1;

			float cut = (program->cut + program->adsrAmount * karma_ADSR_getValue(&program->filterCut, note->samplesPlayed, note->relSample)/1024.0f)*8192 + lfo2 / 1024.0f;
			float res = program->resonance;
			float f = (float) (2 * sin(3.1415927f * cut / 44100));
			float q = res;
			float scale = res;
			int dist = 1024 + (program->distortion*20);
			short realSample = 0;
			int sample = (((1024 - program->waveformMix) * program->waveform1Table[PHASE2TABLE(program->waveform1, note->phase1, program->wavelen1)]) >> 10) + ((program->waveform2Table[PHASE2TABLE(program->waveform2, note->phase2, program->wavelen2)] * program->waveformMix) >> 10);

			int vol = (((karma_ADSR_getValue(&program->amplifier, note->samplesPlayed, note->relSample) * program->gain) >> 10) * channel->volume) >> 10;
			int *wf1 = program->waveform1Table;
			int *wf2 = program->waveform2Table;

			lBuf = &channelBufferL[samplePos];
			rBuf = &channelBufferR[samplePos];

			if (note->delta > 0)
			{
				int sub = note->delta < frames ? note->delta : frames;
				
				sampleFrames -= sub;
				note->delta -= sub;
				lBuf += sub;
				rBuf += sub;
			}

			// loop
			while (--sampleFrames >= 0)
			{

				if (freq1 < 0) freq1 = 0;
				if (freq2 < 0) freq2 = 0;


				if (program->filter) {
					float fsample = (float) (sample / 32767.0f);
//					if (cut < 0) cut = 0;
//					if (cut > 8192) cut = 8192;

					note->low += f * note->band;
					note->high = scale * fsample - note->low - q * note->band;
					note->band += f * note->high;
					note->notch = note->high + note->low;

					switch (program->filter) {
						default:
						case 1:	fsample = note->low; break;
						case 2: fsample = note->high; break;
						case 3: fsample = note->band; break;
						case 4: fsample = note->notch; break;
					}

					if (program->adsrAmount > 50) {
						cut = (program->cut + program->adsrAmount * karma_ADSR_getValue(&program->filterCut, note->samplesPlayed, note->relSample)/1024.0f)*8192 + lfo2 / 1024.0f;
					}
					f = (float) (2 * sin(3.141592f * cut / 44100));

					fsample *= 32767;
					sample = (int) fsample;
				}


				if (dist > 1024) {
					sample = (sample * dist) >> 10;
					if (sample > 32767) sample = 32767;
					else if (sample < -32767) sample = -32767;
				}

				realSample = (short) (((sample * vol) >> 10)&0xffff);

				*lBuf++ += (realSample * channel->panl) >> 10;
				*rBuf++ += (realSample * channel->panr) >> 10;

				note->phase1 += freq1;
				note->phase2 += freq2;

				lfo1phase += lfo1rate;
				lfo2phase += lfo2rate;

				note->samplesPlayed++;

				if ((note->samplesPlayed & note->volumeUpdateRate) == note->volumeUpdateRate) {
					vol = karma_ADSR_getValue(&program->amplifier, note->samplesPlayed, note->relSample);
					if (!note->released && vol == program->amplifier.sustain && note->samplesPlayed > program->amplifier.attack + program->amplifier.decay) {
						// the volume adsr envelope is sustained and will not update anymore
						note->volumeUpdateRate = MIN_UPDATE_RATE - 1;
					}
					vol *= program->gain;
					vol >>= 10;
					vol *= channel->volume;
					vol >>= 10;
		
				}

				if ((note->samplesPlayed & note->lfo1UpdateRate) == note->lfo1UpdateRate) {
					lfo1 = (program->lfo1.waveformTable[PHASE2TABLE(program->lfo1.waveform, lfo1phase, WAVETABLESIZE/2)] * program->lfo1.amount) >> 10;
					freq1 = freq1base + lfo1;
					freq2 = freq2base + lfo1;
				}
				if ((note->samplesPlayed & note->lfo2UpdateRate) == note->lfo2UpdateRate) {
					lfo2 = (program->lfo2.waveformTable[PHASE2TABLE(program->lfo2.waveform, lfo2phase, WAVETABLESIZE/2)] * program->lfo1.amount) >> 10;
				}


				if ((note->samplesPlayed & note->freq2UpdateRate) == note->freq2UpdateRate) {
					if (!note->released && note->samplesPlayed > program->modEnv.attack + program->modEnv.decay) {
						// the mod adsr envelope is sustained and will not update anymore
						note->freq2UpdateRate = MIN_UPDATE_RATE - 1;
					}

					freq2base = note->noteFreq * (1 + program->freq2 + program->modEnvAmount * karma_ADSR_getValue(&program->modEnv,note->samplesPlayed, note->relSample) / 1024.0f);
					freq2 = freq2base + lfo1;
				}

				if ((note->samplesPlayed & note->waveformUpdateRate) == note->waveformUpdateRate) {
					if (program->waveformMix < 50)
						sample = wf1[PHASE2TABLE(program->waveform1, note->phase1, program->wavelen1)];
					else if (program->waveformMix > 1000)
						sample = wf2[PHASE2TABLE(program->waveform2, note->phase2, program->wavelen2)];
					else {
						int tmp = 1024 - program->waveformMix;
						sample = (wf1[PHASE2TABLE(program->waveform1, note->phase1, program->wavelen1)] * tmp) >> 10;
						
						tmp = (wf2[PHASE2TABLE(program->waveform2, note->phase2, program->wavelen2)] * program->waveformMix) >> 10;
						sample += tmp;
					}
				}


				if (note->released && vol <= 0) {
					// this notes volume is too low to hear and the note has been released
					// so remove it from the playing notes
					note->active = FALSE;
					channel->playing_notes--;

					if (i != channel->playing_notes) {
						// there are notes after this note that needs to be moved "up"
						// just copy the last note into this space.
						// the note in this place needs to be played again so i--
						memcpy(&channel->note[i], &channel->note[channel->playing_notes], sizeof(karma_Note));
						i--;
					}

					if (channel->playing_notes == 0) {// no notes playing in the channel
						channel->active = FALSE;
					}
					break;
				}
			}

		}

		program->lfo1.phase += (int) (frames * (0xffffffff/44100) * program->lfo1.rate*20);
		program->lfo2.phase += (int) (frames * (0xffffffff/44100) * program->lfo2.rate*20);

		if (channel->event) {
			int num = 0;
			karma_MidiEvent *e = channel->event;
			karma_MidiEvent *last = NULL;
			karma_MidiEvent *first = NULL;
			while (e) {
				e->time -= frames;
				if (e->time <= 0) {
					karma_MidiEvent *tmp = e;
					karma_Channel_processEvent(channel, e);
					e = tmp->next;
					free(tmp);
					if (last) {
						last->next = e;
					}
					num++;
				} else {
					if (!first) first = e;
					last = e;
					e = e->next;
				}
			}
			channel->event = first;
		}
		samplePos += frames;
	}

	lBuf = channelBufferL;
	rBuf = channelBufferR;

	if (channel->echoSamples > 0) {

		--channel->echoSamples;
		if (channel->leftEcho && channel->rightEcho) {
			int tmp;
			int j;

			for (i = 0; i < length; i++) {
				if (channel->echoPos >= MAX_ECHO * 2)
					channel->echoPos = 0;

				j = channel->echoPos - (program->echoDelay);

				if( j < 0 )
					j += MAX_ECHO * 2;

				channel->leftEcho[ channel->echoPos ] = *lBuf++;
				tmp = channel->leftEcho[j];
				tmp *= program->echoAmount;
				tmp >>= 10;
				channel->leftEcho[ channel->echoPos ] += tmp;
				left[i] += channel->leftEcho[ channel->echoPos ];

				channel->rightEcho[ channel->echoPos ] = *rBuf++;
				tmp = channel->rightEcho[j];
				tmp *= program->echoAmount;
				tmp >>= 10;
				channel->rightEcho[ channel->echoPos ] += tmp;
				right[i] += channel->rightEcho[ channel->echoPos ];
				++channel->echoPos;
			}
		}
	} else {
		for (i = 0; i < length; i++) {
			left[i] += *lBuf++;
			right[i] += *rBuf++;
		}
	}
}
void debug(char *str) {
	FILE *f = NULL;
	if ((f = fopen("c:\\karma.log", "a")) != NULL) {
		fprintf(f, str);
		fclose(f);
	}
}
//-----------------------------------------------------------------------------------------
void karma_Channel_noteOn(karma_Channel *channel, long notenum, long velocity, long delta)
{
	int idx = channel->playing_notes;
//	char buf[128];

//	sprintf(buf, "notes: %d\n", channel->playing_notes);
//	debug(buf);
	if (channel->playing_notes == MAX_NOTES) {
		int i;
		int kolen = 0;
		int snlen = 0;
		int len = 0;

		int index[4];
		for (i = 0; i < 4; i++) index[i] = -1;

		for (i = 0; i < channel->playing_notes; i++) {
			if (channel->note[i].currentNote == notenum && channel->note[i].released && (index[0] != -1)) {
				index[0] = i;
			} else if (channel->note[i].released && channel->note[i].samplesPlayed > kolen ) {
				index[1] = i;
				kolen = channel->note[i].samplesPlayed;
			} else if (channel->note[i].currentNote == notenum && channel->note[i].samplesPlayed > snlen) {
				index[2] = i;
				snlen = channel->note[i].samplesPlayed;
			} else if (channel->note[i].samplesPlayed >= len) {
				index[3] = i;
				len = channel->note[i].samplesPlayed;
			}
		}
		for (i = 0; i < 4; i++) {
			if (index[i] != -1) {
				idx = index[i];
				break;
			}
		}
		channel->playing_notes--;
	}

	if (idx == MAX_NOTES) {
		debug("error max notes\n");
		return;
	}

	memset(&channel->note[idx], 0, sizeof(karma_Note));
	channel->note[idx].active = TRUE;
	channel->note[idx].currentNote = notenum;
	channel->note[idx].noteFreq = (0xffffffff/44100) * freqtab[channel->note[idx].currentNote]; //(440.0f * pow(2.0, (channel->note[idx].currentNote - 69) / 12.0));
	channel->note[idx].delta = delta;

	calculateUpdateRates(&channel->program, &channel->note[idx]);
	channel->playing_notes++;

	if (channel->program.echoDelay > 1024 && !channel->leftEcho) {
		channel->leftEcho = (int*) malloc(MAX_ECHO * 2 * sizeof(int));
		channel->rightEcho = (int*) malloc(MAX_ECHO * 2 * sizeof(int));
	}
	channel->active = TRUE;
}

//-----------------------------------------------------------------------------------------
void karma_Channel_noteOff(karma_Channel *channel, long notenum) {
	int i;
	int idx = -1;
	int lastSamples = 0;
	for (i = 0; i < channel->playing_notes; i++) {
		if (channel->note[i].currentNote == notenum && !channel->note[i].released && channel->note[i].samplesPlayed > lastSamples) {
			idx = i; 
			lastSamples = channel->note[i].samplesPlayed;
		}
	}
	if (idx != -1) {
		channel->note[idx].released = TRUE;
		channel->note[idx].relSample = channel->note[idx].samplesPlayed;
		calculateUpdateRates(&channel->program, &channel->note[idx]);
	}
}

//-----------------------------------------------------------------------------------------
void karma_Channel_allNotesOff(karma_Channel *channel) {
	int i;
	for (i = 0; i < channel->playing_notes; i++) {
		channel->note[i].released = TRUE;
		channel->note[i].relSample = channel->note[i].samplesPlayed;
		calculateUpdateRates(&channel->program, &channel->note[i]);
	}
}

//-----------------------------------------------------------------------------------------
void karma_Channel_processEvent(karma_Channel *channel, karma_MidiEvent *event) {
	long cmd = event->data[0] & 0xf0;

	if (cmd == 0x80) {	// note off
		long note = event->data[1] & 0x7f;
		karma_Channel_noteOff(channel, note);
	} else if (cmd == 0xb0)	{// Channel Mode Messages
		 if (event->data[1] == 7) { // volume change
			 channel->volume = ((event->data[2]&0x7f) / 127.0f) * 1024;
		} else if (event->data[1] == 10) { // pan change
			float value = (event->data[2]&0x7f) / 127.0f;
			channel->panl = sqrt(1.0f-value) * 1024;
			channel->panl = sqrt(value) * 1024;
		} else if (event->data[1] == 74) { // cut off
			karma_Program_setParameter(&channel->program, kFilterCut, (event->data[2]&0x7f) / 127.0f);
		} else if (event->data[1] >= 12) {
			karma_Program_setParameter(&channel->program, event->data[1] - 12, (event->data[2]&0x7f) / 127.0f);
		}
	}
}

//-----------------------------------------------------------------------------------------
static void addEvent(karma_Channel *channel, karma_MidiEvent *e) {

	if (!channel->event) {
		// first event in list
		channel->event = e;
	} else {
		karma_MidiEvent *tmp = channel->event;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = e;
	}
}
//-----------------------------------------------------------------------------------------
void karma_Channel_addEvent(karma_Channel *channel, karma_MidiEvent *event) {
	long cmd = event->data[0] & 0xf0;

	if (cmd == 0x90)	{	// note on
		long note = event->data[1] & 0x7f;
		long velocity = event->data[2] & 0x7f;
		if (velocity == 0) {
			event->data[0] = 0x80 | (event->data[0]&0xf);
			addEvent(channel, event);
		} else 
			karma_Channel_noteOn(channel, note, velocity, event->time);
	} else if (!event->time) {
		karma_Channel_processEvent(channel, event);
	} else {
		addEvent(channel, event);
	}
}
//-----------------------------------------------------------------------------------------
