#ifndef __KARMA_VSTKARMA_EDITOR_H
#define __KARMA_VSTKARMA_EDITOR_H

#include "D:\CODE\vst\vstsdk2.3\vstsdk2.3\source\common\vstgui.h"
#include "vstkarma.h"

// class Test;
// class CLabel;

//-----------------------------------------------------------------------------
class VstKarmaEditor : public AEffGUIEditor, public CControlListener
{
public:
	VstKarmaEditor (AudioEffect *effect);
	virtual ~VstKarmaEditor ();

	void suspend ();
	void resume ();
	bool keysRequired ();

protected:
	virtual long open (void *ptr);
	virtual void idle ();
	void setParameter (long index, float value);
	virtual void close ();
	
	// VST 2.1
	virtual long onKeyDown (VstKeyCode &keyCode);
	virtual long onKeyUp (VstKeyCode &keyCode);

private:
	void valueChanged (CDrawContext* context, CControl* control);

	//==Osc-panel===========================================================
	CKickButton	*waveform1Button;
	CKickButton	*waveform2Button;

	CKnob		*freq1Knob;
	CKnob		*freq2Knob;

	CKnob		*waveformMixKnob;

	CMovieBitmap	*osc1Display;
	CMovieBitmap	*osc2Display;

	//==LFO1-panel===========================================================
	CKickButton	*lfo1Button;

	CKnob		*lfo1AKnob;
	CKnob		*lfo1RKnob;

	CMovieBitmap	*lfo1Display;

	//==LFO2-panel===========================================================
	CKickButton	*lfo2Button;

	CKnob		*lfo2AKnob;
	CKnob		*lfo2RKnob;

	CMovieBitmap	*lfo2Display;

	//==Test-button===========================================================
	COnOffButton	*testButton;

	//==Mod-envelope===========================================================
	CKnob		*modEnvAttack;
	CKnob		*modEnvDecay;
	CKnob		*modEnvAmount;

	//==Amplifier===========================================================
	CKnob		*ampAttack;
	CKnob		*ampDecay;
	CKnob		*ampSustain;
	CKnob		*ampRelease;
	CKnob		*ampGain;

	//==Filter===========================================================
	CKnob		*filterA;
	CKnob		*filterD;
	CKnob		*filterS;
	CKnob		*filterR;
	CKnob		*filterADSRAmount;
	CKnob		*filterCut;
	CKnob		*filterRes;
	CKnob		*distortion;
	CKickButton	*filterButton;
	CMovieBitmap	*filterDisplay;


	// others
	long              oldTicks;
};

#endif
