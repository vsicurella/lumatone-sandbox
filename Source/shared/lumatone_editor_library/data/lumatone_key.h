/*
  ==============================================================================

    lumatone_key.h
    Created: 6 Aug 2023 2:16:18pm
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_KEY_STATE_H
#define LUMATONE_KEY_STATE_H

#include "lumatone_editor_common.h"

#include "../lumatone_midi_driver/firmware_definitions.h"

typedef LumatoneFirmware::LumatoneKeyType LumatoneKeyType;

namespace LumatoneKeyProperty
{
    static const juce::Identifier State = juce::Identifier("LumatoneKeyState");
    static const juce::Identifier Type = juce::Identifier("Type");
    static const juce::Identifier Colour = juce::Identifier("Colour");
    static const juce::Identifier MidiNote = juce::Identifier("MidiNote");
    static const juce::Identifier MidiChnl = juce::Identifier("MidiChannel");
    static const juce::Identifier DefaultCCFader = juce::Identifier("InvertCCFader");

    static const juce::Identifier Index = juce::Identifier("Index");
    static const juce::Identifier Board = juce::Identifier("Board");
}

// Mapping of one key

class LumatoneKey
{
public:
    LumatoneKey(LumatoneKeyType newKeyType = LumatoneKeyType::disabled);
    LumatoneKey(LumatoneKeyType newKeyType, int newChannelNumber, int newNoteNumber, juce::Colour newColour, bool invertCCFader = false);
    LumatoneKey(juce::ValueTree keyStateIn);

    bool isEmpty() const { return channelNumber == 0; }

    bool configIsEqual(const LumatoneKey& compare) const;
    bool colourIsEqual(const LumatoneKey& compare) const;

    void operator=(const LumatoneKey& keyToCopy);

    bool operator==(const LumatoneKey& compare) const { return configIsEqual(compare) && colourIsEqual(compare); }
    bool operator!=(const LumatoneKey& compare) const { return !(*this == compare); }

    LumatoneKeyType getType() const { return keyType; }
    int getMidiNumber() const { return noteNumber; }
    int getMidiChannel() const { return channelNumber; }
    juce::Colour getColour() const { return colour; }
    bool isCCFaderDefault() const { return ccFaderDefault; }

    bool hasType(LumatoneKeyType compare) const { return keyType == compare; }
    bool isColour(juce::Colour compare) const { return colour == compare; }
    bool hasMidiNumber(int number) const { return noteNumber == number; }
    bool hasMidiChannel(int number) const { return channelNumber == number; }
    bool hasCCFaderFlag(bool flag) const { return ccFaderDefault == flag; }

    void setKeyType(LumatoneKeyType typeIn);
    void setColour(juce::Colour colourIn);
    void setNoteOrCC(int noteOrCC);
    void setChannelNumber(int channelIn);
    void setDefaultCCFader(bool invertCCFaderIn);

    // Everything except for colour
    void setConfig(const LumatoneKey& config);

    // // Chainable setters
    // LumatoneKey withKeyType(LumatoneKeyType type) const { return LumatoneKey(type, channelNumber, noteNumber, colour, ccFaderDefault); }
    // LumatoneKey withColour(juce::Colour newColour) const { return LumatoneKey(keyType, channelNumber, noteNumber, newColour, ccFaderDefault); }
    // LumatoneKey withNoteOrCC(int noteOrCC) const { return LumatoneKey(keyType, channelNumber, noteOrCC, colour, ccFaderDefault); }
    // LumatoneKey withChannelNumber(int channel) const { return LumatoneKey(keyType, channel, noteNumber, colour, ccFaderDefault); }
    // LumatoneKey withInvertCCFader(bool invertCCFader) const { return LumatoneKey(keyType, channelNumber, noteNumber, colour, invertCCFader); }

    juce::ValueTree getState() const;

private:
    void updateState();
    void refreshFromState();

private:
    LumatoneKeyType    keyType;
    int                noteNumber;
    int                channelNumber;
    juce::Colour       colour;
    bool               ccFaderDefault;

    juce::ValueTree     state;

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

#endif LUMATONE_KEY_STATE_H
