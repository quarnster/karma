#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vstkarma.h"
#include "vstkarmaeditor.h"

void VstKarma::Debug(char *str) {
	FILE *f = NULL;
	if ((f = fopen("c:\\karma.log", "a")) != NULL) {
		fprintf(f, str);
		fclose(f);
	}
}

//-----------------------------------------------------------------------------------------
// VstKarma
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
VstKarma::VstKarma(audioMasterCallback audioMaster) : AudioEffectX (audioMaster, kNumPrograms, kNumParams)
{
	programs = new VstProgram[kNumPrograms];
	if (programs) {
		setProgram (0);
		for (int i = 0; i < 16; i++) {
			programs[i].channel = i;
			programs[i].fChannel = i / 15.0f;
			sprintf(programs[i].name, "Instr. %d", i+1);
			channel[i].setProgram(&programs[i]);
		}
	} else {
		Debug("Error allocating programspace????!!\n");
	}

	if (audioMaster)
	{
		setNumInputs(0);		// no inputs
		setNumOutputs(kNumOutputs);		// 2 output
		canProcessReplacing();
		hasVu(false);
		hasClip(false);
		isSynth();
		setUniqueID('OTBk');	// <<<! *must* change this!!!!
	}
	editor = new VstKarmaEditor (this);

	initProcess();
	suspend();
}

//-----------------------------------------------------------------------------------------
VstKarma::~VstKarma ()
{
	if (programs)
		delete[] programs;
}

//-----------------------------------------------------------------------------------------
void VstKarma::setProgram (long program)
{

	currentProgram = &programs[program];
//	channel[0].program = &programs[program];
	currentProgramIndex = program;

	setParameter(kChannel,		currentProgram->fChannel);
	setParameter(kWaveform1,	currentProgram->waveform1);
	setParameter(kFreq1,		(currentProgram->freq1/2.0)+0.5f);
	setParameter(kWaveform2,	currentProgram->waveform2);
	setParameter(kFreq2,		(currentProgram->freq2/2.0)+0.5f);
	setParameter(kWaveformMix,	currentProgram->waveformMix);
	setParameter(kModEnvA,		currentProgram->modEnv.attack);
	setParameter(kModEnvD,		currentProgram->modEnv.decay);
	setParameter(kModEnvAmount,	(currentProgram->modEnvAmount/2.0)+0.5f);
	setParameter(kLFO1,		currentProgram->lfo1.waveform);
	setParameter(kLFO1rate,		currentProgram->lfo1.rate);
	setParameter(kLFO1amount,	currentProgram->lfo1.amount);
	setParameter(kLFO2,		currentProgram->lfo2.waveform);
	setParameter(kLFO2rate,		currentProgram->lfo2.rate);
	setParameter(kLFO2amount,	currentProgram->lfo2.amount);
	setParameter(kFilterType,	currentProgram->filter);
	setParameter(kFilterRes,	currentProgram->resonance);
	setParameter(kFilterCut,	currentProgram->cut);
	setParameter(kFilterADSRAmount,	currentProgram->adsrAmount);
	setParameter(kFilterCutA,	currentProgram->filterCut.attack);
	setParameter(kFilterCutD,	currentProgram->filterCut.decay);
	setParameter(kFilterCutS,	currentProgram->filterCut.sustain);
	setParameter(kFilterCutR,	currentProgram->filterCut.release);
	setParameter(kDistortion,	currentProgram->distortion);
	setParameter(kAmplifierA,	currentProgram->amplifier.attack);
	setParameter(kAmplifierD,	currentProgram->amplifier.decay);
	setParameter(kAmplifierS,	currentProgram->amplifier.sustain);
	setParameter(kAmplifierR,	currentProgram->amplifier.release);
	setParameter(kGain,		currentProgram->gain);
	setParameter(kTest,		!channel[currentProgram->channel].note[0].released);
}

//-----------------------------------------------------------------------------------------
long VstKarma::getProgram() {
	return currentProgramIndex;
}

//-----------------------------------------------------------------------------------------
void VstKarma::setProgramName (char *name)
{
	strcpy (currentProgram->name, name);
}

//-----------------------------------------------------------------------------------------
void VstKarma::getProgramName (char *name)
{
	strcpy (name, currentProgram->name);
}

//-----------------------------------------------------------------------------------------
void VstKarma::getParameterLabel (long index, char *label)
{
	switch (index)
	{
//		case kChannel:		strcpy (label, " Channel");		break;
		case kLFO1:
		case kLFO2:

		case kWaveform1:
		case kWaveform2:	strcpy (label, " Shape  ");		break;
		case kFilterType:	strcpy (label, "  Type  ");		break;

		case kModEnvA:
		case kModEnvD:
		case kFilterCutA:
		case kFilterCutD:
		case kFilterCutR:
		case kAmplifierA:
		case kAmplifierD:
		case kAmplifierR: 	strcpy (label, "   Sec   ");		break;

//		case kVolume2:
		case kFreq1:
		case kFreq2:
		case kLFO1rate:
		case kLFO2rate:
		case kFilterRes:
		case kFilterADSRAmount:
		case kFilterCut:
		case kFilterCutS:	strcpy (label, "  Hz  ");		break;

		case kAmplifierS:
		case kLFO1amount:
		case kLFO2amount:
		case kGain:		strcpy (label, "   dB   ");		break;
	}
}

//-----------------------------------------------------------------------------------------
void VstKarma::getParameterDisplay(long index, char *text)
{
	text[0] = 0;
	float value = -1;
	switch (index)
	{
		case kChannel:		long2string(currentProgram->channel+1, text); break;
		case kFilterType:
			if (currentProgram->filter < .20)
				strcpy(text, "Off");
			else if (currentProgram->filter < 0.4)
				strcpy(text, " Lowpass");
			else if (currentProgram->filter < 0.6)
				strcpy(text, "Highpass");
			else if (currentProgram->filter < 0.8)
				strcpy(text, "Bandpass");
			else
				strcpy(text, "  Notch ");
			break;

		case kWaveform1:
			if (currentProgram->waveform1 < .20)
				strcpy(text, "  Sine  ");
			else if (currentProgram->waveform1 < 0.4)
				strcpy(text, "Triangle");
			else if (currentProgram->waveform1 < 0.6)
				strcpy(text, "Sawtooth");
			else if (currentProgram->waveform1 < 0.8)
				strcpy(text, " Square ");
			else
				strcpy(text, "  Noise ");
			break;
		case kWaveform2:
			if (currentProgram->waveform2 < .20)
				strcpy(text, "  Sine  ");
			else if (currentProgram->waveform2 < 0.4)
				strcpy(text, "Triangle");
			else if (currentProgram->waveform2 < 0.6)
				strcpy(text, "Sawtooth");
			else if (currentProgram->waveform2 < 0.8)
				strcpy(text, " Square ");
			else
				strcpy(text, "  Noise ");
			break;

		case kLFO1:
			if (currentProgram->lfo1.waveform < .20)
				strcpy(text, "  Sine  ");
			else if (currentProgram->lfo1.waveform < 0.4)
				strcpy(text, "Triangle");
			else if (currentProgram->lfo1.waveform < 0.6)
				strcpy(text, "Sawtooth");
			else if (currentProgram->lfo1.waveform < 0.8)
				strcpy(text, " Square ");
			else
				strcpy(text, "  Noise ");
			break;
		case kLFO2:
			if (currentProgram->lfo2.waveform < .20)
				strcpy(text, "  Sine  ");
			else if (currentProgram->lfo2.waveform < 0.4)
				strcpy(text, "Triangle");
			else if (currentProgram->lfo2.waveform < 0.6)
				strcpy(text, "Sawtooth");
			else if (currentProgram->lfo2.waveform < 0.8)
				strcpy(text, " Square ");
			else
				strcpy(text, "  Noise ");
			break;
		case kFreq1:		float2string(currentProgram->freq1, text);		break;
		case kFreq2:		float2string(currentProgram->freq2, text);		break;
		case kLFO1amount:	float2string(currentProgram->lfo1.amount, text);	break;
		case kLFO1rate:		float2string(currentProgram->lfo1.rate, text);		break;
		case kLFO2amount:	float2string(currentProgram->lfo2.amount, text);	break;
		case kLFO2rate:		float2string(currentProgram->lfo2.rate, text);		break;
		case kWaveformMix:	float2string(currentProgram->waveformMix, text);	break;
		case kFilterCutS:	float2string(currentProgram->filterCut.sustain, text);	break;
		case kFilterCut:	float2string(currentProgram->cut, text);		break;
		case kFilterRes:	float2string(currentProgram->resonance, text);		break;
		case kFilterADSRAmount:	float2string(currentProgram->adsrAmount, text);		break;
		case kModEnvAmount:	float2string(currentProgram->modEnvAmount, text);	break;
		case kAmplifierS:	dB2string(currentProgram->amplifier.sustain, text);	break;

		case kGain:		dB2string (currentProgram->gain, text);		break;

		case kFilterCutA:	value = currentProgram->filterCut.attack;	break;
		case kFilterCutD:	value = currentProgram->filterCut.decay;	break;
		case kFilterCutR:	value = currentProgram->filterCut.release;	break;
		case kModEnvA:		value = currentProgram->modEnv.attack;		break;
		case kModEnvD:		value = currentProgram->modEnv.decay;		break;
		case kAmplifierA:	value = currentProgram->amplifier.attack;	break;
		case kAmplifierD:	value = currentProgram->amplifier.decay;	break;
		case kAmplifierR:	value = currentProgram->amplifier.release;	break;
		case kDistortion:	value = currentProgram->distortion;		break;

		case kTest:
			if (!channel[currentProgram->channel].note[0].released) {
				strcpy(text, "  On  ");
			} else {
				strcpy(text, "  Off  ");
			}

	}
	if (value > -0.5) {
//		if (value == 0.0) {
//			strcpy(text, "  Off  ");
//		} else {
			float2string(value*2, text);
//		}
	}
}

//-----------------------------------------------------------------------------------------
void VstKarma::getParameterName (long index, char *label)
{
	switch (index)
	{
		case kChannel:		strcpy (label, "Channel");		break;
		case kWaveform1:	strcpy (label, " Wave 1 ");		break;
		case kFreq1:		strcpy (label, " - freq");		break;
		case kWaveform2:	strcpy (label, " Wave 2 ");		break;
		case kFreq2:		strcpy (label, " - freq");		break;
		case kWaveformMix:	strcpy (label, "Waveform mix");		break;
		case kModEnvAmount:	strcpy (label, "Amount");		break;
		case kModEnvA:		strcpy (label, "ModEnv A");		break;
		case kModEnvD:		strcpy (label, "ModEnv D");		break;
		case kLFO1:		strcpy (label, "LFO 1");		break;
		case kLFO1amount:	strcpy (label, " - amount");		break;
		case kLFO1rate:		strcpy (label, " - rate");		break;
		case kLFO2:		strcpy (label, "LFO 2");		break;
		case kLFO2amount:	strcpy (label, " - amount");		break;
		case kLFO2rate:		strcpy (label, " - rate");		break;

		case kFilterType:	strcpy (label, " Filter type");		break;
		case kFilterCutA:	strcpy (label, " - Cut attack");	break;
		case kFilterCutD:	strcpy (label, " - Cut decay");		break;
		case kFilterCutS:	strcpy (label, " - Cut sustain");	break;
		case kFilterCutR:	strcpy (label, " - Cut release");	break;
		case kFilterRes:	strcpy (label, " - Resonance");		break;
		case kFilterCut:	strcpy (label, " - Cut");		break;
		case kFilterADSRAmount:	strcpy (label, " - ADSR Amount");	break;
		case kDistortion:	strcpy (label, " Distortion  ");	break;
		case kAmplifierA:	strcpy (label, " Amp attack ");		break;
		case kAmplifierD:	strcpy (label, " Amp decay ");		break;
		case kAmplifierS:	strcpy (label, " Amp sustain ");	break;
		case kAmplifierR:	strcpy (label, " Amp release ");	break;
		case kGain:		strcpy (label, " Gain ");		break;

		case kTest:		strcpy (label, " Test");		break;

	}
}

//-----------------------------------------------------------------------------------------
void VstKarma::setParameter (long index, float value)
{
	switch (index)
	{
		case kChannel:
/*
			{
				int newChannel = (int)(value*15.0);
				if (currentProgram->channel != newChannel) {
					char buf[256];
					sprintf(buf, "Channel: %d, %f, (old: %d)\n", newChannel, value, currentProgram->channel);
					Debug(buf);

					channel[currentProgram->channel].delProgram(currentProgram);
					currentProgram->fChannel = value;
					currentProgram->channel	= newChannel;
					channel[currentProgram->channel].setProgram(currentProgram);
					sprintf(buf, "Channel after: %d\n", currentProgram->channel);
					Debug(buf);
				}
			}

*/
			break;
		case kWaveform1:	currentProgram->waveform1	= value;	break;
		case kFreq1:		currentProgram->freq1		= (value*2)-1;	break;
		case kFreq2:		currentProgram->freq2		= (value*2)-1;	break;
		case kWaveform2:	currentProgram->waveform2	= value;	break;
		case kModEnvA:		currentProgram->modEnv.attack	= value;	break;
		case kModEnvD:		currentProgram->modEnv.decay	= value;	break;
		case kModEnvAmount:	currentProgram->modEnvAmount	= (value*2)-1;	break;
		case kWaveformMix:	currentProgram->waveformMix	= value;	break;
		case kLFO1:		currentProgram->lfo1.waveform	= value;	break;
		case kLFO1amount:	currentProgram->lfo1.amount	= value;	break;
		case kLFO1rate:		currentProgram->lfo1.rate	= value;	break;
		case kLFO2:		currentProgram->lfo2.waveform	= value;	break;
		case kLFO2amount:	currentProgram->lfo2.amount	= value;	break;
		case kLFO2rate:		currentProgram->lfo2.rate	= value;	break;
		case kFilterType:	currentProgram->filter		= value;	break;
		case kFilterRes:	currentProgram->resonance	= value;	break;
		case kFilterCut:	currentProgram->cut		= value;	break;
		case kFilterADSRAmount:	currentProgram->adsrAmount	= value;	break;
		case kFilterCutA:	currentProgram->filterCut.attack  = value;	break;
		case kFilterCutD:	currentProgram->filterCut.decay   = value;	break;
		case kFilterCutS:	currentProgram->filterCut.sustain = value;	break;
		case kFilterCutR:	currentProgram->filterCut.release = value;	break;
		case kDistortion:	currentProgram->distortion	  = value;	break;
		case kAmplifierA:	currentProgram->amplifier.attack  = value;	break;
		case kAmplifierD:	currentProgram->amplifier.decay   = value;	break;
		case kAmplifierS:	currentProgram->amplifier.sustain = value;	break;
		case kAmplifierR:	currentProgram->amplifier.release = value;	break;
		case kGain:		currentProgram->gain		  = value;	break;

		case kTest:
			if (value > .5 && channel[currentProgram->channel].note[0].released) {
				channel[currentProgram->channel].noteOn (0x40, 0x64, 0);
			} else if (value <= 0.5 && !channel[currentProgram->channel].note[0].released) {
				channel[currentProgram->channel].noteOff(0x40);
			}
			break;
	}
	if (editor)
		((AEffGUIEditor*)editor)->setParameter (index, value);
}

//-----------------------------------------------------------------------------------------
float VstKarma::getParameter (long index)
{
	float value = 0;
	switch (index)
	{
		case kChannel:		value = currentProgram->fChannel;		break;
		case kWaveform1:	value = currentProgram->waveform1;		break;
		case kFreq1:		value = (currentProgram->freq1/2.0)+0.5f;	break;
		case kFreq2:		value = (currentProgram->freq2/2.0)+0.5f;	break;
		case kWaveform2:	value = currentProgram->waveform2;		break;
		case kModEnvA:		value = currentProgram->modEnv.attack;		break;
		case kModEnvD:		value = currentProgram->modEnv.decay;		break;
		case kModEnvAmount:	value = (currentProgram->modEnvAmount/2.0)+0.5f;break;
		case kWaveformMix:	value = currentProgram->waveformMix;		break;
		case kLFO1:		value = currentProgram->lfo1.waveform;		break;
		case kLFO1amount:	value = currentProgram->lfo1.amount;		break;
		case kLFO1rate:		value = currentProgram->lfo1.rate;		break;
		case kLFO2:		value = currentProgram->lfo2.waveform;		break;
		case kLFO2amount:	value = currentProgram->lfo2.amount;		break;
		case kLFO2rate:		value = currentProgram->lfo2.rate;		break;
		case kFilterType:	value = currentProgram->filter;			break;
		case kFilterRes:	value = currentProgram->resonance;		break;
		case kFilterCut:	value = currentProgram->cut;			break;
		case kFilterADSRAmount:	value = currentProgram->adsrAmount;		break;
		case kFilterCutA:	value = currentProgram->filterCut.attack;	break;
		case kFilterCutD:	value = currentProgram->filterCut.decay;	break;
		case kFilterCutS:	value = currentProgram->filterCut.sustain;	break;
		case kFilterCutR:	value = currentProgram->filterCut.release;	break;
		case kDistortion:	value = currentProgram->distortion;		break;
		case kAmplifierA:	value = currentProgram->amplifier.attack;	break;
		case kAmplifierD:	value = currentProgram->amplifier.decay;	break;
		case kAmplifierS:	value = currentProgram->amplifier.sustain;	break;
		case kAmplifierR:	value = currentProgram->amplifier.release;	break;
		case kGain:		value = currentProgram->gain;			break;
		case kTest:
				value = !channel[currentProgram->channel].note[0].released;

	}
	return value;
}

//-----------------------------------------------------------------------------------------
bool VstKarma::getOutputProperties (long index, VstPinProperties* properties)
{
	// TODO: check
	if (index < kNumOutputs)
	{
		sprintf(properties->label, "Karma %1d", index + 1);
		properties->flags = kVstPinIsActive;
		if (index < 2)
			properties->flags |= kVstPinIsStereo;	// test, make channel 1+2 stereo
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------
bool VstKarma::getProgramNameIndexed (long category, long index, char* text)
{
	if (index < kNumPrograms)
	{
		strcpy (text, programs[index].name);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------
bool VstKarma::copyProgram (long destination)
{
	if (destination < kNumPrograms)
	{
		programs[destination] = programs[curProgram];
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------
bool VstKarma::getEffectName (char* name)
{
	strcpy (name, "Karma");
	return true;
}

//-----------------------------------------------------------------------------------------
bool VstKarma::getVendorString (char* text)
{
	strcpy (text, "quarn/outbreak");
	return true;
}

//-----------------------------------------------------------------------------------------
bool VstKarma::getProductString (char* text)
{
	strcpy (text, "Vst Synth");
	return true;
}

//-----------------------------------------------------------------------------------------
long VstKarma::canDo (char* text)
{
	if (!strcmp (text, "receiveVstEvents"))
		return 1;
	if (!strcmp (text, "receiveVstMidiEvent"))
		return 1;
	return -1;	// explicitly can't do; 0 => don't know
}

