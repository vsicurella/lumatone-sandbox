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

typedef LumatoneFirmware::LumatoneKeyType LumatoneKeyType;

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

    bool configIsEqual(const LumatoneKey& compare) const;
    bool colourIsEqual(const LumatoneKey& compare) const;

    bool operator==(const LumatoneKey& compare) const { return configIsEqual(compare) && colourIsEqual(compare); }
    bool operator!=(const LumatoneKey& compare) const { return !(*this == compare); }

    // Chainable setters
    LumatoneKey withKeyType(LumatoneKeyType type) const { return LumatoneKey(type, channelNumber, noteNumber, colour, ccFaderDefault); }
    LumatoneKey withColour(juce::Colour newColour) const { return LumatoneKey(keyType, channelNumber, noteNumber, newColour, ccFaderDefault); }
    LumatoneKey withNoteOrCC(int noteOrCC) const { return LumatoneKey(keyType, channelNumber, noteOrCC, colour, ccFaderDefault); }
    LumatoneKey withChannelNumber(int channel) const { return LumatoneKey(keyType, channel, noteNumber, colour, ccFaderDefault); }
    LumatoneKey withInvertCCFader(bool invertCCFader) const { return LumatoneKey(keyType, channelNumber, noteNumber, colour, invertCCFader); }

public:
    int                noteNumber;
    int                channelNumber;
    juce::Colour       colour;
    bool               ccFaderDefault;
    LumatoneKeyType    keyType;
};

class MappedLumatoneKey : public LumatoneKey
{
public:
    MappedLumatoneKey()
        : LumatoneKey()
        , boardIndex(-1)
        , keyIndex(-1)
        { }

    MappedLumatoneKey(LumatoneKey key, int boardIndexIn, int keyIndexIn)
        : LumatoneKey(key)
		, boardIndex(boardIndexIn)
        , keyIndex(keyIndexIn)
        { }

	MappedLumatoneKey(LumatoneKey key, LumatoneKeyCoord coord)
		: MappedLumatoneKey(key, coord.boardIndex, coord.keyIndex) {}

    LumatoneKeyCoord getKeyCoord() const { return LumatoneKeyCoord(boardIndex, keyIndex); }

public:

    int boardIndex = -1;
    int keyIndex = -1;
};
