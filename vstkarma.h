#ifndef __INCLUDED_KARMA_VSTKARMA_H
#define __INCLUDED_KARMA_VSTKARMA_H

#include <string.h>
#include "D:\CODE\vst\vstsdk2.3\vstsdk2.3\source\common\audioeffectx.h"

#include "VstProgram.h"
#include "VstChannel.h"

enum
{
	kNumPrograms = 16,
	kNumOutputs = 2,

	kChannel = 0,
	kWaveform1,
	kFreq1,

	kWaveform2,

	kFreq2,

	kWaveformMix,

	kModEnvA,
	kModEnvD,
	kModEnvAmount,

	kLFO1,
	kLFO1rate,
	kLFO1amount,

	kLFO2,
	kLFO2rate,
	kLFO2amount,

	kFilterType,
	kFilterRes,
	kFilterCut,
	kFilterADSRAmount,

	kFilterCutA,
	kFilterCutD,
	kFilterCutS,
	kFilterCutR,

	kDistortion,
	kAmplifierA,
	kAmplifierD,
	kAmplifierS,
	kAmplifierR,
	kGain,

	kTest,

	kNumParams
};



//------------------------------------------------------------------------------------------
class VstKarma : public AudioEffectX
{
public:
	VstKarma(audioMasterCallback audioMaster);
	~VstKarma();

	static void Debug(char *blah);

	virtual void process(float **inputs, float **outputs, long sampleframes);
	virtual void processReplacing(float **inputs, float **outputs, long sampleframes);
	virtual long processEvents(VstEvents* events);

	virtual void setProgram(long program);
	virtual long getProgram();
	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);
	virtual void setParameter(long index, float value);
	virtual float getParameter(long index);
	virtual void getParameterLabel(long index, char *label);
	virtual void getParameterDisplay(long index, char *text);
	virtual void getParameterName(long index, char *text);
	virtual void setSampleRate(float sampleRate);
	virtual void setBlockSize(long blockSize);
	virtual void resume();

	virtual bool getOutputProperties (long index, VstPinProperties* properties);
	virtual bool getProgramNameIndexed (long category, long index, char* text);
	virtual bool copyProgram (long destination);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual long getVendorVersion () {return 1;}
	virtual long canDo (char* text);

private:
	void initProcess();

	VstChannel channel[16];
	VstProgram* programs;
	VstProgram* currentProgram;
	long	currentProgramIndex;
};


#endif
