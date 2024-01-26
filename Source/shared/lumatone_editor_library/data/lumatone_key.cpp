/*
==============================================================================

    lumatone_key.cpp
    Created: 6 Aug 2023 2:16:18pm
    Author:  Vincenzo

==============================================================================
*/

#include "lumatone_key.h"

LumatoneKey::LumatoneKey(LumatoneKeyType newKeyType)
{
    noteNumber = 0;
    channelNumber = 1;
    colour = juce::Colour();
    keyType = newKeyType;
    ccFaderDefault = true;

    state = juce::ValueTree(LumatoneKeyProperty::State);
    updateState();
}

LumatoneKey::LumatoneKey(LumatoneKeyType newKeyType, int newChannelNumber, int newNoteNumber, juce::Colour newColour, bool invertCCFader)
{
    keyType = newKeyType;
    channelNumber = newChannelNumber;
    noteNumber = newNoteNumber;
    colour = newColour;
    ccFaderDefault = invertCCFader;

    state = juce::ValueTree(LumatoneKeyProperty::State);
    updateState();
}

LumatoneKey::LumatoneKey(juce::ValueTree keyStateIn)
{
    juce::ValueTree stateToCopy(LumatoneKeyProperty::State);

    if (keyStateIn.hasType(LumatoneKeyProperty::State))
    {
        stateToCopy = keyStateIn;
    }

    state = stateToCopy;
    refreshFromState();
}

bool LumatoneKey::configIsEqual(const LumatoneKey &compare) const
{
    return keyType == compare.keyType
        && channelNumber == compare.channelNumber
        && noteNumber == compare.noteNumber
        && ccFaderDefault == compare.ccFaderDefault;
}

bool LumatoneKey::colourIsEqual(const LumatoneKey& compare) const
{
    return colour == compare.colour;
}

void LumatoneKey::operator=(const LumatoneKey &keyToCopy)
{
    if (*this == keyToCopy)
      return;

    keyType = keyToCopy.keyType;
    channelNumber = keyToCopy.channelNumber;
    noteNumber = keyToCopy.noteNumber;
    colour = keyToCopy.colour;
    ccFaderDefault = keyToCopy.ccFaderDefault;
    updateState();
}

void LumatoneKey::setKeyType(LumatoneKeyType typeIn)
{
    keyType = typeIn;
    state.setProperty(LumatoneKeyProperty::Type, keyType, nullptr);
}

void LumatoneKey::setColour(juce::Colour colourIn)
{
    colour = colourIn;
    state.setProperty(LumatoneKeyProperty::Colour, colour.toString(), nullptr);
}

void LumatoneKey::setNoteOrCC(int noteOrCC)
{
    noteNumber = noteOrCC;
    state.setProperty(LumatoneKeyProperty::MidiNote, noteNumber, nullptr);
}

void LumatoneKey::setChannelNumber(int channelIn)
{
    channelNumber = channelIn;
    state.setProperty(LumatoneKeyProperty::MidiChnl, channelNumber, nullptr);
}

void LumatoneKey::setDefaultCCFader(bool defaultCCFader)
{
    ccFaderDefault = defaultCCFader;
    state.setProperty(LumatoneKeyProperty::DefaultCCFader, ccFaderDefault, nullptr);
}

void LumatoneKey::setConfig(const LumatoneKey& config)
{
    noteNumber = config.noteNumber;
    channelNumber = config.channelNumber;
    ccFaderDefault = config.ccFaderDefault;
    keyType = config.keyType;
    updateState();
}

juce::ValueTree LumatoneKey::getState() const
{
    return state;
}

void LumatoneKey::updateState()
{
    // juce::ValueTree newState(LumatoneKeyProperty::State);
    state.setProperty(LumatoneKeyProperty::Type, keyType, nullptr);
    state.setProperty(LumatoneKeyProperty::Colour, colour.toString(), nullptr);
    state.setProperty(LumatoneKeyProperty::MidiNote, noteNumber, nullptr);
    state.setProperty(LumatoneKeyProperty::MidiChnl, channelNumber, nullptr);
    state.setProperty(LumatoneKeyProperty::DefaultCCFader, ccFaderDefault, nullptr);
    // state.copyPropertiesFrom(newState, nullptr);
}

void LumatoneKey::refreshFromState()
{
    keyType = (LumatoneKeyType)(int)state.getProperty(LumatoneKeyProperty::Type, (int)LumatoneKeyType::disabled);
    colour = juce::Colour::fromString(state.getProperty(LumatoneKeyProperty::Colour, juce::Colours::transparentBlack.toString()).toString());
    noteNumber = (int)state.getProperty(LumatoneKeyProperty::MidiNote, 0);
    channelNumber = (int)state.getProperty(LumatoneKeyProperty::MidiChnl, 1);
    ccFaderDefault = (bool)state.getProperty(LumatoneKeyProperty::DefaultCCFader, false);
}
