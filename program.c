#include "program.h"
#include "param.h"

void karma_Program_init(karma_Program *program) {
	memset(program, 0, sizeof(karma_Program));
	program->amplifier.sustain = 512;
	program->gain = 512;
	program->waveform1 = program->waveform2 = 0;
	program->wavelen1 = program->wavelen2 =  0.5 * 4096;
	program->waveform1Table = program->waveform2Table = program->lfo1.waveformTable = program->lfo2.waveformTable = sineTable;
}

float karma_Program_getParameter(karma_Program *program, long index) {
	float value = 0;
	switch (index)
	{
		case kWaveform1:	value = (program->waveform1 / 4.0f);		break;
		case kFreq1:		value = (program->freq1/2.0)+0.5f;	break;
		case kFreq2:		value = (program->freq2/2.0)+0.5f;	break;
		case kWaveLen1:		value = program->wavelen1 / (float) WAVETABLESIZE;	break;
		case kWaveLen2:		value = program->wavelen2 / (float) WAVETABLESIZE;	break;
		case kWaveform2:	value = (program->waveform2 / 4.0f);		break;
		case kModEnvA:		value = (program->modEnv.attack / (44100*2.0f));		break;
		case kModEnvD:		value = (program->modEnv.decay / (44100*2.0f));		break;
		case kModEnvAmount:	value = (program->modEnvAmount/2.0)+0.5f;break;
		case kWaveformMix:	value = (program->waveformMix / 1024.0f);		break;
		case kLFO1:		value = (program->lfo1.waveform / 4.0f);		break;
		case kLFO1amount:	value = program->lfo1.amount / (1024.0f * 80.0f);		break;
		case kLFO1rate:		value = program->lfo1.rate;		break;
		case kLFO2:		value = (program->lfo2.waveform / 4.0f);		break;
		case kLFO2amount:	value = program->lfo2.amount / (1024.0f * 1024.0f);		break;
		case kLFO2rate:		value = program->lfo2.rate;		break;
		case kFilterType:	value = program->filter / 4.0f;			break;
		case kFilterRes:	value = program->resonance;		break;
		case kFilterCut:	value = program->cut;			break;
		case kFilterADSRAmount:	value = program->adsrAmount;		break;
		case kFilterCutA:	value = (program->filterCut.attack / (44100*2.0f));	break;
		case kFilterCutD:	value = (program->filterCut.decay / (44100*2.0f));	break;
		case kFilterCutS:	value = program->filterCut.sustain / 1024.0f;	break;
		case kFilterCutR:	value = (program->filterCut.release / (44100*2.0f));	break;
		case kDistortion:	value = (program->distortion / 1024.0f);		break;
		case kAmplifierA:	value = (program->amplifier.attack / (44100*2.0f));	break;
		case kAmplifierD:	value = (program->amplifier.decay / (44100*2.0f));	break;
		case kAmplifierS:	value = program->amplifier.sustain / 1024.0f;	break;
		case kAmplifierR:	value = (program->amplifier.release / (44100*2.0f));	break;
		case kGain:		value = program->gain / 1024.0f;			break;
		case kEchoDelay:	value = program->echoDelay / ((float) MAX_ECHO);		break;
		case kEchoAmount:	value = program->echoAmount / 1024.0f;		break;
	}
	return value;
}

void karma_Program_setParameter(karma_Program *program, long index, float value) {
	switch (index)
	{
		case kWaveform1:	
			program->waveform1	= (int) (value*4.0f);
			switch (program->waveform1) {
				default: case 0: program->waveform1Table = sineTable; break;
				case 1: program->waveform1Table = triTable; break;
				case 2: program->waveform1Table = sawTable; break;
				case 3: program->waveform1Table = squareTable; break;
				case 4: program->waveform1Table = noiseTable; break;
			}
			break;
		case kFreq1:		program->freq1		= (value*2)-1;	break;
		case kFreq2:		program->freq2		= (value*2)-1;	break;
		case kWaveform2:
			program->waveform2	= (int) (value*4.0f);
			switch (program->waveform2) {
				default: case 0: program->waveform2Table = sineTable; break;
				case 1: program->waveform2Table = triTable; break;
				case 2: program->waveform2Table = sawTable; break;
				case 3: program->waveform2Table = squareTable; break;
				case 4: program->waveform2Table = noiseTable; break;
			}

			break;
		case kWaveLen1:		program->wavelen1	= value*WAVETABLESIZE;	break;
		case kWaveLen2:		program->wavelen2	= value*WAVETABLESIZE;	break;
		case kModEnvA:		program->modEnv.attack	= (int)(value*44100*2);	break;
		case kModEnvD:		program->modEnv.decay	= (int)(value*44100*2);	break;
		case kModEnvAmount:	program->modEnvAmount	= (value*2)-1;	break;
		case kWaveformMix:	program->waveformMix	= (int) (value * 1024);	break;
		case kLFO1:
			program->lfo1.waveform	= (int) (value*4.0f);
			switch (program->lfo1.waveform) {
				default: case 0: program->lfo1.waveformTable = sineTable; break;
				case 1: program->lfo1.waveformTable = triTable; break;
				case 2: program->lfo1.waveformTable = sawTable; break;
				case 3: program->lfo1.waveformTable = squareTable; break;
				case 4: program->lfo1.waveformTable = noiseTable; break;
			}
			break;
		case kLFO1amount:	program->lfo1.amount	= (int) (value * (1024.0f * 80.0f));	break;
		case kLFO1rate:		program->lfo1.rate	= value;	break;
		case kLFO2:
			program->lfo2.waveform	= (int) (value*4.0f);
			switch (program->lfo2.waveform) {
				default: case 0: program->lfo2.waveformTable = sineTable; break;
				case 1: program->lfo2.waveformTable = triTable; break;
				case 2: program->lfo2.waveformTable = sawTable; break;
				case 3: program->lfo2.waveformTable = squareTable; break;
				case 4: program->lfo2.waveformTable = noiseTable; break;
			}
			break;
		case kLFO2amount:	program->lfo2.amount	= (int) (value * (1024.0f * 1024.0f));	break;
		case kLFO2rate:		program->lfo2.rate	= value;	break;
		case kFilterType:	program->filter		= value*4;	break;
		case kFilterRes:	program->resonance	= value;	break;
		case kFilterCut:	program->cut		= value;	break;
		case kFilterADSRAmount:	program->adsrAmount	= value;	break;
		case kFilterCutA:	program->filterCut.attack  = (int)(value*44100*2);	break;
		case kFilterCutD:	program->filterCut.decay   = (int)(value*44100*2);	break;
		case kFilterCutS:	program->filterCut.sustain = (int)(value*1024);	break;
		case kFilterCutR:	program->filterCut.release = (int)(value*44100*2);	break;
		case kDistortion:	program->distortion	  = (int) (value*1024);	break;
		case kAmplifierA:	program->amplifier.attack  = (int)(value*44100*2);	break;
		case kAmplifierD:	program->amplifier.decay   = (int)(value*44100*2);	break;
		case kAmplifierS:	program->amplifier.sustain = (int)(value*1024);	break;
		case kAmplifierR:	program->amplifier.release = (int)(value*44100*2);	break;
		case kGain:		program->gain		  = (int)(value*1024);	break;
		case kEchoDelay:	program->echoDelay	  = (int)(value*MAX_ECHO);	break;
		case kEchoAmount:	program->echoAmount	  = (int)(value*1024);	break;
	}
}