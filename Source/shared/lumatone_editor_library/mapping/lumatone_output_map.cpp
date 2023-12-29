/*
  ==============================================================================

    lumatone_output_map.cpp
    Created: 31 Jul 2023 9:28:35pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_output_map.h"

LumatoneOutputMap::LumatoneOutputMap(LumatoneLayout* layout)
    : midiMap(280)
{
    if (layout != nullptr)
    {
        render(*layout);
    }
}

juce::Array<LumatoneKeyCoord> LumatoneOutputMap::getKeysAssignedToNoteOn(int midiChannel, int noteNumber)
{
    juce::String hash = juce::String(midiChannel) + "," + juce::String(noteNumber);
    return midiMap[hash];
}

void LumatoneOutputMap::render(const LumatoneLayout& layout, juce::HashMap<juce::String, juce::Array<LumatoneKeyCoord>>& map)
{
    map.clear();

    for (int boardIndex = 0; boardIndex < layout.getNumBoards(); boardIndex++)
    {
        for (int keyIndex = 0; keyIndex < layout.getOctaveBoardSize(); keyIndex++)
        {
            auto key = layout.readKey(boardIndex, keyIndex);
            
            juce::String midiHash = juce::String(key->channelNumber) + "," + juce::String(key->noteNumber);
            auto mappedKeys = map[midiHash];
            mappedKeys.add(LumatoneKeyCoord(boardIndex, keyIndex));
            map.set(midiHash, mappedKeys);
        }
    }
}

void LumatoneOutputMap::render(const LumatoneLayout& layout)
{
    render(layout, midiMap);
}
