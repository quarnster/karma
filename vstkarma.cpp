#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vstkarma.h"

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
	Debug("in init...\n");
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
	initProcess();
	suspend();
}

//-----------------------------------------------------------------------------------------
VstKarma::~VstKarma ()
{
	Debug("unload...\n");
	if (programs)
		delete[] programs;
}

//-----------------------------------------------------------------------------------------
void VstKarma::setProgram (long program)
{

	currentProgram = &programs[program];
//	channel[0].program = &programs[program];
	currentProgramIndex = program;
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
		case kWaveform1:
		case kWaveform2:	strcpy (label, " Shape  ");		break;
		case kFilterType:	strcpy (label, "  Type  ");		break;

		case kFreq1A:
		case kFreq1D:
		case kFreq1R:
		case kFreq2A:
		case kFreq2D:
		case kFreq2R:
		case kFilterCutA:
		case kFilterCutD:
		case kFilterCutR:
		case kFilterResA:
		case kFilterResD:
		case kFilterResR:
		case kVolume1A:
		case kVolume1D:
		case kVolume1R:
		case kVolume2A:
		case kVolume2D:
		case kVolume2R: 	strcpy (label, "   Sec   ");		break;

//		case kVolume2:
		case kFilterResS:
		case kFreq1S:
		case kFreq2S:
		case kFilterCutS:	strcpy (label, "  Hz  ");		break;
		case kVolume1S:
		case kVolume2S:
		case kVolume:		strcpy (label, "   dB   ");		break;
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
				strcpy(text, "Off");
			else if (currentProgram->waveform1 < 0.4)
				strcpy(text, "Sawtooth");
			else if (currentProgram->waveform1 < 0.6)
				strcpy(text, " Square ");
			else if (currentProgram->waveform1 < 0.8)
				strcpy(text, "  Sine  ");
			else
				strcpy(text, "  Noise ");
			break;
		case kWaveform2:
			if (currentProgram->waveform2 < .20)
				strcpy(text, "Off");
			else if (currentProgram->waveform2 < 0.4)
				strcpy(text, "Sawtooth");
			else if (currentProgram->waveform2 < 0.6)
				strcpy(text, " Square ");
			else if (currentProgram->waveform2 < 0.8)
				strcpy(text, "  Sine  ");
			else
				strcpy(text, "  Noise ");
			break;

		case kFilterCutS:	float2string(currentProgram->filterCut.sustain, text);	break;
		case kFilterResS:	float2string(currentProgram->filterRes.sustain, text);	break;
		case kFreq1S:		float2string(currentProgram->freq1.sustain, text);	break;
		case kFreq2S:		float2string(currentProgram->freq2.sustain, text);	break;
		case kVolume1S:		dB2string(currentProgram->volume1.sustain, text);	break;
		case kVolume2S:		dB2string(currentProgram->volume2.sustain, text);	break;

		case kVolume:		dB2string (currentProgram->volume, text);		break;

		case kFilterCutA:	value = currentProgram->filterCut.attack;	break;
		case kFilterCutD:	value = currentProgram->filterCut.decay;	break;
		case kFilterCutR:	value = currentProgram->filterCut.release;	break;
		case kFilterResA:	value = currentProgram->filterRes.attack;	break;
		case kFilterResD:	value = currentProgram->filterRes.decay;	break;
		case kFilterResR:	value = currentProgram->filterRes.release;	break;
		case kFreq1A:		value = currentProgram->freq1.attack;		break;
		case kFreq1D:		value = currentProgram->freq1.decay;		break;
		case kFreq1R:		value = currentProgram->freq1.release;		break;
		case kFreq2A:		value = currentProgram->freq2.attack;		break;
		case kFreq2D:		value = currentProgram->freq2.decay;		break;
		case kFreq2R:		value = currentProgram->freq2.release;		break;
		case kVolume1A:		value = currentProgram->volume1.attack;		break;
		case kVolume1D:		value = currentProgram->volume1.decay;		break;
		case kVolume1R:		value = currentProgram->volume1.release;	break;
		case kVolume2A:		value = currentProgram->volume2.attack;		break;
		case kVolume2D:		value = currentProgram->volume2.decay;		break;
		case kVolume2R:		value = currentProgram->volume2.release;	break;
		case kDistortion:	value = currentProgram->distortion;		break;

		case kTest:
			if (!channel[currentProgram->channel].released) {
				strcpy(text, "  On  ");
			} else {
				strcpy(text, "  Off  ");
			}

	}
	if (value > -0.5) {
		if (value == 0.0) {
			strcpy(text, "  Off  ");
		} else {
			float2string(value*2, text);
		}
	}
}

//-----------------------------------------------------------------------------------------
void VstKarma::getParameterName (long index, char *label)
{
	switch (index)
	{
		case kChannel:		strcpy (label, "Channel");		break;
		case kWaveform1:	strcpy (label, " Wave 1 ");		break;
		case kFreq1A:		strcpy (label, " - attack");		break;
		case kFreq1D:		strcpy (label, " - decay");		break;
		case kFreq1S:		strcpy (label, " - sustain");		break;
		case kFreq1R:		strcpy (label, " - release");		break;
		case kVolume1A:		strcpy (label, " - Vol attack");	break;
		case kVolume1D:		strcpy (label, " - Vol decay");		break;
		case kVolume1S:		strcpy (label, " - Vol sustain");	break;
		case kVolume1R:		strcpy (label, " - Vol release");	break;
		case kWaveform2:	strcpy (label, " Wave 2 ");		break;
		case kFreq2A:		strcpy (label, " - attack");		break;
		case kFreq2D:		strcpy (label, " - decay");		break;
		case kFreq2S:		strcpy (label, " - sustain");		break;
		case kFreq2R:		strcpy (label, " - release");		break;
		case kVolume2A:		strcpy (label, " - Vol attack");	break;
		case kVolume2D:		strcpy (label, " - Vol decay");		break;
		case kVolume2S:		strcpy (label, " - Vol sustain");	break;
		case kVolume2R:		strcpy (label, " - Vol release");	break;
		case kVolume:		strcpy (label, " General volume ");	break;
		case kFilterType:	strcpy (label, " Filter type");		break;
		case kFilterCutA:	strcpy (label, " - Cut attack");	break;
		case kFilterCutD:	strcpy (label, " - Cut decay");		break;
		case kFilterCutS:	strcpy (label, " - Cut sustain");	break;
		case kFilterCutR:	strcpy (label, " - Cut release");	break;
		case kFilterResA:	strcpy (label, " - Res attack");	break;
		case kFilterResD:	strcpy (label, " - Res decay");		break;
		case kFilterResS:	strcpy (label, " - Res sustain");	break;
		case kFilterResR:	strcpy (label, " - Res release");	break;
		case kDistortion:	strcpy (label, " Distortion  ");	break;
		case kTest:		strcpy (label, " Test");		break;

	}
}

//-----------------------------------------------------------------------------------------
void VstKarma::setParameter (long index, float value)
{
	switch (index)
	{
		case kChannel:
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

			break;
		case kWaveform1:	currentProgram->waveform1	= value;	break;
		case kFreq1A:		currentProgram->freq1.attack	= value;	break;
		case kFreq1D:		currentProgram->freq1.decay	= value;	break;
		case kFreq1S:		currentProgram->freq1.sustain	= value;	break;
		case kFreq1R:		currentProgram->freq1.release	= value;	break;
		case kVolume1A:		currentProgram->volume1.attack	= value;	break;
		case kVolume1D:		currentProgram->volume1.decay	= value;	break;
		case kVolume1S:		currentProgram->volume1.sustain	= value;	break;
		case kVolume1R:		currentProgram->volume1.release	= value;	break;
		case kWaveform2:	currentProgram->waveform2	= value;	break;
		case kFreq2A:		currentProgram->freq2.attack	= value;	break;
		case kFreq2D:		currentProgram->freq2.decay	= value;	break;
		case kFreq2S:		currentProgram->freq2.sustain	= value;	break;
		case kFreq2R:		currentProgram->freq2.release	= value;	break;
		case kVolume2A:		currentProgram->volume2.attack	= value;	break;
		case kVolume2D:		currentProgram->volume2.decay	= value;	break;
		case kVolume2S:		currentProgram->volume2.sustain	= value;	break;
		case kVolume2R:		currentProgram->volume2.release	= value;	break;
		case kVolume:		currentProgram->volume		= value;	break;
		case kFilterType:	currentProgram->filter		= value;	break;
		case kFilterResA:	currentProgram->filterRes.attack = value;	break;
		case kFilterResD:	currentProgram->filterRes.decay = value;	break;
		case kFilterResS:	currentProgram->filterRes.sustain = value;	break;
		case kFilterResR:	currentProgram->filterRes.release = value;	break;
		case kFilterCutA:	currentProgram->filterCut.attack = value;	break;
		case kFilterCutD:	currentProgram->filterCut.decay = value;	break;
		case kFilterCutS:	currentProgram->filterCut.sustain = value;	break;
		case kFilterCutR:	currentProgram->filterCut.release = value;	break;
		case kDistortion:	currentProgram->distortion = value;		break;

		case kTest:
			if (value > .5 && channel[currentProgram->channel].released) {
				channel[currentProgram->channel].noteOn (0x40, 0x64, 0);
			} else if (value <= 0.5 && !channel[currentProgram->channel].released) {
				channel[currentProgram->channel].noteOff();
			}
			break;
	}
}

//-----------------------------------------------------------------------------------------
float VstKarma::getParameter (long index)
{
	float value = 0;
	switch (index)
	{
		case kChannel:		value = currentProgram->fChannel;		break;
		case kWaveform1:	value = currentProgram->waveform1;		break;
		case kFreq1A:		value = currentProgram->freq1.attack;		break;
		case kFreq1D:		value = currentProgram->freq1.decay;		break;
		case kFreq1S:		value = currentProgram->freq1.sustain;		break;
		case kFreq1R:		value = currentProgram->freq1.release;		break;
		case kVolume1A:		value = currentProgram->volume1.attack;		break;
		case kVolume1D:		value = currentProgram->volume1.decay;		break;
		case kVolume1S:		value = currentProgram->volume1.sustain;	break;
		case kVolume1R:		value = currentProgram->volume1.release;	break;
		case kWaveform2:	value = currentProgram->waveform2;		break;
		case kFreq2A:		value = currentProgram->freq2.attack;		break;
		case kFreq2D:		value = currentProgram->freq2.decay;		break;
		case kFreq2S:		value = currentProgram->freq2.sustain;		break;
		case kFreq2R:		value = currentProgram->freq2.release;		break;
		case kVolume2A:		value = currentProgram->volume2.attack;		break;
		case kVolume2D:		value = currentProgram->volume2.decay;		break;
		case kVolume2S:		value = currentProgram->volume2.sustain;	break;
		case kVolume2R:		value = currentProgram->volume2.release;	break;
		case kVolume:		value = currentProgram->volume;			break;
		case kFilterType:	value = currentProgram->filter;			break;
		case kFilterResA:	value = currentProgram->filterRes.attack;	break;
		case kFilterResD:	value = currentProgram->filterRes.decay;	break;
		case kFilterResS:	value = currentProgram->filterRes.sustain;	break;
		case kFilterResR:	value = currentProgram->filterRes.release;	break;
		case kFilterCutA:	value = currentProgram->filterCut.attack;	break;
		case kFilterCutD:	value = currentProgram->filterCut.decay;	break;
		case kFilterCutS:	value = currentProgram->filterCut.sustain;	break;
		case kFilterCutR:	value = currentProgram->filterCut.release;	break;
		case kDistortion:	value = currentProgram->distortion;		break;
		case kTest:
				value = !channel[currentProgram->channel].released;

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

