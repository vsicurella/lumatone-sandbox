/*
  ==============================================================================

    LumatoneKeyComponent.cpp
    Created: 26 Jul 2023 11:51:13pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "LumatoneKeyComponent.h"


//==============================================================================

LumatoneKeyDisplay::LumatoneKeyDisplay(int newBoardIndex, int newKeyIndex)
	: Component("AllKeysOverview_Key" + juce::String(newKeyIndex) + "," + juce::String(newBoardIndex))
{
	// In your constructor, you should add any child components, and
	// initialise any special settings that your component needs.
	boardIndex = newBoardIndex;
	keyIndex = newKeyIndex;

	//	TerpstraSysExApplication::getApp().getLumatoneController()->addMidiListener(this);
}

LumatoneKeyDisplay::~LumatoneKeyDisplay()
{
	//	TerpstraSysExApplication::getApp().getLumatoneController()->removeMidiListener(this);
}

void LumatoneKeyDisplay::paint(juce::Graphics& g)
{
	jassert(getParentComponent() != nullptr);
	bool boardIsSelected = false;// boardIndex == dynamic_cast<AllKeysOverview*>(getParentComponent())->getCurrentSetSelection();

	juce::Colour hexagonColour = findColour(LumatoneKeyEdit::backgroundColourId).overlaidWith(getKeyColour());
	if (hexagonColour.getPerceivedBrightness() >= 0.6)
		hexagonColour = hexagonColour.darker((1.0 - hexagonColour.getPerceivedBrightness()));

	g.setColour(hexagonColour);

	if (colourGraphic && shadowGraphic)
	{
		int x = juce::roundToInt((getWidth() - colourGraphic->getWidth()) * 0.5f);
		int y = juce::roundToInt((getHeight() - colourGraphic->getHeight()) * 0.5f);

		g.drawImageAt(*colourGraphic, x, y, true);
		g.drawImageAt(*shadowGraphic, x, y);
	}
}

void LumatoneKeyDisplay::resized()
{

}

void LumatoneKeyDisplay::mouseDown(const juce::MouseEvent& e)
{
	Component::mouseDown(e);

	// Select this octave board
	jassert(getParentComponent() != nullptr);
	jassert(getParentComponent()->getParentComponent() != nullptr);
	//dynamic_cast<MainContentComponent*>(getParentComponent()->getParentComponent())->
	//	getOctaveBoardSelectorTab()->setCurrentTabIndex(boardIndex);

	isHighlighted = true;
	repaint();

	if (e.mods.isRightButtonDown())
	{
		// Right mouse click: popup menu
		//juce::PopupMenu menu;
		//TerpstraSysExApplication::getApp().getMainMenu()->createEditMenu(menu);
		//menu.showMenuAsync(PopupMenu::Options());
	}

	// TODO integrate interaction through LumatoneController

	//else
	//{
	//	// NoteOn MIDI message
	//	auto keyData = getKeyData();
	//	if (keyData != nullptr && keyData->channelNumber > 0)
	//	{
	//		if (keyData->keyType == LumatoneKeyType::velocityInterval)
	//		{
	//			// Send "note on" event
	//			//TerpstraSysExApplication::getApp().getMidiDriver().sendNoteOnMessage(keyData->noteNumber, keyData->channelNumber, 60);
	//		}
	//		// ToDo if keyType is "continuous controller": send controller event?
	//	}
	//}
}

void LumatoneKeyDisplay::mouseUp(const juce::MouseEvent& e)
{
	Component::mouseDown(e);

	isHighlighted = false;
	repaint();

	// TODO integrate interaction through LumatoneController

	//// NoteOff MIDI message
	//auto keyData = getKeyData();
	//if (keyData != nullptr && keyData->channelNumber > 0)
	//{
	//	if (keyData->keyType == TerpstraKey::velocityInterval)
	//	{
	//		// Send "note off" event
	//		TerpstraSysExApplication::getApp().getMidiDriver().sendNoteOffMessage(keyData->noteNumber, keyData->channelNumber, 60);
	//	}
	//}
}

//void LumatoneKeyDisplay::handleMidiMessage(const juce::MidiMessage& msg)
//{
//	auto keyData = getKeyData();
//	if (keyData != nullptr && msg.getChannel() == keyData->channelNumber && msg.getNoteNumber() == keyData->noteNumber)
//	{
//		isHighlighted = msg.isNoteOn();
//		repaint();
//	}
//}

const LumatoneKey* LumatoneKeyDisplay::getKeyData() const
{
	//if (boardIndex >= 0 && boardIndex < NUMBEROFBOARDS && keyIndex >= 0 && keyIndex < TerpstraSysExApplication::getApp().getOctaveBoardSize())
	//{
	//	jassert(getParentComponent() != nullptr);
	//	jassert(getParentComponent()->getParentComponent() != nullptr);
	//	return &dynamic_cast<MainContentComponent*>(getParentComponent()->getParentComponent())
	//		->getMappingInEdit().sets[boardIndex].theKeys[keyIndex];
	//}

	return nullptr;
}

juce::Colour LumatoneKeyDisplay::getKeyColour() const
{
	auto keyData = getKeyData();
	if (keyData != nullptr)
		return keyData->colour;
	else
		return findColour(LumatoneKeyEdit::backgroundColourId);
}

void LumatoneKeyDisplay::setKeyGraphics(juce::Image& colourGraphicIn, juce::Image& shadowGraphicIn)
{
	colourGraphic = &colourGraphicIn;
	shadowGraphic = &shadowGraphicIn;
}
