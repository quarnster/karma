//------------------------------------------------------------------------
//-
//- Project     : Use different Controls of VSTGUI
//- Filename    : VstKarmaEditor.cpp
//- Created by  : Yvan Grabit
//- Description :
//-
//- © 2000-1999, Steinberg Soft und Hardware GmbH, All Rights Reserved
//------------------------------------------------------------------------

#include "vstkarmaeditor.h"


#include <math.h>
#include <stdlib.h>	
#include <stdio.h>

#include "resource1.h"


enum
{
	// bitmaps
	kOscBitmap = IDB_BITMAP1,
	kBackgroundBitmap,
	
	kButtonBitmap,
	kEnvDisplayBitmap,
	kFilterDisplayBitmap,
	

	kKnobBgBitmap,
	kKnobFgBitmap,

	kLFODisplayBitmap,
	


	// others
	kBackgroundW = 904,
	kBackgroundH = 680
};


void stringConvert (float value, char* string);

/*
//-----------------------------------------------------------------------------
// CLabel declaration
//-----------------------------------------------------------------------------
class CLabel : public CParamDisplay
{
public:
	CLabel (CRect &size, char *text);

	void draw (CDrawContext *pContext);

	void setLabel (char *text);
	bool onDrop (void **ptrItems, long nbItems, long type, CPoint &where);

protected:
	char label[256];
};

//-----------------------------------------------------------------------------
// CLabel implementation
//-----------------------------------------------------------------------------
CLabel::CLabel (CRect &size, char *text)
: CParamDisplay (size)
{
	strcpy (label, "");
	setLabel (text);
}

//------------------------------------------------------------------------
void CLabel::setLabel (char *text)
{
	if (text)
		strcpy (label, text);
	setDirty ();
}

//-----------------------------------------------------------------------------
bool CLabel::onDrop (void **ptrItems, long nbItems, long type, CPoint &where)
{
	if (nbItems > 0 && type == kDropFiles)
	{
		char text[1024];
		long pos = where.h - size.left;
		sprintf (text, "%d : %s at %d", nbItems, (char*)ptrItems[0], pos);
		setLabel (text);
	}
	return true;
}

//------------------------------------------------------------------------
void CLabel::draw (CDrawContext *pContext)
{
	pContext->setFillColor (backColor);
	pContext->fillRect (size);
	pContext->setFrameColor (fontColor);
	pContext->drawRect (size);

	pContext->setFont (fontID);
	pContext->setFontColor (fontColor);
	pContext->drawString (label, size, false, kCenterText);
}

*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VstKarmaEditor::VstKarmaEditor (AudioEffect *effect) 
	:	AEffGUIEditor (effect)
{
	frame = 0;
	oldTicks = 0;

	rect.left   = 0;
	rect.top    = 0;
	rect.right  = kBackgroundW;
	rect.bottom = kBackgroundH;

	// we decide in this plugin to open all bitmaps in the open function
}

//-----------------------------------------------------------------------------
VstKarmaEditor::~VstKarmaEditor ()
{}

//-----------------------------------------------------------------------------
long VstKarmaEditor::open (void *ptr)
{
	// always call this !!!
	AEffGUIEditor::open (ptr);

	// get version
	int version = getVstGuiVersion ();
	int verMaj = (version & 0xFF00) >> 16;
	int verMin = (version & 0x00FF);

	// init the background bitmap
	CBitmap *background = new CBitmap (kBackgroundBitmap);

	//--CFrame-----------------------------------------------
	CRect size (0, 0, background->getWidth () + 100, background->getHeight ());
	frame = new CFrame (size, ptr, this);
	frame->setBackground (background);
	background->forget ();

	CPoint point (0, 0);
	CBitmap *button = new CBitmap (kButtonBitmap);
	CBitmap *knob   = new CBitmap (kKnobFgBitmap);
	CBitmap *bgKnob = new CBitmap (kKnobBgBitmap);

	//==Osc-panel===========================================================

	// osc-selector 1 and 2
	size(0, 0, 60, 29);
	size.offset(39, 251);
	waveform1Button = new CKickButton (size, this, kWaveform1, 29, button, point);
	frame->addView (waveform1Button);

	size(0, 0, 60, 29);
	size.offset(218, 250);
	waveform2Button = new CKickButton (size, this, kWaveform2, 29, button, point);
	frame->addView (waveform2Button);

	// freq1 and 2

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(57, 81); 
	freq1Knob = new CKnob (size, this, kFreq1, bgKnob, knob, point);
	freq1Knob->setInsetValue (7);
	freq1Knob->setValue(effect->getParameter(kFreq1));
	frame->addView (freq1Knob);

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(226, 80); 
	freq2Knob = new CKnob (size, this, kFreq2, bgKnob, knob, point);
	freq2Knob->setInsetValue (7);
	freq2Knob->setValue(effect->getParameter(kFreq2));
	frame->addView (freq2Knob);

	// waveform mix
 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(144, 93); 
	waveformMixKnob = new CKnob (size, this, kWaveformMix, bgKnob, knob, point);
	waveformMixKnob->setInsetValue (7);
	waveformMixKnob->setValue(effect->getParameter(kWaveformMix));
	frame->addView (waveformMixKnob);


	// wavelen 1 and 2
 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(144, 181); 
	wavelen1Knob = new CKnob (size, this, kWaveLen1, bgKnob, knob, point);
	wavelen1Knob->setInsetValue (7);
	wavelen1Knob->setValue(effect->getParameter(kWaveLen1));
	frame->addView (wavelen1Knob);

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(144, 263); 
	wavelen2Knob = new CKnob (size, this, kWaveLen2, bgKnob, knob, point);
	wavelen2Knob->setInsetValue (7);
	wavelen2Knob->setValue(effect->getParameter(kWaveLen2));
	frame->addView (wavelen2Knob);

	// osc1 and 2 display
	CBitmap* oscBitmap  = new CBitmap (kOscBitmap);

	size (0, 0, 13, 91);
	size.offset (41, 150 );
	osc1Display = new CMovieBitmap (size, this, -1, 5, 91, oscBitmap, point);
	osc1Display->setValue(effect->getParameter(kWaveform1));
	frame->addView (osc1Display);

	size (0, 0, 13, 91);
	size.offset (220, 149 );
	osc2Display = new CMovieBitmap (size, this, -1, 5, 91, oscBitmap, point);
	osc2Display->setValue(effect->getParameter(kWaveform2));
	frame->addView (osc2Display);

	oscBitmap->forget ();

	//==LFO1===========================================================

	// mode button
	size(0, 0, 60, 29);
	size.offset(124, 423);
	lfo1Button = new CKickButton (size, this, kLFO1, 29, button, point);
	frame->addView (lfo1Button);

	// rate and amount

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(47, 391); 
	lfo1RKnob = new CKnob (size, this, kLFO1rate, bgKnob, knob, point);
	lfo1RKnob->setInsetValue (7);
	lfo1RKnob->setValue(effect->getParameter(kLFO1rate));
	frame->addView (lfo1RKnob);

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(225, 391); 
	lfo1AKnob = new CKnob (size, this, kLFO1amount, bgKnob, knob, point);
	lfo1AKnob->setInsetValue (7);
	lfo1AKnob->setValue(effect->getParameter(kLFO1amount));
	frame->addView (lfo1AKnob);

	// LFO display
	CBitmap* lfoBitmap  = new CBitmap (kLFODisplayBitmap);

	size (0, 0, 13, 51);
	size.offset (146, 365);
	lfo1Display = new CMovieBitmap (size, this, -1, 5, 51, lfoBitmap, point);
	lfo1Display->setValue(effect->getParameter(kLFO1));
	frame->addView (lfo1Display);

	//==LFO2===========================================================

	// mode button
	size(0, 0, 60, 29);
	size.offset(124, 550);
	lfo2Button = new CKickButton (size, this, kLFO2, 29, button, point);
	frame->addView (lfo2Button);

	// rate and amount

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(47, 518); 
	lfo2RKnob = new CKnob (size, this, kLFO2rate, bgKnob, knob, point);
	lfo2RKnob->setInsetValue (7);
	lfo2RKnob->setValue(effect->getParameter(kLFO2rate));
	frame->addView (lfo2RKnob);

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(225, 518); 
	lfo2AKnob = new CKnob (size, this, kLFO2amount, bgKnob, knob, point);
	lfo2AKnob->setInsetValue (7);
	lfo2AKnob->setValue(effect->getParameter(kLFO2amount));
	frame->addView (lfo2AKnob);

	// LFO display

	size (0, 0, 13, 51);
	size.offset (146, 492);
	lfo2Display = new CMovieBitmap (size, this, -1, 5, 51, lfoBitmap, point);
	lfo2Display->setValue(effect->getParameter(kLFO2));
	frame->addView (lfo2Display);
	lfoBitmap->forget();


	//==Test-button===========================================================
	size(0, 0, 60, 29);
	size.offset(800, 56);
	testButton = new COnOffButton (size, this, kTest, button);
	frame->addView (testButton);


	//==Mod-envelope===========================================================
	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(374, 79); 
	modEnvAttack = new CKnob (size, this, kModEnvA, bgKnob, knob, point);
	modEnvAttack->setInsetValue (7);
	modEnvAttack->setValue(effect->getParameter(kModEnvA));
	frame->addView (modEnvAttack);

	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(486, 79); 
	modEnvDecay = new CKnob (size, this, kModEnvD, bgKnob, knob, point);
	modEnvDecay->setInsetValue (7);
	modEnvDecay->setValue(effect->getParameter(kModEnvD));
	frame->addView (modEnvDecay);

	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(652, 79); 
	modEnvAmount = new CKnob (size, this, kModEnvAmount, bgKnob, knob, point);
	modEnvAmount->setInsetValue (7);
	modEnvAmount->setValue(effect->getParameter(kModEnvAmount));
	frame->addView (modEnvAmount);

	//==Amplifier===========================================================
	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(359, 509); 
	ampAttack = new CKnob (size, this, kAmplifierA, bgKnob, knob, point);
	ampAttack->setInsetValue (7);
	ampAttack->setValue(effect->getParameter(kAmplifierA));
	frame->addView (ampAttack);

	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(475, 509); 
	ampDecay = new CKnob (size, this, kAmplifierD, bgKnob, knob, point);
	ampDecay->setInsetValue (7);
	ampDecay->setValue(effect->getParameter(kAmplifierD));
	frame->addView (ampDecay);

	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(590, 509); 
	ampSustain = new CKnob (size, this, kAmplifierS, bgKnob, knob, point);
	ampSustain->setInsetValue (7);
	ampSustain->setValue(effect->getParameter(kAmplifierS));
	frame->addView (ampSustain);

	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(706, 509); 
	ampRelease = new CKnob (size, this, kAmplifierR, bgKnob, knob, point);
	ampRelease->setInsetValue (7);
	ampRelease->setValue(effect->getParameter(kAmplifierR));
	frame->addView (ampRelease);

	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(821, 509); 
	ampGain = new CKnob (size, this, kGain, bgKnob, knob, point);
	ampGain->setInsetValue (7);
	ampGain->setValue(effect->getParameter(kGain));
	frame->addView (ampGain);

	//==Filter===========================================================
	size(0, 0, 60, 29);
	size.offset(817, 267);
	filterButton = new CKickButton (size, this, kFilterType, 29, button, point);
	frame->addView (filterButton);

	// ADSR
 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(369, 226); 
	filterA = new CKnob (size, this, kFilterCutA, bgKnob, knob, point);
	filterA->setInsetValue (7);
	filterA->setValue(effect->getParameter(kFilterCutA));
	frame->addView (filterA);

	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(484, 226); 
	filterD = new CKnob (size, this, kFilterCutD, bgKnob, knob, point);
	filterD->setInsetValue (7);
	filterD->setValue(effect->getParameter(kFilterCutD));
	frame->addView (filterD);

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(600, 226); 
	filterS = new CKnob (size, this, kFilterCutS, bgKnob, knob, point);
	filterS->setInsetValue (7);
	filterS->setValue(effect->getParameter(kFilterCutS));
	frame->addView (filterS);

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(716, 226); 
	filterR = new CKnob (size, this, kFilterCutR, bgKnob, knob, point);
	filterR->setInsetValue (7);
	filterR->setValue(effect->getParameter(kFilterCutR));
	frame->addView (filterR);

	//
 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(369, 360); 
	filterCut = new CKnob (size, this, kFilterCut, bgKnob, knob, point);
	filterCut->setInsetValue (7);
	filterCut->setValue(effect->getParameter(kFilterCut));
	frame->addView (filterCut);

	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(485, 360); 
	filterRes = new CKnob (size, this, kFilterRes, bgKnob, knob, point);
	filterRes->setInsetValue (7);
	filterRes->setValue(effect->getParameter(kFilterRes));
	frame->addView (filterRes);

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(600, 360); 
	filterADSRAmount = new CKnob (size, this, kFilterADSRAmount, bgKnob, knob, point);
	filterADSRAmount->setInsetValue (7);
	filterADSRAmount->setValue(effect->getParameter(kFilterADSRAmount));
	frame->addView (filterADSRAmount);

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(797, 359); 
	distortion = new CKnob (size, this, kDistortion, bgKnob, knob, point);
	distortion->setInsetValue (7);
	distortion->setValue(effect->getParameter(kDistortion));
	frame->addView (distortion);

	// display
	CBitmap* filterBitmap  = new CBitmap (kFilterDisplayBitmap);

	size (0, 0, 13, 51);
	size.offset (840, 206);
	filterDisplay = new CMovieBitmap (size, this, -1, 5, 51, filterBitmap, point);
	filterDisplay->setValue(effect->getParameter(kFilterType));
	frame->addView (filterDisplay);

	filterBitmap->forget();

	//==Effects===========================================================
 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(369, 600); 
	echoDelay = new CKnob (size, this, kEchoDelay, bgKnob, knob, point);
	echoDelay->setInsetValue (7);
	echoDelay->setValue(effect->getParameter(kEchoDelay));
	frame->addView (echoDelay);

	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(485, 600); 
	echoAmount = new CKnob (size, this, kEchoAmount, bgKnob, knob, point);
	echoAmount->setInsetValue (7);
	echoAmount->setValue(effect->getParameter(kEchoAmount));
	frame->addView (echoAmount);

 	size (0, 0, 80, 14);
	size.offset(345, 650); 
	echoDisplay = new CParamDisplay(size);
	echoDisplay->setStringConvert (stringConvert);
	echoDisplay->setValue(effect->getParameter(kEchoDelay));
	frame->addView (echoDisplay);


	button->forget();
	knob->forget ();
	bgKnob->forget ();


	// here we can call a initialize () function to initalize all controls values
	return true;
}

//-----------------------------------------------------------------------------
bool VstKarmaEditor::keysRequired ()
{
	if (frame && frame->getEditView ())
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------
long VstKarmaEditor::onKeyDown (VstKeyCode &keyCode)
{
/*
	if (frame && cLabel && (keyCode.character >= 'a' && keyCode.character <= 'z'))
	{
		char val[64];
		char modifiers[32];
		strcpy (modifiers, "");
		if (keyCode.modifier & MODIFIER_SHIFT)
			strcpy (modifiers, "Shift+");
		if (keyCode.modifier & MODIFIER_ALTERNATE)
			strcat (modifiers, "Alt+");
		if (keyCode.modifier & MODIFIER_COMMAND)
			strcat (modifiers, "Cmd+");
		if (keyCode.modifier & MODIFIER_CONTROL)
			strcat (modifiers, "Ctrl+");

		sprintf (val, "onKeyDown : '%s%c'", modifiers, (char)(keyCode.character));
		cLabel->setLabel (val);
		return 1;
	}

	if (frame && (keyCode.virt == VKEY_UP || keyCode.virt == VKEY_DOWN))
	{
		CView *pView = frame->getCurrentView ();
		if (pView == cVerticalSlider || pView == cKnob || pView == cViewContainer)
		{
			CControl *control = (CControl*)pView;
			if (pView == cViewContainer)
			{
				pView = (CControl*)(cViewContainer->getCurrentView ());
				if (pView == cHorizontalSlider)
					control = (CControl*)pView;
				else
					return -1;
			}

			float inc;
			if (keyCode.virt == VKEY_UP)
				inc = 0.05f;
			else
				inc = -0.05f;
			float val = control->getValue () + inc;
			float min = control->getMin ();
			float max = control->getMax ();
			if (val > max)
				val = max;
			else if (val < min)
				val = min;
			control->setValue (val);

			return 1;
		}
	}
*/
	return -1;
}

//-----------------------------------------------------------------------------
long VstKarmaEditor::onKeyUp (VstKeyCode &keyCode)
{
/*
	if (cLabel && (keyCode.character >= 'a' && keyCode.character <= 'z'))
	{
		char val[64];
		char modifiers[32];
		strcpy (modifiers, "");
		if (keyCode.modifier & MODIFIER_SHIFT)
			strcpy (modifiers, "Shift+");
		if (keyCode.modifier & MODIFIER_ALTERNATE)
			strcat (modifiers, "Alt+");
		if (keyCode.modifier & MODIFIER_COMMAND)
			strcat (modifiers, "Cmd+");
		if (keyCode.modifier & MODIFIER_CONTROL)
			strcat (modifiers, "Ctrl+");

		sprintf (val, "onKeyUp : '%s%c'", modifiers, (char)(keyCode.character));
		cLabel->setLabel (val);
		return 1;
	}
*/
	return -1; 
}

//-----------------------------------------------------------------------------
void VstKarmaEditor::resume ()
{
	// called when the plugin will be On
}

//-----------------------------------------------------------------------------
void VstKarmaEditor::suspend ()
{
	// called when the plugin will be Off
}

//-----------------------------------------------------------------------------
void VstKarmaEditor::close ()
{
	// don't forget to remove the frame !!
	if (frame)
		delete frame;
	frame = 0;

	// set to zero all pointer (security)
	freq1Knob		= 0;
	freq2Knob		= 0;
	waveform1Button		= 0;
	waveform2Button		= 0;
	waveformMixKnob		= 0;
	wavelen1Knob		= 0;
	wavelen2Knob		= 0;
	osc1Display		= 0;
	osc2Display		= 0;

	lfo1AKnob		= 0;
	lfo1RKnob		= 0;
	lfo1Display		= 0;
	lfo1Button		= 0;

	lfo2AKnob		= 0;
	lfo2RKnob		= 0;
	lfo2Display		= 0;
	lfo2Button		= 0;

	testButton		= 0;

	modEnvAttack		= 0;
	modEnvDecay		= 0;
	modEnvAmount		= 0;

	//==Amplifier===========================================================
	ampAttack		= 0;
	ampDecay		= 0;
	ampSustain		= 0;
	ampRelease		= 0;
	ampGain			= 0;

	//==Filter===========================================================
	filterA			= 0;
	filterD			= 0;
	filterS			= 0;
	filterR			= 0;
	filterADSRAmount	= 0;
	filterCut		= 0;
	filterRes		= 0;
	distortion		= 0;
	filterButton		= 0;
	filterDisplay		= 0;

	//==Effects===========================================================
	echoDelay		= 0;
	echoAmount		= 0;
	echoDisplay		= 0;
}

//-----------------------------------------------------------------------------
void VstKarmaEditor::idle ()
{
	AEffGUIEditor::idle ();		// always call this to ensure update
/*
	if (cAutoAnimation && cAutoAnimation->isWindowOpened ())
	{
		long newTicks = getTicks ();
		if (newTicks > oldTicks + 60)
		{
			cAutoAnimation->nextPixmap ();
			oldTicks = newTicks;
		}
	}
*/
}

//-----------------------------------------------------------------------------
void VstKarmaEditor::setParameter (long index, float value)
{
	// called from the Aeffect to update the control's value

	// test if the plug is opened
	if (!frame)
		return;

	switch (index)
	{
		case kWaveform1:
			if (osc1Display)
				osc1Display->setValue(effect->getParameter(index));
			break;
		case kWaveform2:
			if (osc2Display)
				osc2Display->setValue(effect->getParameter(index));
			break;
		case kWaveLen1:
			if (wavelen1Knob)
				wavelen1Knob->setValue(effect->getParameter(index));
			break;
		case kWaveLen2:
			if (wavelen2Knob)
				wavelen2Knob->setValue(effect->getParameter(index));
			break;
		case kWaveformMix:
			if (waveformMixKnob)
				waveformMixKnob->setValue(effect->getParameter(index));
			break;
		case kFreq1:
			if (freq1Knob)
				freq1Knob->setValue(effect->getParameter(index));
			break;
		case kFreq2:
			if (freq2Knob)
				freq2Knob->setValue(effect->getParameter(index));
			break;
		case kLFO1:
			if (lfo1Display)
				lfo1Display->setValue(effect->getParameter(index));
			break;
		case kLFO2:
			if (lfo2Display)
				lfo2Display->setValue(effect->getParameter(index));
			break;
		case kLFO1amount:
			if (lfo1AKnob)
				lfo1AKnob->setValue(effect->getParameter(index));
			break;
		case kLFO2amount:
			if (lfo2AKnob)
				lfo2AKnob->setValue(effect->getParameter(index));
			break;
		case kLFO1rate:
			if (lfo1RKnob)
				lfo1RKnob->setValue(effect->getParameter(index));
			break;
		case kLFO2rate:
			if (lfo2RKnob)
				lfo2RKnob->setValue(effect->getParameter(index));
			break;
		case kModEnvAmount:
			if (modEnvAmount)
				modEnvAmount->setValue(effect->getParameter(index));
			break;
		case kModEnvA:
			if (modEnvAttack)
				modEnvAttack->setValue(effect->getParameter(index));
			break;
		case kModEnvD:
			if (modEnvDecay)
				modEnvDecay->setValue(effect->getParameter(index));
			break;
		case kGain:
			if (ampGain)
				ampGain->setValue(effect->getParameter(index));
			break;
		case kAmplifierA:
			if (ampAttack)
				ampAttack->setValue(effect->getParameter(index));
			break;
		case kAmplifierD:
			if (ampDecay)
				ampDecay->setValue(effect->getParameter(index));
			break;
		case kAmplifierS:
			if (ampSustain)
				ampSustain->setValue(effect->getParameter(index));
			break;
		case kAmplifierR:
			if (ampRelease)
				ampRelease->setValue(effect->getParameter(index));
			break;
		case kFilterCutA:
			if (filterA)
				filterA->setValue(effect->getParameter(index));
			break;
		case kFilterCutD:
			if (filterD)
				filterD->setValue(effect->getParameter(index));
			break;
		case kFilterCutS:
			if (filterS)
				filterS->setValue(effect->getParameter(index));
			break;
		case kFilterCutR:
			if (filterR)
				filterR->setValue(effect->getParameter(index));
			break;
		case kFilterCut:
			if (filterCut)
				filterCut->setValue(effect->getParameter(index));
			break;
		case kFilterRes:
			if (filterRes)
				filterRes->setValue(effect->getParameter(index));
			break;
		case kFilterADSRAmount:
			if (filterADSRAmount)
				filterADSRAmount->setValue(effect->getParameter(index));
			break;
		case kDistortion:
			if (distortion)
				distortion->setValue(effect->getParameter(index));
			break;
		case kFilterType:
			if (filterDisplay)
				filterDisplay->setValue(effect->getParameter(index));
			break;

		case kEchoDelay:
			if (echoDelay)
				echoDelay->setValue(effect->getParameter(index));
			if (echoDisplay)
				echoDisplay->setValue(effect->getParameter(index));
			break;
		case kEchoAmount:
			if (echoAmount)
				echoAmount->setValue(effect->getParameter(index));
			break;

/*
	case kSliderHTag:
		if (cHorizontalSlider)
			cHorizontalSlider->setValue (effect->getParameter (index));
		if (cHorizontalSlider2)
			cHorizontalSlider2->setValue (effect->getParameter (index));
		break;

	case kSliderVTag:
		if (cVerticalSlider)
			cVerticalSlider->setValue (effect->getParameter (index));
 		break;

	case kKnobTag:
		if (cKnob)
			cKnob->setValue (effect->getParameter (index));
		if (cParamDisplay)
			cParamDisplay->setValue (effect->getParameter (index));
		if (cSpecialDigit)
			cSpecialDigit->setValue (1000000 * effect->getParameter (index));
		if (cVuMeter)
			cVuMeter->setValue (effect->getParameter (index));
		if (cAnimKnob)
			cAnimKnob->setValue (effect->getParameter (index));
		if (cMovieBitmap)
			cMovieBitmap->setValue (effect->getParameter (index));
		break;
*/
	}

	// call this to be sure that the graphic will be updated
	postUpdate ();
}

//-----------------------------------------------------------------------------
void VstKarmaEditor::valueChanged (CDrawContext* context, CControl* control)
{
	// called when something changes in the UI (mouse, key..)
	int tag = control->getTag();
	switch (tag)
	{
		case kFilterType:
			{
				CKickButton *b = (CKickButton*) control;
				if (b->getValue() == 0) {
					float val = effect->getParameter(tag);
					val += 0.25;
					if (val > 1) val = 0;
					effect->setParameter(tag, val);
				}
			}
			break;
		case kWaveform1:
		case kWaveform2:
		case kLFO1:
		case kLFO2:
			{
				CKickButton *b = (CKickButton*) control;
				if (b->getValue() == 0) {
					float val = effect->getParameter(tag);
					val += 0.25;
					if (val > 1) val = 0;
					effect->setParameter(tag, val);
				}
			}
			break;
		default:
/*
		case kLFO1rate:
		case kLFO1amount:
		case kLFO2rate:
		case kLFO2amount:
		case kFreq1:
		case kFreq2:
		case kWaveformMix:
		case kTest:
*/
			{
//				CKnob *k = (CKnob*) control;
				effect->setParameter(tag, control->getValue());
			}
			break;

/*
	case kSliderVTag:
		// this function will called later the setParameter of VstKarmaEditor
		effect->setParameter (control->getTag (), control->getValue ());	
		effect->setParameter (kSliderHTag, control->getValue ());
		effect->setParameter (kKnobTag, control->getValue ());
		break;

	case kSliderHTag:
		effect->setParameter (control->getTag (), control->getValue ());	
		effect->setParameter (kSliderVTag, control->getValue ());
		effect->setParameter (kKnobTag, control->getValue ());
		break;

	case kKnobTag:
	{
		effect->setParameter (control->getTag (), control->getValue ());	
		effect->setParameter (kSliderVTag, control->getValue ());
		effect->setParameter (kSliderHTag, control->getValue ());
		char text[256];
		cTextEdit->getText (text);
		long v = context->getStringWidth (text);
	} break;

	case kAnimKnobTag:
		effect->setParameter (control->getTag (), control->getValue ());	
		effect->setParameter (kSliderVTag, control->getValue ());
		effect->setParameter (kSliderHTag, control->getValue ());
		effect->setParameter (kKnobTag, control->getValue ());
		break;


	// open file selector
	case kOnOffTag:
	{
		control->update (context);	

		AudioEffectX *effect = (AudioEffectX*)getEffect ();
		if (effect && control->getValue () > 0.5f)
		{
			if (effect->canHostDo ("openFileSelector"))
			{
				VstFileType aiffType ("AIFF File", "AIFF", "aif", "aiff", "audio/aiff", "audio/x-aiff");
				VstFileType aifcType ("AIFC File", "AIFC", "aif", "aifc", "audio/x-aifc");
				VstFileType waveType ("Wave File", ".WAV", "wav", "wav",  "audio/wav", "audio/x-wav");
				VstFileType sdIIType ("SoundDesigner II File", "Sd2f", "sd2", "sd2");

				VstFileSelect vstFileSelect;
				memset (&vstFileSelect, 0, sizeof (VstFileType));

				vstFileSelect.command     = kVstFileLoad;
				vstFileSelect.type        = kVstFileType;
				strcpy (vstFileSelect.title, "Test for open file selector");
				vstFileSelect.nbFileTypes = 1;
				vstFileSelect.fileTypes   = &waveType;
				vstFileSelect.returnPath  = new char[1024];
				//vstFileSelect.initialPath  = new char[1024];
				vstFileSelect.initialPath = 0;
				if (effect->openFileSelector (&vstFileSelect))
				{
					if (cLabel)
						cLabel->setLabel (vstFileSelect.returnPath);
					frame->setDropActive (true);
				}
				else
				{
					if (cLabel)
						cLabel->setLabel ("OpenFileSelector: canceled!!!!");
					frame->setDropActive (false);
				}
				delete []vstFileSelect.returnPath;
				if (vstFileSelect.initialPath)
					delete []vstFileSelect.initialPath;
			}
		}
		} break;
*/
//	default:
//		control->update (context);	
	}
}

//-----------------------------------------------------------------------------
void stringConvert (float value, char* string)
{
	 sprintf (string, "%1.4f sec", value*2);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
