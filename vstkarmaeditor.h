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

	CKickButton	*waveform1Button;
	CKickButton	*waveform2Button;

	CKnob		*freq1Knob;
	CKnob		*freq2Knob;

	CKnob		*waveformMixKnob;

	CMovieBitmap	*osc1Display;
	CMovieBitmap	*osc2Display;

	// others
	long              oldTicks;
};

#endif
