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

#include "resource.h"


enum
{
	// bitmaps
	kBackgroundBitmap = IDB_BITMAP1,
	
	kButtonBitmap,
	kEnvDisplayBitmap,
	kFilterDisplayBitmap,
	kOscBitmap,

	kKnobBgBitmap,
	kKnobFgBitmap,


	// others
	kBackgroundW = 904,
	kBackgroundH = 589
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

	//--CKickButton-----------------------------------------------
	CBitmap *button = new CBitmap (kButtonBitmap);

	size(0, 0, 60, 29);
	size.offset(39, 251);
	waveform1Button = new CKickButton (size, this, kWaveform1, 29, button, point);
	frame->addView (waveform1Button);

	size(0, 0, 60, 29);
	size.offset(218, 250);
	waveform2Button = new CKickButton (size, this, kWaveform2, 29, button, point);
	frame->addView (waveform2Button);

	button->forget();


	//--CKnob--------------------------------------
	CBitmap *knob   = new CBitmap (kKnobFgBitmap);
	CBitmap *bgKnob = new CBitmap (kKnobBgBitmap);

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

 	size (0, 0, bgKnob->getWidth (), bgKnob->getHeight ());
	size.offset(142, 173); 
	waveformMixKnob = new CKnob (size, this, kWaveformMix, bgKnob, knob, point);
	waveformMixKnob->setInsetValue (7);
	waveformMixKnob->setValue(effect->getParameter(kWaveformMix));
	frame->addView (waveformMixKnob);

	knob->forget ();
	bgKnob->forget ();


	CBitmap* oscBitmap  = new CBitmap (kOscBitmap);
//	CBitmap* vuOffBitmap = new CBitmap (kOscBackBitmap);

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
//	vuOffBitmap->forget();

/*
	//--CMovieButton--------------------------------------
 	size (0, 0, onOffButton->getWidth (), onOffButton->getHeight () / 2);
	size.offset (210 + 20, 20);
	point (0, 0);
	cMovieButton = new CMovieButton (size, this, kMovieButtonTag, onOffButton->getHeight () / 2, onOffButton, point);
	frame->addView (cMovieButton);

	onOffButton->forget ();
*/


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
	freq1Knob = 0;
	freq2Knob = 0;
	waveform1Button = 0;
	waveform2Button = 0;
	waveformMixKnob = 0;
	osc1Display = 0;
	osc2Display = 0;
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
			osc1Display->setValue(effect->getParameter(index));
			break;
		case kWaveform2:
			osc2Display->setValue(effect->getParameter(index));
			break;
		case kWaveformMix:
			waveformMixKnob->setValue(effect->getParameter(index));
			break;
		case kFreq1:
			freq1Knob->setValue(effect->getParameter(index));
			break;
		case kFreq2:
			freq2Knob->setValue(effect->getParameter(index));
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
		case kWaveform1:
		case kWaveform2:
			{
				CKickButton *b = (CKickButton*) control;
				if (b->getValue() == 0) {
					float val = effect->getParameter(tag);
					val += 0.219;
					if (val > 1) val = 0;
					effect->setParameter(tag, val);
				}
			}
			break;
		case kFreq1:
		case kFreq2:
		case kWaveformMix:
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
	default:
		control->update (context);	
	}
}

//-----------------------------------------------------------------------------
void stringConvert (float value, char* string)
{
	 sprintf (string, "p %.2f", value);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
