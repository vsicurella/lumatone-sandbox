/*
  ==============================================================================

    lumatone_key.h
    Created: 6 Aug 2023 2:16:18pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "../lumatone_editor_common.h"
#include "../lumatone_midi_driver/firmware_definitions.h"

// Mapping of one key
class LumatoneKey
{
public:
	LumatoneKey(LumatoneKeyType newKeyType = LumatoneKeyType::disabled) { noteNumber = 0; channelNumber = 1; colour = juce::Colour(); keyType = newKeyType; ccFaderDefault = true; };
	LumatoneKey(LumatoneKeyType newKeyType, int newChannelNumber, int newNoteNumber, juce::Colour newColour, bool invertCCFader = false)
	{
		keyType = newKeyType; channelNumber = newChannelNumber; noteNumber = newNoteNumber; colour = newColour; ccFaderDefault = invertCCFader;
	}
	bool isEmpty() const { return channelNumber == 0; }

	bool operator!=(const LumatoneKey& second) const { return noteNumber != second.noteNumber || channelNumber != second.channelNumber || colour != second.colour || keyType != second.keyType || ccFaderDefault != second.ccFaderDefault; }

	// Chainable setters
	LumatoneKey withKeyType(LumatoneKeyType type) const { return LumatoneKey(type, channelNumber, noteNumber, colour, ccFaderDefault); }
	LumatoneKey withColour(juce::Colour newColour) const { return LumatoneKey(keyType, channelNumber, noteNumber, newColour, ccFaderDefault); }
	LumatoneKey withNoteOrCC(int noteOrCC) const { return LumatoneKey(keyType, channelNumber, noteOrCC, colour, ccFaderDefault); }
	LumatoneKey withChannelNumber(int channel) const { return LumatoneKey(keyType, channel, noteNumber, colour, ccFaderDefault); }
	LumatoneKey withInvertCCFader(bool invertCCFader) const { return LumatoneKey(keyType, channelNumber, noteNumber, colour, invertCCFader); }

public:
	int				noteNumber;
	int			    channelNumber;
	juce::Colour    colour;
	bool            ccFaderDefault;
	LumatoneKeyType	keyType;
};
