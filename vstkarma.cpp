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
	setProgram (0);
	karma_Waveform_initTables();
	for (int i = 0; i < 16; i++) {
		karma_Channel_init(&channel[i]);
		sprintf(names[i], "Instr. %d", i+1);
	}

	if (audioMaster)
	{
		setNumInputs(0);		// no inputs
		setNumOutputs(kNumOutputs);		// 2 output
		canProcessReplacing();
		hasVu(false);
		hasClip(true);
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
	for (int i = 0; i < 16; i++) {
		karma_Channel_free(&channel[i]);
	}
}

//-----------------------------------------------------------------------------------------
void VstKarma::setProgram (long program)
{

	currentProgram = &channel[program].program;
	currentProgramIndex = program;

	setParameter(kWaveform1,	(currentProgram->waveform1 / 4.0f));
	setParameter(kFreq1,		(currentProgram->freq1/2.0)+0.5f);
	setParameter(kWaveLen1,		(currentProgram->wavelen1 / (float) (WAVETABLESIZE)));
	setParameter(kWaveform2,	(currentProgram->waveform2 / 4.0f));
	setParameter(kFreq2,		(currentProgram->freq2/2.0)+0.5f);
	setParameter(kWaveLen2,		(currentProgram->wavelen2 / (float) (WAVETABLESIZE)));
	setParameter(kWaveformMix,	(currentProgram->waveformMix / 1024.0f));
	setParameter(kModEnvA,		(currentProgram->modEnv.attack / (44100 * 2.0f)));
	setParameter(kModEnvD,		(currentProgram->modEnv.decay / (44100 * 2.0f)));
	setParameter(kModEnvAmount,	(currentProgram->modEnvAmount/2.0)+0.5f);
	setParameter(kLFO1,		(currentProgram->lfo1.waveform / 4.0f));
	setParameter(kLFO1rate,		currentProgram->lfo1.rate);
	setParameter(kLFO1amount,	currentProgram->lfo1.amount/ (1024.0f * 80.0f));
	setParameter(kLFO2,		(currentProgram->lfo2.waveform/4.0f));
	setParameter(kLFO2rate,		currentProgram->lfo2.rate);
	setParameter(kLFO2amount,	currentProgram->lfo2.amount / (1024.0f * 1024.0f));
	setParameter(kFilterType,	currentProgram->filter / 4.0f);
	setParameter(kFilterRes,	currentProgram->resonance);
	setParameter(kFilterCut,	currentProgram->cut);
	setParameter(kFilterADSRAmount,	currentProgram->adsrAmount);
	setParameter(kFilterCutA,	(currentProgram->filterCut.attack / (44100 * 2.0f)));
	setParameter(kFilterCutD,	(currentProgram->filterCut.decay / (44100 * 2.0f)));
	setParameter(kFilterCutS,	currentProgram->filterCut.sustain / 1024.0f);
	setParameter(kFilterCutR,	(currentProgram->filterCut.release / (44100 * 2.0f)));
	setParameter(kDistortion,	(currentProgram->distortion / 1024.0f));
	setParameter(kAmplifierA,	(currentProgram->amplifier.attack / (44100 * 2.0f)));
	setParameter(kAmplifierD,	(currentProgram->amplifier.decay / (44100 * 2.0f)));
	setParameter(kAmplifierS,	currentProgram->amplifier.sustain / 1024.0f);
	setParameter(kAmplifierR,	(currentProgram->amplifier.release / (44100 * 2.0f)));
	setParameter(kGain,		currentProgram->gain / 1024.0f);
	setParameter(kEchoDelay,	currentProgram->echoDelay / ((float) MAX_ECHO));
	setParameter(kEchoAmount,	currentProgram->echoAmount / (1024.0f));
	setParameter(kTest,		!channel[currentProgramIndex].note[0].released);
}

//-----------------------------------------------------------------------------------------
long VstKarma::getProgram() {
	return currentProgramIndex;
}

//-----------------------------------------------------------------------------------------
void VstKarma::setProgramName (char *name)
{
	strcpy (names[currentProgramIndex], name);
}

//-----------------------------------------------------------------------------------------
void VstKarma::getProgramName (char *name)
{
	strcpy (name, names[currentProgramIndex]);
}

//-----------------------------------------------------------------------------------------
void VstKarma::getParameterLabel (long index, char *label)
{
	switch (index)
	{
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
		case kLFO1amount:	float2string(currentProgram->lfo1.amount / (1024.0f * 80.0f), text);	break;
		case kLFO1rate:		float2string(currentProgram->lfo1.rate, text);		break;
		case kLFO2amount:	float2string(currentProgram->lfo2.amount/ (1024.0f * 1024.0f), text);	break;
		case kLFO2rate:		float2string(currentProgram->lfo2.rate, text);		break;
		case kWaveformMix:	float2string(currentProgram->waveformMix/ 1024.0f, text);	break;
		case kFilterCutS:	float2string(currentProgram->filterCut.sustain / 1024.0f, text);	break;
		case kFilterCut:	float2string(currentProgram->cut, text);		break;
		case kFilterRes:	float2string(currentProgram->resonance, text);		break;
		case kFilterADSRAmount:	float2string(currentProgram->adsrAmount, text);		break;
		case kModEnvAmount:	float2string(currentProgram->modEnvAmount, text);	break;
		case kAmplifierS:	dB2string(currentProgram->amplifier.sustain / 1024.0f, text);	break;

		case kGain:		dB2string (currentProgram->gain / 1024.0f, text);		break;

		case kFilterCutA:	value = (currentProgram->filterCut.attack / (44100 * 2.0f));	break;
		case kFilterCutD:	value = (currentProgram->filterCut.decay / (44100 * 2.0f));	break;
		case kFilterCutR:	value = (currentProgram->filterCut.release / (44100 * 2.0f));	break;
		case kModEnvA:		value = (currentProgram->modEnv.attack / (44100 * 2.0f));		break;
		case kModEnvD:		value = (currentProgram->modEnv.decay / (44100 * 2.0f));		break;
		case kAmplifierA:	value = (currentProgram->amplifier.attack / (44100 * 2.0f));	break;
		case kAmplifierD:	value = (currentProgram->amplifier.decay / (44100 * 2.0f));	break;
		case kAmplifierR:	value = (currentProgram->amplifier.release / (44100 * 2.0f));	break;
		case kDistortion:	value = (currentProgram->distortion / 1024.0f);		break;

		case kTest:
			if (!channel[currentProgramIndex].note[0].released) {
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
	if (index == kTest) {
		if (value > .5 && channel[currentProgramIndex].note[0].released) {
			karma_Channel_noteOn(&channel[currentProgramIndex], 0x40, 0x64, 0);
		} else if (value <= 0.5 && !channel[currentProgramIndex].note[0].released) {
			karma_Channel_noteOff(&channel[currentProgramIndex], 0x40);
		}
	} else {
		karma_Program_setParameter(&channel[currentProgramIndex].program, index, value);
	}
	if (editor)
		((AEffGUIEditor*)editor)->setParameter (index, value);
}

//-----------------------------------------------------------------------------------------
float VstKarma::getParameter (long index)
{
	float value = 0;

	if (index == kTest) {
		value = !channel[currentProgramIndex].note[0].released;
	} else {
		value = karma_Program_getParameter(&channel[currentProgramIndex].program, index);
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
		strcpy (text, names[index]);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------
bool VstKarma::copyProgram (long destination)
{
	if (destination < kNumPrograms)
	{
		memcpy(&channel[destination].program, &channel[curProgram].program, sizeof(karma_Program));
		memcpy(&names[destination], &names[curProgram], 30);
//		channel.programs[destination] = programs[curProgram];
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

