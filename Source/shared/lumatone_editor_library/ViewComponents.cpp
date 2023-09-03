/*
  ==============================================================================

    ViewComponents.cpp
    Created: 16 Jun 2014 7:20:34pm
    Author:  hsstraub

  ==============================================================================
*/

#include "ViewComponents.h"

/*
==============================================================================
LumatoneKeyEdit class
==============================================================================
*/

LumatoneKeyEdit::LumatoneKeyEdit(int boardIndex, int keyIndex)
	: isSelected(false), board_idx(boardIndex), key_idx(keyIndex), keyColour(juce::Colour()), keyType(LumatoneKeyType::noteOnNoteOff)
{
	midiNoteLabel = new juce::Label("midiNoteLabel", "0");
	addAndMakeVisible(midiNoteLabel);
	midiNoteLabel->setJustificationType(juce::Justification::centredRight);
	//midiNoteLabel->setFont(TerpstraSysExApplication::getApp().getAppFont(LumatoneEditorFont::GothamNarrowBold));

	
	midiChannelLabel = new juce::Label("midiChannelLabel", "0");
	addAndMakeVisible(midiChannelLabel);
	//midiChannelLabel->setFont(TerpstraSysExApplication::getApp().getAppFont(LumatoneEditorFont::GothamNarrowMedium));
	midiChannelLabel->setJustificationType(juce::Justification::centredRight);

	addMouseListener(this, true);
}

LumatoneKeyEdit::~LumatoneKeyEdit()
{
	listeners.clear();
	deleteAllChildren();
}

int LumatoneKeyEdit::getBoardIndex() const
{
	return board_idx;
}

int LumatoneKeyEdit::getKeyIndex() const
{
	return key_idx;
}


LumatoneKey LumatoneKeyEdit::getValue() const
{
	LumatoneKey newValue;
	newValue.noteNumber = midiNoteLabel->getText().getIntValue();
	newValue.channelNumber = midiChannelLabel->getText().getIntValue();
	newValue.colour = keyColour;
	newValue.keyType = keyType;
    newValue.ccFaderDefault = ccFaderDefault;

	return newValue;
}

void LumatoneKeyEdit::setValue(const LumatoneKey& newValue)
{
	auto newKeyType = newValue.keyType;
    if (newKeyType == LumatoneKeyType::disabled || newKeyType == LumatoneKeyType::disabledDefault)
    {
		// TODO move this behavior elsewhere
        //newValue.keyType = LumatoneKeyType::disabled;

		newKeyType = LumatoneKeyType::disabled;
        midiNoteLabel->setText("x", juce::NotificationType::sendNotification);
        midiChannelLabel->setText("x", juce::NotificationType::sendNotification);
    }
    else
    {
        midiNoteLabel->setText(juce::String(newValue.noteNumber), juce::NotificationType::sendNotification);
        midiChannelLabel->setText(juce::String(newValue.channelNumber), juce::NotificationType::sendNotification);
    }

	keyColour = newValue.colour;
	keyType = newKeyType;
    ccFaderDefault = newValue.ccFaderDefault;

	juce::String newTooltip = juce::translate("KeyType") + " ";
	switch (keyType)
	{
	case LumatoneKeyType::noteOnNoteOff:
		newTooltip += juce::translate("NoteOnOff");
		break;
	case LumatoneKeyType::continuousController:
		newTooltip += juce::translate("ContinuousController");
        newTooltip += juce::newLine;
        if (ccFaderDefault)
            newTooltip += "CC Default (0->127)";
        else
            newTooltip += "CC Inverted (127->0)";
		break;
	case LumatoneKeyType::lumaTouch:
		newTooltip += juce::translate("Lumatouch");
		break;
	case LumatoneKeyType::disabled:
		newTooltip += juce::translate("Disabled");
		break;
	default:
		//jassertfalse;
		newTooltip += juce::translate("Unknown");
		break;
	}

	newTooltip += juce::newLine;
	newTooltip += juce::translate("KeyColour") + " " + keyColour.toDisplayString(false);
	
	setTooltip(newTooltip);
	midiNoteLabel->setTooltip(newTooltip);
	midiChannelLabel->setTooltip(newTooltip);

	repaint();
}

void LumatoneKeyEdit::setIsSelected(bool newValue)
{
	if (this->isSelected != newValue)
	{
		this->isSelected = newValue;
		repaint();
	}
}

void LumatoneKeyEdit::paint(juce::Graphics& g)
{
	LumatoneKey currentValue = getValue();

	// Selected or not: color and thickness of the line
	float lineWidth = TERPSTRASINGLEKEYFLDLINEWIDTH;
	juce::Colour lineColor = findColour(selectedKeyOutlineId);

	// Color: empty or the parametrized color
	juce::Colour bgColour = findColour(backgroundColourId).overlaidWith(currentValue.colour.withAlpha(TERPSTRASINGLEKEYCOLOURALPHA));
	juce::Colour textColour = bgColour.contrasting(0.7f);

	if (bgColour.getPerceivedBrightness() < 0.5)
	{
		textColour = textColour.brighter();
	}
	
    midiChannelLabel->setColour(juce::Label::textColourId, textColour);
    midiNoteLabel->setColour(juce::Label::textColourId, textColour);
    
    // Look depending on Key type
    if (currentValue.keyType == LumatoneKeyType::continuousController)
	{
		// Key type is continuous controller. Set colour gradient.
        float w = this->getWidth();
        float h = this->getHeight();
		juce::Colour inside, outside;
        if (currentValue.ccFaderDefault)
        {
            outside = bgColour.darker();
            inside = bgColour.brighter();
        }
        else
        {
            outside = bgColour.brighter();
            inside = bgColour.darker();
        }
		g.setGradientFill(
			juce::ColourGradient(inside, w * 0.5f, h * 0.5f, outside, w * 0.5f, 0.0f, true));
	}
	else
	{
		// Key type is note on/note off. Just plain colour.
		g.setColour(bgColour);
	}

	g.fillPath(hexPath);

	if (isSelected)
	{
		// Shrink bounds slightly
		juce::Path hexOutline = juce::Path(hexPath);
		juce::Rectangle<int> hexBounds = getLocalBounds().reduced(1);
		hexOutline.scaleToFit(hexBounds.getX(), hexBounds.getY(), hexBounds.getWidth(), hexBounds.getHeight(), true);

		// Draw line
		g.setColour(lineColor);
		g.strokePath(hexOutline, juce::PathStrokeType(lineWidth));
	}

//	if (currentValue.keyType == LumatoneKeyType::disabled)
//	{
//		float w = this->getWidth();
//		float h = this->getHeight();
//		float xProportion = 0.25f;
//		// Draw X on key
//		g.setColour(bgColour.contrasting(0.5f));
//		g.drawLine(w * xProportion, h * xProportion, w * (1-xProportion), h * (1-xProportion), 2);
//		g.drawLine(w * (1 - xProportion), h * xProportion, w * xProportion, h * (1 - xProportion), 2);
//	}

//    if (currentValue.keyType == LumatoneKeyType::disabled)
//    {
//        TerpstraSysExApplication::getApp().getLookAndFeel().getLabelFont(*midiNoteLabel);
//        g.setColour(textColour);
//        g.setFont(midiChannelLabel->getFont());
//        g.drawText("x", midiChannelLabel->getBounds(), midiChannelLabel->getJustificationType());
//        g.drawText("x", midiNoteLabel->getBounds(), midiChannelLabel->getJustificationType());
//    }
    
	// Something parametrized or not?
	if (currentValue.isEmpty())
	{
		midiChannelLabel->setAlpha(0.3f);
		midiNoteLabel->setAlpha(0.3f);
	}
	else
	{
		midiChannelLabel->setAlpha(1.0f);
		midiNoteLabel->setAlpha(1.0f);
	}
}

void LumatoneKeyEdit::resized()
{
	juce::Point<int> centre = getLocalBounds().getCentre();
	float radius = keySize * 0.5f;

	// Draw hexagon
	hexPath.clear();
	hexPath.addPolygon(centre.toFloat(), 6, radius, TERPSTRASINGLEKEYROTATIONANGLE);
	//Rectangle<float> hexBounds = hexPath.getBounds().reduced(1, 1);

	float lblSize = radius * TERPSTRASINGLEKEYLABELSIZE;
	float lblOffset = radius * 0.375f;

	midiChannelLabel->setSize(radius, lblSize);
	midiChannelLabel->setCentrePosition(centre.translated(-lblOffset, -(radius * channelLabelRadiusScalar)));

	midiNoteLabel->setSize(radius, lblSize);
	midiNoteLabel->setCentrePosition(centre.translated(0, lblOffset));
}

void LumatoneKeyEdit::setKeySize(float keySizeIn)
{
	keySize = keySizeIn;
	setSize(keySize, keySize);
}

void LumatoneKeyEdit::mouseDown(const juce::MouseEvent& e)
{
	DBG("key clicked!");
	auto value = getValue();
	listeners.call(&LumatoneKeyEditListener::keyClickedCallback, e, board_idx, key_idx, value);
}

juce::String LumatoneKeyEdit::toString() const
{
	juce::StringArray tokens;
	tokens.add("Board " + juce::String(board_idx));
	tokens.add("Key " + juce::String(key_idx));
	tokens.add("Note " + midiNoteLabel->getText());
	tokens.add("Ch " + midiChannelLabel->getText());
	tokens.add("Type " + juce::String(keyType));
	tokens.add("Col " + keyColour.toString());
	tokens.add("Size " + juce::String(keySize));
	return tokens.joinIntoString(" | ");
}

void LumatoneKeyEdit::addKeyEditListener(LumatoneKeyEditListener* listenerIn)
{
	listeners.add(listenerIn);
}

void LumatoneKeyEdit::removeKeyEditListener(LumatoneKeyEditListener* listenerIn)
{
	listeners.remove(listenerIn);
}


/*
==============================================================================
LumatoneKeyPtr struct
==============================================================================
*/

LumatoneKeyPtr::LumatoneKeyPtr(int boardIndex, int keyIndex, LumatoneKey* keyPtr)
	: board_idx(boardIndex), key_idx(keyIndex), key(keyPtr) {}

bool LumatoneKeyPtr::isValid() const
{
	//if (key_idx >= 0 && key_idx < TerpstraSysExApplication::getApp().getOctaveBoardSize() && board_idx >= 0)
	//	return true;
	return false;
}

bool LumatoneKeyPtr::isNull() const
{
	return key == nullptr;
}


LumatoneBoardSet::LumatoneBoardSet(int boardIndex)
	: board_idx(boardIndex)
{
	//for (int i = 0; i < TerpstraSysExApplication::getApp().getOctaveBoardSize(); i++)
	//{
	//	keys[i].reset(new LumatoneKeyEdit(boardIndex, i));
	//}
}

