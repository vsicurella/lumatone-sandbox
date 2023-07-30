/*
  ==============================================================================

    LumatoneKeyComponent.h
    Created: 26 Jul 2023 11:51:13pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LumatoneDataStructures.h"
#include "ViewComponents.h"
#include "ImageResampling/ImageResampler.h"
#include "HexagonTilingGeometry.h"

// Representation of a key inside the overview
class LumatoneKeyDisplay : public juce::Component
{
public:
	LumatoneKeyDisplay(int newBoardIndex, int newKeyIndex, LumatoneKey keyData=LumatoneKey());
	~LumatoneKeyDisplay();

	void paint(juce::Graphics&) override;
	void resized() override;
	void mouseDown(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;

	void setLumatoneKey(const LumatoneKey& lumatoneKey);

	void setKeyGraphics(juce::Image& colourGraphicIn, juce::Image& shadowGraphicIn);

	// Implementation of TerpstraNidiDriver::Listener
	//void midiMessageReceived(const MidiMessage& midiMessage) override;
	//void midiMessageSent(const MidiMessage& midiMessage) override {}
	//void midiSendQueueSize(int queueSize) override {}
	//void generalLogMessage(String textMessage, HajuErrorVisualizer::ErrorLevel errorLevel) override {}
	//void handleMidiMessage(const MidiMessage& msg) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumatoneKeyDisplay)

	const LumatoneKey* getKeyData() const;
	juce::Colour getKeyColour() const;

	LumatoneKey keyData;

	int boardIndex = -1;
	int keyIndex = -1;
	bool isHighlighted = false;

	juce::Image* colourGraphic = nullptr;
	juce::Image* shadowGraphic = nullptr;

	//DEBUG
	juce::Colour keyColour;
};

